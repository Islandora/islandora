<?php

namespace Drupal\islandora_text_extraction\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\media\Entity\Media;
use Drupal\islandora\MediaSource\MediaSourceService;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException;
use Symfony\Component\HttpKernel\Exception\BadRequestHttpException;

/**
 * Controller for Media Source.
 */
class MediaSourceController extends ControllerBase {

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSourceService;

  /**
   * Constructs the OCR attachment controller.
   *
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source_service
   *   Media source service.
   */
  public function __construct(MediaSourceService $media_source_service) {
    $this->mediaSourceService = $media_source_service;
  }

  /**
   * Controller's create method for dependency injection.
   *
   * @param \Symfony\Component\DependencyInjection\ContainerInterface $container
   *   The App Container.
   *
   * @return \Drupal\islandora\Controller\MediaSourceController
   *   Controller instance.
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('islandora.media_source_service'),
    );
  }

  /**
   * Attaches incoming file to existing media.
   *
   * @param \Drupal\media\Entity\Media $media
   *   Media to hold file.
   * @param string $destination_field
   *   Media field to hold file.
   * @param string $destination_text_field
   *   Media field to hold extracted text.
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   HTTP Request from Karaf.
   *
   * @return \Symfony\Component\HttpFoundation\Response
   *   HTTP response.
   *
   * @throws \Drupal\Core\Entity\EntityStorageException
   * @throws \Drupal\Core\TypedData\Exception\ReadOnlyException
   */
  public function attachToMedia(
    Media $media,
    string $destination_field,
    string $destination_text_field,
    Request $request,
  ) {
    $content_location = $request->headers->get('Content-Location', "");
    $contents = $request->getContent();

    if ($contents === '') {
      return new Response("<h1>Complete</h1>");
    }

    if (!$media->hasField($destination_text_field)) {
      throw new BadRequestHttpException("Media does not have destination text field $destination_text_field");
    }
    if (!$media->get($destination_text_field)->access('edit')) {
      throw new AccessDeniedHttpException("You do not have permission to edit the destination text field");
    }
    $this->mediaSourceService->putToMedia(
      $media,
      $destination_field,
      $request->getContent(TRUE),
      $request->headers->get('Content-Type', 'text/plain'),
      $content_location
    );
    // @todo The request actually has a malformed parameter string, ?text_format=plain_text?connection_close=true.
    if (substr($request->query->get('text_format', ''), 0, 10) == 'plain_text') {
      $contents = nl2br($contents);
    }
    $media->get($destination_text_field)->setValue($contents);
    $media->save();

    return new Response("<h1>Complete</h1>");
  }

}
