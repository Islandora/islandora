<?php

namespace Drupal\islandora\MediaSource;

use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Field\EntityReferenceFieldItemListInterface;
use Drupal\Core\File\FileSystemInterface;
use Drupal\Core\Language\LanguageManagerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\StreamWrapper\StreamWrapperManagerInterface;
use Drupal\field\FieldConfigInterface;
use Drupal\file\Entity\File;
use Drupal\file\FileInterface;
use Drupal\file\Validation\FileValidatorInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\media\MediaInterface;
use Drupal\media\MediaTypeInterface;
use Drupal\node\NodeInterface;
use Drupal\taxonomy\TermInterface;
use Symfony\Component\Filesystem\Path;
use Symfony\Component\HttpKernel\Exception\BadRequestHttpException;
use Symfony\Component\HttpKernel\Exception\HttpException;
use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;
use Symfony\Component\Mime\MimeTypeGuesserInterface;

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
   * File Validator service.
   *
   * @var \Drupal\file\Validation\FileValidatorInterface
   */
  protected $fileValidator;

  /**
   * Stream wrapper manager.
   *
   * @var \Drupal\Core\StreamWrapper\StreamWrapperManagerInterface
   */
  protected $streamWrapperManager;

  /**
   * Mime type guesser.
   *
   * @var \Symfony\Component\Mime\MimeTypeGuesserInterface
   */
  protected $mimeTypeGuesser;

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
   * @param \Drupal\file\Validation\FileValidatorInterface $file_validator
   *   File Validator service.
   * @param \Drupal\Core\StreamWrapper\StreamWrapperManagerInterface $stream_wrapper_manager
   *   Stream wrapper manager.
   * @param \Symfony\Component\Mime\MimeTypeGuesserInterface $mime_type_guesser
   *   Mime type guesser.
   */
  public function __construct(
    EntityTypeManagerInterface $entity_type_manager,
    AccountInterface $account,
    LanguageManagerInterface $language_manager,
    FileSystemInterface $file_system,
    IslandoraUtils $islandora_utils,
    FileValidatorInterface $file_validator,
    StreamWrapperManagerInterface $stream_wrapper_manager,
    MimeTypeGuesserInterface $mime_type_guesser,
  ) {
    $this->entityTypeManager = $entity_type_manager;
    $this->account = $account;
    $this->languageManager = $language_manager;
    $this->fileSystem = $file_system;
    $this->islandoraUtils = $islandora_utils;
    $this->fileValidator = $file_validator;
    $this->streamWrapperManager = $stream_wrapper_manager;
    $this->mimeTypeGuesser = $mime_type_guesser;
  }

  /**
   * Determines MIME-type to persist for a file.
   *
   * @param \Drupal\file\FileInterface $file
   *   File whose contents were just written.
   *
   * @return string
   *   MIME-type to persist.
   */
  private function determinePersistedMimeType(FileInterface $file) : string {
    $uri = $file->getFileUri();
    $path = $this->fileSystem->realpath($uri) ?: $uri;
    return $this->mimeTypeGuesser->guessMimeType($path) ?: 'application/octet-stream';
  }

  /**
   * Validates a content location URI and blocks traversal.
   *
   * @param string $content_location
   *   The user supplied content location.
   *
   * @return string
   *   The validated content location.
   */
  private function validateContentLocation(string $content_location) : string {
    $content_location = trim($content_location);
    if ($content_location === '') {
      throw new BadRequestHttpException("Missing Content-Location header");
    }

    if (str_contains($content_location, "\0")) {
      throw new BadRequestHttpException("Invalid Content-Location header");
    }

    if (!$this->streamWrapperManager->isValidUri($content_location)) {
      throw new BadRequestHttpException("Content-Location must be a valid stream wrapper URI");
    }

    $target = $this->streamWrapperManager->getTarget($content_location);
    if (!is_string($target) || $target === '') {
      throw new BadRequestHttpException("Content-Location must include a filename");
    }

    $target = trim($target);
    $trimmed_target = trim($target, '/');
    if ($trimmed_target === '') {
      throw new BadRequestHttpException("Content-Location must include a filename");
    }

    // Use Symfony path normalization to reject traversal and non-canonical
    // targets before any writes occur.
    $canonical_target = Path::canonicalize($target);
    if (
      str_contains($target, '\\') ||
      $canonical_target === '.' ||
      $canonical_target === '..' ||
      str_starts_with($canonical_target, '../') ||
      $canonical_target !== $trimmed_target
    ) {
      throw new BadRequestHttpException("Content-Location must not contain path traversal segments");
    }

    return $content_location;
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
   * @return \Drupal\file\FileInterface|\Drupal\Core\Entity\EntityInterface|false|null
   *   The first source entity if there is one, generally expected to be of
   *   \Drupal\file\FileInterface. Boolean FALSE if there was no such entity.
   *   NULL if the source field does not refer to Drupal entities (as in, the
   *   field is not a \Drupal\Core\Field\EntityReferenceFieldItemListInterface
   *   implementation).
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
    $source_list = $media->get($source_field);
    if ($source_list instanceof EntityReferenceFieldItemListInterface) {
      $files = $source_list->referencedEntities();
      return reset($files);
    }

    return NULL;
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
    $mimetype,
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

    $file->setMimeType($this->determinePersistedMimeType($file));

    // Flush the image cache for the image so thumbnails get regenerated.
    image_path_flush($uri);
  }

  /**
   * Ensure the directory exists into which we can create files.
   *
   * @param string $content_location
   *   A file we want to save.
   */
  private function initializeDestination(string $content_location) : void {
    $directory = $this->fileSystem->dirname($content_location);
    if (!$this->fileSystem->prepareDirectory($directory, FileSystemInterface::CREATE_DIRECTORY | FileSystemInterface::MODIFY_PERMISSIONS)) {
      throw new HttpException(500, "The destination directory does not exist, could not be created, or is not writable");
    }
  }

  /**
   * Initialize an empty file entity.
   *
   * We create as a temporary file, in case the uploading thread
   * exits without properly completing the upload. Drupal should try to clean up
   * any "temporary" files older than the system.file:temporary_maximum_age
   * config indicates (which defaults to 6 hours) during Drupal's cron runs.
   *
   * Additionally, Drupal should handle making the "temporary" file permanent,
   * when a reference to the file entity is saved into another entity.
   *
   * @param string $content_location
   *   The location in which to initialize the file.
   *
   * @return \Drupal\file\FileInterface
   *   The initialized file.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  private function initializeFile(string $content_location) : FileInterface {
    $this->initializeDestination($content_location);

    touch($content_location);
    return $this->entityTypeManager->getStorage('file')->create([
      'uid' => $this->account->id(),
      'uri' => $content_location,
      'filename' => $this->fileSystem->basename($content_location),
      'filemime' => 'application/octet-stream',
      'status' => 0,
    ]);
  }

  /**
   * Validate the given file's extension matches those from its field config.
   *
   * @param string $content_location
   *   The URI of the content to validate.
   * @param string $filemime
   *   The MIME-type of the file in question, if it is used during the extension
   *   validation.
   * @param \Drupal\field\FieldConfigInterface $field_config
   *   The field bearing some configured extensions against which to match.
   */
  private function validateFileExtension(string $content_location, string $filemime, FieldConfigInterface $field_config) : void {
    // Synthesize a file entity to throw at the validator, to validate the
    // extension while avoiding dealing with `hook_file_create()` as those hook
    // implementations may expect the file to exist in the indicated location;
    // however, it is not necessary for the file to exist in the given location
    // in order to validate its extensions.
    // XXX: Values passed to FileStorage::create() are not set directly in the
    // constructor.
    // @see https://git.drupalcode.org/project/drupal/-/blob/29c1e5b2ed2e41788869f5752c84d0237350ea12/core/lib/Drupal/Core/Entity/ContentEntityStorageBase.php#L128-129
    $file = new File([], 'file');
    $values = [
      'uid' => $this->account->id(),
      'uri' => $content_location,
      'filename' => $this->fileSystem->basename($content_location),
      'filemime' => $filemime,
      'status' => 0,
    ];
    foreach ($values as $key => $value) {
      $file->set($key, $value);
    }

    $valid_extensions = $field_config->getSetting('file_extensions');
    $validators = ['FileExtension' => ['extensions' => $valid_extensions]];
    $errors = $this->fileValidator->validate($file, $validators);

    if ($errors->count() > 0) {
      throw new BadRequestHttpException("Invalid file extension.  Valid types are $valid_extensions");
    }
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
    $content_location,
  ) {
    $content_location = $this->validateContentLocation($content_location);
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

      // Validate file extension.
      $source_field_config = $this->entityTypeManager->getStorage('field_config')->load("media.$bundle.$source_field");
      $this->validateFileExtension($content_location, $mimetype, $source_field_config);

      // Construct the File.
      $file = $this->initializeFile($content_location);
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
    $content_location,
  ) {
    $content_location = $this->validateContentLocation($content_location);
    if ($media->hasField($destination_field)) {

      // Validate file extension.
      $bundle = $media->bundle();
      $destination_field_config = $this->entityTypeManager->getStorage('field_config')->load("media.$bundle.$destination_field");
      $this->validateFileExtension($content_location, $mimetype, $destination_field_config);

      // Construct the File.
      $file = $this->initializeFile($content_location);
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
