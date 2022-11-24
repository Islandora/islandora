<?php

namespace Drupal\islandora\MediaSource;

use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\File\FileSystemInterface;
use Drupal\Core\Language\LanguageManagerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\file\FileInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\media\MediaInterface;
use Drupal\media\MediaTypeInterface;
use Drupal\node\NodeInterface;
use Drupal\taxonomy\TermInterface;
use Symfony\Component\HttpKernel\Exception\BadRequestHttpException;
use Symfony\Component\HttpKernel\Exception\HttpException;
use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;

/**
 * Utility functions for working with source files for Media.
 */
class MediaSourceService {

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * Current user.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $account;

  /**
   * Language manager.
   *
   * @var \Drupal\Core\Language\LanguageManagerInterface
   */
  protected $languageManager;

  /**
   * File system service.
   *
   * @var \Drupal\Core\File\FileSystemInterface
   */
  protected $fileSystem;

  /**
   * Islandora Utility service.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $islandoraUtils;

  /**
   * Constructor.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   * @param \Drupal\Core\Session\AccountInterface $account
   *   The current user.
   * @param \Drupal\Core\Language\LanguageManagerInterface $language_manager
   *   Language manager.
   * @param \Drupal\Core\File\FileSystemInterface $file_system
   *   File system service.
   * @param \Drupal\islandora\IslandoraUtils $islandora_utils
   *   Utility service.
   */
  public function __construct(
    EntityTypeManagerInterface $entity_type_manager,
    AccountInterface $account,
    LanguageManagerInterface $language_manager,
    FileSystemInterface $file_system,
    IslandoraUtils $islandora_utils
  ) {
    $this->entityTypeManager = $entity_type_manager;
    $this->account = $account;
    $this->languageManager = $language_manager;
    $this->fileSystem = $file_system;
    $this->islandoraUtils = $islandora_utils;
  }

  /**
   * Gets the name of a source field for a Media.
   *
   * @param string $media_type
   *   Media bundle whose source field you are searching for.
   *
   * @return string|null
   *   Field name if it exists in configuration, else NULL.
   */
  public function getSourceFieldName($media_type) {
    $bundle = $this->entityTypeManager->getStorage('media_type')->load($media_type);
    if (!$bundle) {
      throw new NotFoundHttpException("Bundle $media_type does not exist");
    }

    $type_configuration = $bundle->get('source_configuration');
    if (!isset($type_configuration['source_field'])) {
      return NULL;
    }

    return $type_configuration['source_field'];
  }

  /**
   * Gets the value of a source field for a Media.
   *
   * @param \Drupal\media\MediaInterface $media
   *   Media whose source field you are searching for.
   *
   * @return \Drupal\file\FileInterface
   *   File if it exists
   *
   * @throws \Symfony\Component\HttpKernel\Exception\NotFoundHttpException
   */
  public function getSourceFile(MediaInterface $media) {
    // Get the source field for the media type.
    $source_field = $this->getSourceFieldName($media->bundle());

    if (empty($source_field)) {
      throw new NotFoundHttpException("Source field not set for {$media->bundle()} media");
    }

    // Get the file from the media.
    $files = $media->get($source_field)->referencedEntities();
    $file = reset($files);

    return $file;
  }

  /**
   * Updates a media's source field with the supplied resource.
   *
   * @param \Drupal\media\MediaInterface $media
   *   The media to update.
   * @param resource $resource
   *   New file contents as a resource.
   * @param string $mimetype
   *   New mimetype of contents.
   *
   * @throws \Symfony\Component\HttpKernel\Exception\HttpException
   */
  public function updateSourceField(
    MediaInterface $media,
    $resource,
    $mimetype
  ) {
    $source_field = $this->getSourceFieldName($media->bundle());
    $file = $this->getSourceFile($media);

    // Update it.
    $this->updateFile($file, $resource, $mimetype);
    $file->save();

    // Set fields provided by type plugin and mapped in bundle configuration
    // for the media.
    foreach ($media->bundle->entity->getFieldMap() as $source => $destination) {
      if ($media->hasField($destination) && $value = $media->getSource()->getMetadata($media, $source)) {
        $media->set($destination, $value);
      }
      // Ensure width and height are updated on File reference when it's an
      // image. Otherwise you run into scaling problems when updating images
      // with different sizes.
      if ($source == 'width' || $source == 'height') {
        $media->get($source_field)->first()->set($source, $value);
      }
    }

    $media->save();
  }

  /**
   * Updates a File's binary contents on disk.
   *
   * @param \Drupal\file\FileInterface $file
   *   File to update.
   * @param resource $resource
   *   Stream holding the new contents.
   * @param string $mimetype
   *   Mimetype of new contents.
   */
  protected function updateFile(FileInterface $file, $resource, $mimetype = NULL) {
    $uri = $file->getFileUri();

    $destination = fopen($uri, 'wb');
    if (!$destination) {
      throw new HttpException(500, "File $uri could not be opened to write.");
    }

    $content_length = stream_copy_to_stream($resource, $destination);

    fclose($destination);

    if ($content_length === FALSE) {
      throw new HttpException(500, "Request body could not be copied to $uri");
    }

    if ($content_length === 0) {
      // Clean up the newly created, empty file.
      unlink($uri);
      throw new HttpException(400, "No bytes were copied to $uri");
    }

    if (!empty($mimetype)) {
      $file->setMimeType($mimetype);
    }

    // Flush the image cache for the image so thumbnails get regenerated.
    image_path_flush($uri);
  }

