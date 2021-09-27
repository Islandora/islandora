<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\islandora\EventGenerator\EmitEvent;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\File\FileSystemInterface;
use Drupal\Core\Site\Settings;
use Drupal\Core\StreamWrapper\StreamWrapperManager;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Emits a File event.
 *
 * @Action(
 *   id = "emit_file_event",
 *   label = @Translation("Emit a file event to a queue/topic"),
 *   type = "file"
 * )
 */
class EmitFileEvent extends EmitEvent {

  /**
   * File system service.
   *
   * @var \Drupal\Core\File\FileSystemInterface
   */
  protected $fileSystem;

  /**
   * Setter for the file system service.
   */
  public function setFileSystemService(FileSystemInterface $file_system) {
    $this->fileSystem = $file_system;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    $instance = parent::create($container, $configuration, $plugin_id, $plugin_definition);

    $instance->setFileSystemService($container->get('file_system'));

    return $instance;
  }

  /**
   * {@inheritdoc}
   */
  protected function generateData(EntityInterface $entity) {
    $data = parent::generateData($entity);

    // This function is called on Media and File entity types.
    if (method_exists($entity, 'getFileUri')) {
      $uri = $entity->getFileUri();
      $scheme = StreamWrapperManager::getScheme($uri);
      $flysystem_config = Settings::get('flysystem');

      if (isset($flysystem_config[$scheme]) && $flysystem_config[$scheme]['driver'] == 'fedora') {
        // Fdora $uri for files may contain ':///' so we need to replace
        // the three / with two.
        if (strpos($uri, $scheme . ':///') !== FALSE) {
          $uri = str_replace($scheme . ':///', $scheme . '://', $uri);
        }
        $data['fedora_uri'] = str_replace("$scheme://", $flysystem_config[$scheme]['config']['root'], $uri);
      }
    }
    return $data;
  }

}
