<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\File\FileSystemInterface;
use Drupal\Core\Messenger\MessengerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\Site\Settings;
use Drupal\Core\StreamWrapper\StreamWrapperManager;
use Drupal\jwt\Authentication\Provider\JwtAuth;
use Drupal\islandora\EventGenerator\EmitEvent;
use Drupal\islandora\EventGenerator\EventGeneratorInterface;
use Stomp\StatefulStomp;
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
   * @param \Drupal\jwt\Authentication\Provider\JwtAuth $auth
   *   JWT Auth client.
   * @param \Drupal\Core\File\FileSystemInterface $file_system
   *   File system service.
   * @param \Drupal\Core\Messenger\MessengerInterface $messenger
   *   The messenger.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    AccountInterface $account,
    EntityTypeManagerInterface $entity_type_manager,
    EventGeneratorInterface $event_generator,
    StatefulStomp $stomp,
    JwtAuth $auth,
    FileSystemInterface $file_system,
    MessengerInterface $messenger
  ) {
    parent::__construct(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $account,
      $entity_type_manager,
      $event_generator,
      $stomp,
      $auth,
      $messenger
    );
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
      $container->get('current_user'),
      $container->get('entity_type.manager'),
      $container->get('islandora.eventgenerator'),
      $container->get('islandora.stomp'),
      $container->get('jwt.authentication.jwt'),
      $container->get('file_system'),
      $container->get('messenger')
    );
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
