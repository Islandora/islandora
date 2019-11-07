<?php

namespace Drupal\islandora\EventGenerator;

use Drupal\Core\Entity\EntityInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\user\UserInterface;
use Drupal\media\Entity\Media;
use Drupal\Core\Entity\EntityStorageInterface;

/**
 * The default EventGenerator implementation.
 *
 * Provides Activity Stream 2.0 serialized events.
 */
class EventGenerator implements EventGeneratorInterface {

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSource;

  /**
   * Constructor.
   *
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source
   *   Media source service.
   */
  public function __construct(IslandoraUtils $utils, MediaSourceService $media_source) {
    $this->utils = $utils;
    $this->mediaSource = $media_source;
  }

  /**
   * {@inheritdoc}
   */
  public function generateEvent(EntityInterface $entity, UserInterface $user, array $data) {

    $user_url = $this->utils->getEntityUrl($user);

    $entity_type = $entity->getEntityTypeId();

    if ($entity_type == 'file') {
      $entity_url = $this->utils->getDownloadUrl($entity);
      $mimetype = $entity->getMimeType();
    }
    else {
      $entity_url = $this->utils->getEntityUrl($entity);
      $mimetype = 'text/html';
    }

    $event = [
      "@context" => "https://www.w3.org/ns/activitystreams",
      "actor" => [
        "type" => "Person",
        "id" => "urn:uuid:{$user->uuid()}",
        "url" => [
          [
            "name" => "Canonical",
            "type" => "Link",
            "href" => "$user_url",
            "mediaType" => "text/html",
            "rel" => "canonical",
          ],
        ],
      ],
      "object" => [
        "id" => "urn:uuid:{$entity->uuid()}",
        "url" => [
          [
            "name" => "Canonical",
            "type" => "Link",
            "href" => $entity_url,
            "mediaType" => $mimetype,
            "rel" => "canonical",
          ],
        ],
      ],
    ];

    $entity_type = $entity->getEntityTypeId();
    $event_type = $data["event"];
    if ($data["event"] == "Generate Derivative") {
      $event["type"] = "Activity";
      $event["summary"] = $data["event"];
    }
    else {
      $event["type"] = ucfirst($data["event"]);
      $event["summary"] = ucfirst($data["event"]) . " a " . ucfirst($entity_type);
    }

    if ($data['event'] != "Generate Derivative") {
      $isNewRev = FALSE;
      if ($entity->getEntityType()->isRevisionable()) {
        $isNewRev = $this->isNewRevision($entity);
      }
      $event["object"]["isNewVersion"] = $isNewRev;
    }

    // Add REST links for non-file entities.
    if ($entity_type != 'file') {
      $event['object']['url'][] = [
        "name" => "JSON",
        "type" => "Link",
        "href" => $this->utils->getRestUrl($entity, 'json'),
        "mediaType" => "application/json",
        "rel" => "alternate",
      ];
      $event['object']['url'][] = [
        "name" => "JSONLD",
        "type" => "Link",
        "href" => $this->utils->getRestUrl($entity, 'jsonld'),
        "mediaType" => "application/ld+json",
        "rel" => "alternate",
      ];
    }

    // Add a link to the file described by a media.
    if ($entity_type == 'media') {
      $file = $this->mediaSource->getSourceFile($entity);
      if ($file) {
        $event['object']['url'][] = [
          "name" => "Describes",
          "type" => "Link",
          "href" => $this->utils->getDownloadUrl($file),
          "mediaType" => $file->getMimeType(),
          "rel" => "describes",
        ];
      }
    }

    unset($data["event"]);
    unset($data["queue"]);

    if (!empty($data)) {
      $event["attachment"] = [
        "type" => "Object",
        "content" => $data,
        "mediaType" => "application/json",
      ];
    }

    return json_encode($event);
  }

  /**
   * Method to check if an entity is a new revision.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   Drupal Entity.
   *
   * @return bool
   *   Is new version.
   */
  protected function isNewRevision(EntityInterface $entity) {
    if ($entity->getEntityTypeId() == "node") {
      $revision_ids = \Drupal::entityTypeManager()->getStorage($entity->getEntityTypeId())->revisionIds($entity);
      return count($revision_ids) > 1;
    }
    elseif ($entity->getEntityTypeId() == "media") {
      $mediaStorage = \Drupal::entityTypeManager()->getStorage($entity->getEntityTypeId());
      return count($this->getRevisionIds($entity, $mediaStorage)) > 1;
    }
  }

  /**
   * Method to get the revisionIds of a media object.
   *
   * @param \Drupal\media\Entity\Media $media
   *   Media object.
   * @param \Drupal\Core\Entity\EntityStorageInterface $media_storage
   *   Media Storage.
   */
  protected function getRevisionIds(Media $media, EntityStorageInterface $media_storage) {
    $result = $media_storage->getQuery()
      ->allRevisions()
      ->condition($media->getEntityType()->getKey('id'), $media->id())
      ->sort($media->getEntityType()->getKey('revision'), 'DESC')
      ->execute();
    return array_keys($result);
  }

}
