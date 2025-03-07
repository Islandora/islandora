<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a condition based on a node's media's MimeType.
 *
 * Note that this condition applies when the parent node is viewed.
 * It is not fired during ingest (i.e., it doesn't apply to
 * derivative generation).
 *
 * @Condition(
 *   id = "media_has_mimetype",
 *   label = @Translation("Node has Media with Mime type"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", label = @Translation("node"))
 *   }
 * )
 */
class MediaHasMimetype extends ConditionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Term storage.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * A MediaSourceService.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  private $mediaSource;

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
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   Entity type manager.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source
   *   Media source service.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    IslandoraUtils $utils,
    EntityTypeManagerInterface $entity_type_manager,
    MediaSourceService $media_source
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition, $utils);
    $this->utils = $utils;
    $this->entityTypeManager = $entity_type_manager;
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
      $container->get('entity_type.manager'),
      $container->get('islandora.media_source_service')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form['mimetypes'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Mime types'),
      '#default_value' => $this->configuration['mimetypes'],
      '#maxlength' => 256,
      '#description' => $this->t('Comma-delimited list of Mime types (e.g. image/jpeg, video/mp4, etc...) that trigger the condition.'),
    ];
    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->configuration['mimetypes'] = $form_state->getValue('mimetypes');
    parent::submitConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    $mimetypes = $this->configuration['mimetypes'];
    return $this->t(
      'The media has one of the Mime types @mimetypes',
      [
        '@mimetypes' => $mimetypes,
      ]
    );
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    if (empty($this->configuration['mimetypes']) && !$this->isNegated()) {
      return TRUE;
    }

    $node = \Drupal::routeMatch()->getParameter('node');

    if (is_null($node) || is_string($node)) {
      return FALSE;
    }

    $media = $this->utils->getMedia($node);

    if (count($media) > 0) {
      $mimetypes = explode(',', str_replace(' ', '', $this->configuration['mimetypes']));
      foreach ($media as $medium) {
        $file = $this->mediaSource->getSourceFile($medium);
        if ($file && in_array($file->getMimeType(), $mimetypes)) {
          return TRUE;
        }
      }
    }
    else {
      return FALSE;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return array_merge(
      ['mimetypes' => ''],
      parent::defaultConfiguration()
    );
  }

}
