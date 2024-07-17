<?php

namespace Drupal\islandora_iiif\Plugin\Action;

use Drupal\Component\Datetime\TimeInterface;
use Drupal\Core\Action\ConfigurableActionBase;
use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\islandora_iiif\IiifInfo;
use GuzzleHttp\Client;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Retrieve a large image file's dimensions and save them to a media's fields.
 *
 * @Action(
 *   id = "media_attributes_from_iiif_action",
 *   label = @Translation("Add image dimensions retrieved from the IIIF server"),
 *   type = "node"
 * )
 */
class MediaAttributesFromIiif extends ConfigurableActionBase implements ContainerFactoryPluginInterface {

  /**
   * Config factory.
   *
   * @var \Drupal\Core\Config\ConfigFactoryInterface
   */
  protected $configFactory;

  /**
   * Entity Field Manager.
   *
   * @var Drupal\Core\Entity\EntityFieldManagerInterface
   */
  protected $entityFieldManager;

  /**
   * Entity type Manager.
   *
   * @var Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * The HTTP client.
   *
   * @var \GuzzleHttp\Client
   */
  protected $httpClient;

  /**
   * The IIIF Info service.
   *
   * @var \Drupal\islandora_iiif\IiifInfo
   */
  protected $iiifInfo;

  /**
   * The logger.
   *
   * @var \Drupal\Core\Logger\LoggerChannelInterface
   */
  protected $logger;

