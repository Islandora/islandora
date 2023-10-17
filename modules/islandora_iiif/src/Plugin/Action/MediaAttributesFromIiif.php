<?php

namespace Drupal\islandora_iiif\Plugin\Action;

use Drupal\Component\Datetime\TimeInterface;
use Drupal\Core\Action\Plugin\Action\SaveAction;
use Drupal\Core\Config\ImmutableConfig;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\file\FileInterface;
use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\islandora_iiif\IiifInfo;
use Drupal\media\MediaInterface;
use Drupal\node\NodeInterface;
use GuzzleHttp\Client;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\Request;
/**
 * Provides an action that can save any entity.
 *
 * @Action(
 *   id = "islandora_iiif:media_attributes_from_iiif_action",
 *   action_label = @Translation("Add image dimensions retrieved from the IIIF server"),
 *   deriver = "Drupal\Core\Action\Plugin\Action\Derivative\EntityChangedActionDeriver",
 * )
 */
class MediaAttributesFromIiif extends SaveAction {

  /**
   * The HTTP client
   *
   * @var \GuzzleHttp\Client;
   */
  protected $httpClient;

/**
 * The IIIF Info service.
 *
 * @var IiifInfo
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
   * @param
   *   The time service.
   * @param \Guzzle\Http\Client $http_client
   * The HTTP Client.
   * @param IiifInfo $iiif_info
   * The IIIF INfo service.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source
   *   Media source service.
   * @param \Drupal\Core\Logger\LoggerChannelInterface $channel
   *   Logger channel.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, EntityTypeManagerInterface $entity_type_manager, TimeInterface $time, Client $http_client, IiifInfo $iiif_info, IslandoraUtils $islandora_utils, MediaSourceService $media_source, LoggerChannelInterface $channel) {
    parent::__construct($configuration, $plugin_id, $plugin_definition, $entity_type_manager, $time);

    $this->httpClient = $http_client;
$this->iiifInfo = $iiif_info;
$this->utils = $islandora_utils;
$this->mediaSource = $media_source;
$this->logger = $channel;
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
      $container->get('logger.channel.islandora')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function execute($entity = NULL ) {
    $width = $height = FALSE;

    // Get the original File media use term.
    $original_file_term = $this->utils->getTermForUri('http://pcdm.org/use#OriginalFile');

    /**
     * @var \Drupal\media\MediaInterface $original_file_media
     */
    $original_file_mids = $this->utils->getMediaReferencingNodeAndTerm($entity, $original_file_term);
    if (!empty($original_file_mids)) {

      // Ordinarily there shouldn't be more than one Original File media but it's not guaranteed.
      foreach($original_file_mids as $original_file_mid) {

        /*
        * @var \Drupal\Media\MediaInterface  $original_file_media
        */
        $original_file_media = $this->entityTypeManager->getStorage('media')->load($original_file_mid);

        // Get the media MIME Type
        $original_file = $this->mediaSource->getSourceFile($original_file_media);
        $mime_type = $original_file->getMimeType();

        if (in_array($mime_type, ['image/tiff', 'image/jp2'])) {
          [$width, $height] = $this->iiifInfo->getImageDimensions($original_file);
        }


        // @todo Make field configurable. Low priority since this whole thing is a workaround for an Islandora limitation.
        if ($original_file_media->hasField('field_width') && $original_file_media->hasField('field_height')) {
          $original_file_media->set('field_height', $height);
          $original_file_media->set('field_width', $width);
          $original_file_media->save();
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function access($object, AccountInterface $account = NULL, $return_as_object = FALSE) {

    /** @var \Drupal\Core\Entity\EntityInterface $object */
    return $object->access('update', $account, $return_as_object);
  }

  }
