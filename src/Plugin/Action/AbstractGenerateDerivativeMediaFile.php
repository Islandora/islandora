<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManager;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\Url;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\EventGenerator\EmitEvent;
use Drupal\islandora\EventGenerator\EventGeneratorInterface;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\jwt\Authentication\Provider\JwtAuth;
use Drupal\token\Token;
use Stomp\StatefulStomp;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Emits a Node for generating derivatives event.
 *
 * @Action(
 *   id = "generate_derivative_file",
 *   label = @Translation("Generate a Derivative File for Media Attachment"),
 *   type = "media"
 * )
 */
class AbstractGenerateDerivativeMediaFile extends EmitEvent {

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
   * @var \Drupal\token\Token
   */
  protected $token;

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
   * @param \Drupal\Core\Entity\EntityTypeManager $entity_type_manager
   *   Entity type manager.
   * @param \Drupal\islandora\EventGenerator\EventGeneratorInterface $event_generator
   *   EventGenerator service to serialize AS2 events.
   * @param \Stomp\StatefulStomp $stomp
   *   Stomp client.
   * @param \Drupal\jwt\Authentication\Provider\JwtAuth $auth
   *   JWT Auth client.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utility functions.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source
   *   Media source service.
   * @param \Drupal\token\Token $token
   *   Token service.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $entity_field_manager
   *   Field Manager service.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    AccountInterface $account,
    EntityTypeManager $entity_type_manager,
    EventGeneratorInterface $event_generator,
    StatefulStomp $stomp,
    JwtAuth $auth,
    IslandoraUtils $utils,
    MediaSourceService $media_source,
    Token $token,
    EntityFieldManagerInterface $entity_field_manager
  ) {
    parent::__construct(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $account,
      $entity_type_manager,
      $event_generator,
      $stomp,
      $auth
    );
    $this->utils = $utils;
    $this->mediaSource = $media_source;
    $this->token = $token;
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
      $container->get('current_user'),
      $container->get('entity_type.manager'),
      $container->get('islandora.eventgenerator'),
      $container->get('islandora.stomp'),
      $container->get('jwt.authentication.jwt'),
      $container->get('islandora.utils'),
      $container->get('islandora.media_source_service'),
      $container->get('token'),
      $container->get('entity_field.manager')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    $uri = 'http://pcdm.org/use#OriginalFile';
    return [
      'queue' => 'islandora-connector-houdini',
      'event' => 'Generate Derivative',
      'source_term_uri' => $uri,
      'mimetype' => '',
      'args' => '',
      'scheme' => file_default_scheme(),
      'path' => '[date:custom:Y]-[date:custom:m]/[media:mid].bin',
      'source_field_name' => 'field_media_file',
      'destination_field_name' => '',
    ];
  }

  /**
   * Override this to return arbitrary data as an array to be json encoded.
   */
  protected function generateData(EntityInterface $entity) {
    $data = parent::generateData($entity);
    if (get_class($entity) != 'Drupal\media\Entity\Media') {
      return;
    }
    $source_file = $this->mediaSource->getSourceFile($entity);
    if (!$source_file) {
      throw new \RuntimeException("Could not locate source file for media {$entity->id()}", 500);
    }
    $data['source_uri'] = $this->utils->getDownloadUrl($source_file);

    $route_params = [
      'media' => $entity->id(),
      'destination_field' => $this->configuration['destination_field_name'],
    ];
    $data['destination_uri'] = Url::fromRoute('islandora.attach_file_to_media', $route_params)
      ->setAbsolute()
      ->toString();

    $token_data = [
      'media' => $entity,
    ];
    $path = $this->token->replace($data['path'], $token_data);
    $data['file_upload_uri'] = $data['scheme'] . '://' . $path;
    $allowed = ['queue',
      'event',
      'args',
      'source_uri',
      'destination_uri',
      'file_upload_uri',
      'mimetype',
    ];
    foreach ($data as $key => $value) {
      if (!in_array($key, $allowed)) {
        unset($data[$key]);
      }
    }
    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $schemes = $this->utils->getFilesystemSchemes();
    $scheme_options = array_combine($schemes, $schemes);
    $form = parent::buildConfigurationForm($form, $form_state);
    $map = $this->entityFieldManager->getFieldMapByFieldType('file');
    $file_fields = $map['media'];
    $file_options = array_combine(array_keys($file_fields), array_keys($file_fields));
    $file_options = array_merge(['' => ''], $file_options);
    $form['event']['#disabled'] = 'disabled';

    $form['destination_field_name'] = [
      '#required' => TRUE,
      '#type' => 'select',
      '#options' => $file_options,
      '#title' => $this->t('Destination File field Name'),
      '#default_value' => $this->configuration['destination_field_name'],
      '#description' => $this->t('File field on Media Type to hold derivative.  Cannot be the same as source'),
    ];
    $form['mimetype'] = [
      '#type' => 'textfield',
      '#title' => t('Mimetype'),
      '#default_value' => $this->configuration['mimetype'],
      '#required' => TRUE,
      '#rows' => '8',
      '#description' => t('Mimetype to convert to (e.g. image/jpeg, video/mp4, etc...)'),
    ];
    $form['args'] = [
      '#type' => 'textfield',
      '#title' => t('Additional arguments'),
      '#default_value' => $this->configuration['args'],
      '#rows' => '8',
      '#description' => t('Additional command line arguments'),
    ];
    $form['scheme'] = [
      '#type' => 'select',
      '#title' => t('File system'),
      '#options' => $scheme_options,
      '#default_value' => $this->configuration['scheme'],
      '#required' => TRUE,
    ];
    $form['path'] = [
      '#type' => 'textfield',
      '#title' => t('File path'),
      '#default_value' => $this->configuration['path'],
      '#description' => t('Path within the upload destination where files will be stored. Includes the filename and optional extension.'),
    ];
    $form['queue'] = [
      '#type' => 'textfield',
      '#title' => t('Queue name'),
      '#default_value' => $this->configuration['queue'],
      '#description' => t('Queue name to send along to help routing events, CHANGE WITH CARE. Defaults to :queue', [
        ':queue' => $this->defaultConfiguration()['queue'],
      ]),
    ];
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigurationForm(array &$form, FormStateInterface $form_state) {
    parent::validateConfigurationForm($form, $form_state);

    $exploded_mime = explode('/', $form_state->getValue('mimetype'));

    if (count($exploded_mime) != 2) {
      $form_state->setErrorByName(
        'mimetype',
        t('Please enter a mimetype (e.g. image/jpeg, video/mp4, audio/mp3, etc...)')
      );
    }

    if (empty($exploded_mime[1])) {
      $form_state->setErrorByName(
        'mimetype',
        t('Please enter a mimetype (e.g. image/jpeg, video/mp4, audio/mp3, etc...)')
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    parent::submitConfigurationForm($form, $form_state);
    $this->configuration['mimetype'] = $form_state->getValue('mimetype');
    $this->configuration['args'] = $form_state->getValue('args');
    $this->configuration['scheme'] = $form_state->getValue('scheme');
    $this->configuration['path'] = trim($form_state->getValue('path'), '\\/');
    $this->configuration['destination_field_name'] = $form_state->getValue('destination_field_name');
  }

  /**
   * Find a media_type by id and return it or nothing.
   *
   * @param string $entity_id
   *   The media type.
   *
   * @return \Drupal\Core\Entity\EntityInterface|string
   *   Return the loaded entity or nothing.
   *
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   *   Thrown by getStorage() if the entity type doesn't exist.
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   *   Thrown by getStorage() if the storage handler couldn't be loaded.
   */
  protected function getEntityById($entity_id) {
    $entity_ids = $this->entityTypeManager->getStorage('media_type')
      ->getQuery()->condition('id', $entity_id)->execute();

    $id = reset($entity_ids);
    if ($id !== FALSE) {
      return $this->entityTypeManager->getStorage('media_type')->load($id);
    }
    return '';
  }

}