  /**
   * Islandora utility functions.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * A MediaSourceService.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSource;

  /**
   * Constructs a TiffMediaSaveAction object.
   *
   * @param mixed[] $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin ID for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   * @param \Drupal\Component\Datetime\TimeInterface $time
   *   The time service.
   * @param \Guzzle\Http\Client $http_client
   *   The HTTP Client.
   * @param \Drupal\islandora_iiif\IiifInfo $iiif_info
   *   The IIIF INfo service.
   * @param \Drupal\islandora\IslandoraUtils $islandora_utils
   *   Islandora utility functions.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source
   *   Islandora media service.
   * @param \Drupal\Core\Logger\LoggerChannelInterface $channel
   *   Logger channel.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $entity_field_manager
   *   The entity field manager service.
   * @param Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   Config factory.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, EntityTypeManagerInterface $entity_type_manager, TimeInterface $time, Client $http_client, IiifInfo $iiif_info, IslandoraUtils $islandora_utils, MediaSourceService $media_source, LoggerChannelInterface $channel, EntityFieldManagerInterface $entity_field_manager, ConfigFactoryInterface $config_factory) {
    parent::__construct($configuration, $plugin_id, $plugin_definition, $entity_type_manager, $time);

    $this->configFactory = $config_factory;
    $this->entityTypeManager = $entity_type_manager;
    $this->httpClient = $http_client;
    $this->iiifInfo = $iiif_info;
    $this->utils = $islandora_utils;
    $this->mediaSource = $media_source;
    $this->logger = $channel;
    $this->entityFieldManager = $entity_field_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
          $configuration,
          $plugin_id,
          $plugin_definition,
          $container->get('entity_type.manager'),
          $container->get('datetime.time'),
          $container->get('http_client'),
          $container->get('islandora_iiif'),
          $container->get('islandora.utils'),
          $container->get('islandora.media_source_service'),
          $container->get('logger.channel.islandora'),
          $container->get('entity_field.manager'),
          $container->get('config.factory')
      );
  }

  /**
   * {@inheritdoc}
   */
  public function execute($entity = NULL) {
    $width = $height = FALSE;

    // Get the selected media use term.
    $source_term = $this->utils->getTermForUri($this->configuration['source_term_uri']);

    $source_mids = $this->utils->getMediaReferencingNodeAndTerm($entity, $source_term);
    if (!empty($source_mids)) {

      foreach ($source_mids as $source_mid) {

        /**
         * @var \Drupal\Media\MediaInterface
         */
        $source_media = $this->entityTypeManager->getStorage('media')->load($source_mid);

        // Get the media MIME Type.
        $source_file = $this->mediaSource->getSourceFile($source_media);
        $mime_type = $source_file->getMimeType();

        if (in_array($mime_type, ['image/tiff', 'image/jp2'])) {
          [$width, $height] = $this->iiifInfo->getImageDimensions($source_file);
        }

        $width_field = $this->getShortFieldName($this->configuration['width_field']);
        $height_field = $this->getShortFieldName($this->configuration['height_field']);
        if ($source_media->hasField($width_field) && $source_media->hasField($height_field)) {
          $source_media->set($width_field, $width);
          $source_media->set($height_field, $height);
          $source_media->save();
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function access($object, AccountInterface $account = NULL, $return_as_object = FALSE) {

    /**
* @var \Drupal\Core\Entity\EntityInterface $object
*/
    return $object->access('update', $account, $return_as_object);
  }

  /**
   * {@inheritDoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {

    $integer_fields = $this->getIntegerFields();

    $form['source_term'] = [
      '#type' => 'entity_autocomplete',
      '#target_type' => 'taxonomy_term',
      '#title' => $this->t('Media use term of media to process.'),
      '#default_value' => $this->utils->getTermForUri($this->configuration['source_term_uri']),
      '#required' => TRUE,
      '#description' => $this->t('Term that indicates the child media on which to perform this action.'),
    ];

    $form['width_field'] = [
      '#type' => 'select',
      '#title' => $this->t('Width Field'),
      '#description' => $this->t("Field to populate with an image's width."),
      '#default_value' => $this->configuration['width_field'],
      '#options' => $integer_fields,
    ];

    $form['height_field'] = [
      '#type' => 'select',
      '#title' => $this->t('Height Field'),
      '#description' => $this->t("Field to populate with an image's height."),
      '#default_value' => $this->configuration['height_field'],
      '#options' => $integer_fields,
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    $config = parent::defaultConfiguration();

    $config['source_term_uri'] = '';
    $config['width_field'] = '';
    $config['height_field'] = '';

    return $config;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $tid = $form_state->getValue('source_term');
    $term = $this->entityTypeManager->getStorage('taxonomy_term')->load($tid);
    $this->configuration['source_term_uri'] = $this->utils->getUriForTerm($term);

    $this->configuration['width_field'] = $form_state->getValue('width_field');
    $this->configuration['height_field'] = $form_state->getValue('height_field');

  }

  /**
   * Get all fields of an entity that are integer types.
   *
   * @return array
   *   The integer type fields.
   */
  protected function getIntegerFields() {
    // Get media types.
    $media_types = $this->entityTypeManager->getStorage('media_type')->loadMultiple();
    $all_integer_fields = [];
    foreach (array_keys($media_types) as $key => $value) {
      $fields = $this->entityFieldManager->getFieldDefinitions("media", $value);

      $integer_fields = array_filter(
            $fields,
            function ($field_value, $field_key) {
                // Only keep fields of type 'integer'.
                return (strpos($field_value->getType(), 'integer') > -1)
                && is_a($field_value, '\Drupal\Core\Field\FieldConfigInterface');
            }, ARRAY_FILTER_USE_BOTH
        );
      foreach ($integer_fields as $integer_field) {
        $all_integer_fields[$integer_field->id()] = $integer_field->getTargetEntityTypeId()
                    . ' -- ' . $integer_field->getTargetBundle() . ' -- ' . $integer_field->getLabel();
      }

    }
    return $all_integer_fields;
  }

  /**
   * Returns the last part of a qualified field name.
   *
   * @param string $field_id
   *   The full field id, e.g., 'media.file.field_height'.
   *
   * @return string
   *   The short field name, e.g., 'field_height'.
   */
  protected function getShortFieldName(string $field_id): string {
    [$entity_type, $bundle, $field_name] = explode('.', $field_id);
    return $field_name;
  }

}
