<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\File\FileSystemInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\StreamWrapper\StreamWrapperManager;
use Drupal\file\FileInterface;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a condition to filter media based on where its source file resides.
 *
 * @Condition(
 *   id = "file_uses_filesystem",
 *   label = @Translation("File uses filesystem"),
 *   context_definitions = {
 *     "file" = @ContextDefinition("entity:file", required = TRUE , label = @Translation("file"))
 *   }
 * )
 */
class FileUsesFilesystem extends ConditionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * Islandora utility functions.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * File system service.
   *
   * @var \Drupal\Core\File\FileSystemInterface
   */
  protected $fileSystem;

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
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    IslandoraUtils $utils,
    FileSystemInterface $file_system,
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->utils = $utils;
    $this->fileSystem = $file_system;
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
      $container->get('file_system')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $schemes = $this->utils->getFilesystemSchemes();
    $options = array_combine($schemes, $schemes);

    $form['filesystems'] = [
      '#title' => $this->t('Filesystems'),
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $this->configuration['filesystems'],
    ];
    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->configuration['filesystems'] = array_filter($form_state->getValue('filesystems'));
    parent::submitConfigurationForm($form, $form_state);
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
        'The file uses @filesystems or @last',
        [
          '@filesystems' => $filesystems,
          '@last' => $last,
        ]
      );
    }
    $filesystem = reset($this->configuration['filesystems']);
    return $this->t(
      'The file uses @filesystem',
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

    $file = $this->getContextValue('file');
    return $this->evaluateFile($file);
  }

  /**
   * The actual evaluate function.
   *
   * @param \Drupal\file\FileInterface $file
   *   File.
   *
   * @return bool
   *   TRUE on success.
   */
  protected function evaluateFile(FileInterface $file) {
    $uri = $file->getFileUri();
    $scheme = StreamWrapperManager::getScheme($uri);
    return !empty($this->configuration['filesystems'][$scheme]);
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return array_merge(
      ['filesystems' => []],
      parent::defaultConfiguration()
    );
  }

}
