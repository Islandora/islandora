<?php

namespace Drupal\islandora_text_extraction\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\File\FileSystem;
use Drupal\Core\File\FileSystemInterface;
use Drupal\media\Entity\Media;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\HttpKernel\Exception\HttpException;

/**
 * Controller for Media Source.
 */
class MediaSourceController extends ControllerBase {

  /**
   * Adds file to existing media.
   *
   * @param Drupal\media\Entity\Media $media
   *   The media to which file is added.
   * @param string $destination_field
   *   The name of the media field to add file reference.
   * @param string $destination_text_field
   *   The name of the media field to add file reference.
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The request object.
   *
   * @return \Symfony\Component\HttpFoundation\Response
   *   201 on success with a Location link header.
   *
   * @throws \Drupal\Core\Entity\EntityStorageException
   * @throws \Drupal\Core\TypedData\Exception\ReadOnlyException
   */

  /**
   * File system service.
   *
   * @var \Drupal\Core\File\FileSystem
   */
  protected $fileSystem;

  /**
   * MediaSourceController constructor.
   *
   * @param \Drupal\Core\File\FileSystem $fileSystem
   *   Filesystem service.
   */
  public function __construct(FileSystem $fileSystem) {
    $this->fileSystem = $fileSystem;
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
      $container->get('file_system')
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
    Request $request) {
    $content_location = $request->headers->get('Content-Location', "");
    $contents = $request->getContent();

    if ($contents) {
      $directory = $this->fileSystem->dirname($content_location);
      if (!$this->fileSystem->prepareDirectory($directory, FileSystemInterface::CREATE_DIRECTORY | FileSystemInterface::MODIFY_PERMISSIONS)) {
        throw new HttpException(500, "The destination directory does not exist, could not be created, or is not writable");
      }
      $file = file_save_data($contents, $content_location, FileSystemInterface::EXISTS_REPLACE);
      if ($media->hasField($destination_field)) {
        $media->{$destination_field}->setValue([
          'target_id' => $file->id(),
        ]);
      }
      else {
        $this->getLogger('islandora')->warning("Field $destination_field is not defined in  Media Type {$media->bundle()}");
      }
      if ($media->hasField($destination_text_field)) {
        // @todo The request actually has a malformed parameter string, ?text_format=plain_text?connection_close=true.
        if (substr($request->query->get('text_format'), 0, 10) == 'plain_text') {
          $contents = nl2br($contents);
        }
        $media->{$destination_text_field}->setValue($contents);
      }
      else {
        $this->getLogger('islandora')->warning("Field $destination_text_field is not defined in Media Type {$media->bundle()}");
      }
      $media->save();
    }
    // We'd only ever get here if testing the function with GET.
    return new Response("<h1>Complete</h1>");
  }

}