  /**
   * Creates a new Media using the provided resource, adding it to a Node.
   *
   * @param \Drupal\node\NodeInterface $node
   *   The node to reference the newly created Media.
   * @param \Drupal\media\MediaTypeInterface $media_type
   *   Media type for new media.
   * @param \Drupal\taxonomy\TermInterface $taxonomy_term
   *   Term from the 'Behavior' vocabulary to give to new media.
   * @param resource $resource
   *   New file contents as a resource.
   * @param string $mimetype
   *   New mimetype of contents.
   * @param string $content_location
   *   Drupal/PHP stream wrapper for where to upload the binary.
   *
   * @throws \Symfony\Component\HttpKernel\Exception\HttpException
   */
  public function putToNode(
    NodeInterface $node,
    MediaTypeInterface $media_type,
    TermInterface $taxonomy_term,
    $resource,
    $mimetype,
    $content_location
  ) {
    $existing = $this->islandoraUtils->getMediaReferencingNodeAndTerm($node, $taxonomy_term);

    if (!empty($existing)) {
      // Just update already existing media.
      $media = $this->entityTypeManager->getStorage('media')->load(reset($existing));
      $this->updateSourceField(
          $media,
          $resource,
          $mimetype
      );
      return FALSE;
    }
    else {
      // Otherwise, the media doesn't exist yet.
      // So make everything by hand.
      // Get the source field for the media type.
      $bundle = $media_type->id();
      $source_field = $this->getSourceFieldName($bundle);
      if (empty($source_field)) {
        throw new NotFoundHttpException("Source field not set for $bundle media");
      }

      // Construct the File.
      $file = $this->entityTypeManager->getStorage('file')->create([
        'uid' => $this->account->id(),
        'uri' => $content_location,
        'filename' => $this->fileSystem->basename($content_location),
        'filemime' => $mimetype,
      ]);
      $file->setPermanent();

      // Validate file extension.
      $source_field_config = $this->entityTypeManager->getStorage('field_config')->load("media.$bundle.$source_field");
      $valid_extensions = $source_field_config->getSetting('file_extensions');
      $errors = file_validate_extensions($file, $valid_extensions);

      if (!empty($errors)) {
        throw new BadRequestHttpException("Invalid file extension.  Valid types are $valid_extensions");
      }

      $directory = $this->fileSystem->dirname($content_location);
      if (!$this->fileSystem->prepareDirectory($directory, FileSystemInterface::CREATE_DIRECTORY | FileSystemInterface::MODIFY_PERMISSIONS)) {
        throw new HttpException(500, "The destination directory does not exist, could not be created, or is not writable");
      }

      // Copy over the file content.
      $this->updateFile($file, $resource, $mimetype);
      $file->save();

      // Construct the Media.
      $media_struct = [
        'bundle' => $bundle,
        'uid' => $this->account->id(),
        'name' => $file->getFilename(),
        'langcode' => $this->languageManager->getDefaultLanguage()->getId(),
        "$source_field" => [
          'target_id' => $file->id(),
        ],
        IslandoraUtils::MEDIA_OF_FIELD => [
          'target_id' => $node->id(),
        ],
        IslandoraUtils::MEDIA_USAGE_FIELD => [
          'target_id' => $taxonomy_term->id(),
        ],
      ];

      // Set alt text.
      if ($source_field_config->getSetting('alt_field') && $source_field_config->getSetting('alt_field_required')) {
        $media_struct[$source_field]['alt'] = $file->getFilename();
      }

      $media = $this->entityTypeManager->getStorage('media')->create($media_struct);
      $media->save();
      return $media;
    }

  }

  /**
   * Creates a new File using the provided resource, adding it to a Media.
   *
   * @param \Drupal\media\MediaInterface $media
   *   The Media that will receive the new file.
   * @param string $destination_field
   *   The field on the media where the file will go.
   * @param resource $resource
   *   New file contents as a resource.
   * @param string $mimetype
   *   New mimetype of contents.
   * @param string $content_location
   *   Drupal/PHP stream wrapper for where to upload the binary.
   *
   * @throws \Symfony\Component\HttpKernel\Exception\BadRequestHttpException
   * @throws \Symfony\Component\HttpKernel\Exception\HttpException
   */
  public function putToMedia(
    MediaInterface $media,
    $destination_field,
    $resource,
    $mimetype,
    $content_location
  ) {
    if ($media->hasField($destination_field)) {
      // Construct the File.
      $file = $this->entityTypeManager->getStorage('file')->create([
        'uid' => $this->account->id(),
        'uri' => $content_location,
        'filename' => $this->fileSystem->basename($content_location),
        'filemime' => $mimetype,
      ]);
      $file->setPermanent();

      // Validate file extension.
      $bundle = $media->bundle();
      $destination_field_config = $this->entityTypeManager->getStorage('field_config')->load("media.$bundle.$destination_field");
      $valid_extensions = $destination_field_config->getSetting('file_extensions');
      $errors = file_validate_extensions($file, $valid_extensions);

      if (!empty($errors)) {
        throw new BadRequestHttpException("Invalid file extension.  Valid types are $valid_extensions");
      }

      $directory = $this->fileSystem->dirname($content_location);
      if (!$this->fileSystem->prepareDirectory($directory, FileSystemInterface::CREATE_DIRECTORY | FileSystemInterface::MODIFY_PERMISSIONS)) {
        throw new HttpException(500, "The destination directory does not exist, could not be created, or is not writable");
      }

      // Copy over the file content.
      $this->updateFile($file, $resource, $mimetype);
      $file->save();

      // Update the media.
      $media->{$destination_field}->setValue([
        'target_id' => $file->id(),
      ]);
      $media->save();
    }
    else {
      throw new BadRequestHttpException("Media does not have destination field $destination_field");
    }
  }

}
