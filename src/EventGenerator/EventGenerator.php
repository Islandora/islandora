<?php

namespace Drupal\islandora\EventGenerator;

use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Site\Settings;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\user\UserInterface;

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

    $flysystem_config = Settings::get('flysystem');
    if ($flysystem_config != NULL && array_key_exists('fedora', $flysystem_config)) {
      $fedora_url = $flysystem_config['fedora']['config']['root'];
      $event["target"] = $fedora_url;
    }

    $entity_type = $entity->getEntityTypeId();
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

    $allowed_keys = [
      "file_upload_uri",
      "fedora_uri",
      "source_uri",
      "destination_uri",
      "args",
      "mimetype",
      "source_field",
    ];
    $keys_to_unset = array_diff(array_keys($data), $allowed_keys);
    foreach ($keys_to_unset as $key) {
      unset($data[$key]);
    }

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
    elseif (in_array($entity->getEntityTypeId(), ["media", "taxonomy_term"])) {
      $entity_storage = \Drupal::entityTypeManager()->getStorage($entity->getEntityTypeId());
      return count($this->getRevisionIds($entity, $entity_storage)) > 1;
    }
  }

  /**
   * Method to get the revisionIds of an entity.
   *
   * @param \Drupal\entity\Entity\ContentEntityInterface $entity
   *   Entity instance such as a Media or Taxonomy term.
   * @param \Drupal\Core\Entity\EntityStorageInterface $entity_storage
   *   Entity Storage.
   */
  protected function getRevisionIds(ContentEntityInterface $entity, EntityStorageInterface $entity_storage) {
    $result = $entity_storage->getQuery()
      ->allRevisions()
      ->accessCheck(TRUE)
      ->condition($entity->getEntityType()->getKey('id'), $entity->id())
      ->sort($entity->getEntityType()->getKey('revision'), 'DESC')
      ->execute();
    return array_keys($result);
  }

}
