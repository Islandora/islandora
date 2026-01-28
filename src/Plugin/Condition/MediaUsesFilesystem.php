<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\File\FileSystemInterface;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a condition to filter media based on where its source file resides.
 *
 * @Condition(
 *   id = "media_uses_filesystem",
 *   label = @Translation("Media uses filesystem"),
 *   context_definitions = {
 *     "media" = @ContextDefinition("entity:media", required = TRUE , label = @Translation("media"))
 *   }
 * )
 */
class MediaUsesFilesystem extends FileUsesFilesystem {

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSource;

  /**
   * Constructor.
   *
   * @param array $configuration
   *   The plugin configuration, i.e. an array with configuration values keyed
   *   by configuration option name. The special key 'context' may be used to
   *   initialize the defined contexts by setting it to an array of context
   *   values keyed by context names.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utility functions.
   * @param \Drupal\Core\File\FileSystemInterface $file_system
   *   File system service.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source
   *   Media source service.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    IslandoraUtils $utils,
    FileSystemInterface $file_system,
    MediaSourceService $media_source,
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition, $utils, $file_system);
    $this->mediaSource = $media_source;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('islandora.utils'),
      $container->get('file_system'),
      $container->get('islandora.media_source_service')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (count($this->configuration['filesystems']) > 1) {
      $filesystems = $this->configuration['filesystems'];
      $last = array_pop($filesystems);
      $filesystems = implode(', ', $filesystems);
      return $this->t(
        'The media uses @filesystems or @last',
        [
          '@filesystems' => $filesystems,
          '@last' => $last,
        ]
      );
    }
    $filesystem = reset($this->configuration['filesystems']);
    return $this->t(
      'The media uses @filesystem',
      [
        '@filesystem' => $filesystem,
      ]
    );
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    if (empty($this->configuration['filesystems']) && !$this->isNegated()) {
      return TRUE;
    }

    $media = $this->getContextValue('media');
    $file = $this->mediaSource->getSourceFile($media);
    if (!$file) {
      return FALSE;
    }
    return $this->evaluateFile($file);
  }

}
