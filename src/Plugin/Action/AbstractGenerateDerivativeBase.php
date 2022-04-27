<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\Core\Messenger\MessengerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\EventGenerator\EmitEvent;
use Drupal\islandora\EventGenerator\EventGeneratorInterface;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\token\TokenInterface;
use Stomp\StatefulStomp;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\EventDispatcher\EventDispatcherInterface;

/**
 * A base class for constructor/creator derivative generators.
 */
class AbstractGenerateDerivativeBase extends EmitEvent {

  /**
   * Islandora utility functions.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSource;

  /**
   * Token replacement service.
   *
   * @var \Drupal\token\TokenInterface
   */
  protected $token;

  /**
   * The messenger.
   *
   * @var \Drupal\Core\Messenger\MessengerInterface
   */
  protected $messenger;

  /**
   * The system file config.
   *
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  protected $config;

  /**
   * The entity field manager.
   *
   * @var \Drupal\Core\Entity\EntityFieldManager
   */
  protected $entityFieldManager;

  /**
   * Constructs a EmitEvent action.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Session\AccountInterface $account
   *   Current user.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   Entity type manager.
   * @param \Drupal\islandora\EventGenerator\EventGeneratorInterface $event_generator
   *   EventGenerator service to serialize AS2 events.
   * @param \Stomp\StatefulStomp $stomp
   *   Stomp client.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utility functions.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source
   *   Media source service.
   * @param \Drupal\token\TokenInterface $token
   *   Token service.
   * @param \Drupal\Core\Messenger\MessengerInterface $messenger
   *   The messenger.
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config
   *   The system file config.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $entity_field_manager
   *   Field Manager service.
   * @param \Symfony\Component\EventDispatcher\EventDispatcherInterface $event_dispatcher
   *   Event dispatcher service.
   * @param \Drupal\Core\Logger\LoggerChannelInterface $channel
   *   The logger channel.
   */
  public function __construct(
        array $configuration,
        $plugin_id,
        $plugin_definition,
        AccountInterface $account,
        EntityTypeManagerInterface $entity_type_manager,
        EventGeneratorInterface $event_generator,
        StatefulStomp $stomp,
        IslandoraUtils $utils,
        MediaSourceService $media_source,
        TokenInterface $token,
        MessengerInterface $messenger,
        ConfigFactoryInterface $config,
        EntityFieldManagerInterface $entity_field_manager,
        EventDispatcherInterface $event_dispatcher,
        LoggerChannelInterface $channel
    ) {
    $this->utils = $utils;
    $this->mediaSource = $media_source;
    $this->token = $token;
    $this->messenger = $messenger;
    $this->config = $config->get('system.file');
    $this->entityFieldManager = $entity_field_manager;
    parent::__construct(
          $configuration,
          $plugin_id,
          $plugin_definition,
          $account,
          $entity_type_manager,
          $event_generator,
          $stomp,
          $messenger,
          $event_dispatcher,
          $channel
      );
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
          $configuration,
          $plugin_id,
          $plugin_definition,
          $container->get('current_user'),
          $container->get('entity_type.manager'),
          $container->get('islandora.eventgenerator'),
          $container->get('islandora.stomp'),
          $container->get('islandora.utils'),
          $container->get('islandora.media_source_service'),
          $container->get('token'),
          $container->get('messenger'),
          $container->get('config.factory'),
          $container->get('entity_field.manager'),
          $container->get('event_dispatcher'),
          $container->get('logger.channel.islandora')
      );
  }

}
