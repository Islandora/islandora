<?php

namespace Drupal\islandora\Form;

use Drupal\Core\Cache\Cache;
use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityTypeBundleInfoInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Site\Settings;
use Drupal\Core\Url;
use Stomp\Client;
use Stomp\Exception\StompException;
use Stomp\StatefulStomp;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Config form for Islandora settings.
 */
class IslandoraSettingsForm extends ConfigFormBase {

  const CONFIG_NAME = 'islandora.settings';
  const BROKER_URL = 'broker_url';
  const BROKER_USER = 'broker_user';
  const BROKER_PASSWORD = 'broker_password';
  const JWT_EXPIRY = 'jwt_expiry';
  const UPLOAD_FORM = 'upload_form';
  const UPLOAD_FORM_LOCATION = 'upload_form_location';
  const UPLOAD_FORM_ALLOWED_MIMETYPES = 'upload_form_allowed_mimetypes';
  const GEMINI_PSEUDO = 'gemini_pseudo_bundles';
  const FEDORA_URL = 'fedora_url';
  const TIME_INTERVALS = [
    'sec',
    'second',
    'min',
    'minute',
    'hour',
    'day',
    'week',
    'month',
    'year',
  ];
  const GEMINI_PSEUDO_FIELD = 'field_gemini_uri';

  /**
   * To list the available bundle types.
   *
   * @var \Drupal\Core\Entity\EntityTypeBundleInfoInterface
   */
  private $entityTypeBundleInfo;

  /**
   * The saved password (if set).
   *
   * @var string
   */
  private $brokerPassword;

  /**
   * The entity type manager service.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  private $entityTypeManager;

  /**
   * Constructs a \Drupal\system\ConfigFormBase object.
   *
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   The factory for configuration objects.
   * @param \Drupal\Core\Entity\EntityTypeBundleInfoInterface $entity_type_bundle_info
   *   The EntityTypeBundleInfo service.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The EntityTypeManager service.
   */
  public function __construct(
    ConfigFactoryInterface $config_factory,
    EntityTypeBundleInfoInterface $entity_type_bundle_info,
    EntityTypeManagerInterface $entity_type_manager
  ) {
    $this->setConfigFactory($config_factory);
    $this->entityTypeBundleInfo = $entity_type_bundle_info;
    $this->brokerPassword = $this->config(self::CONFIG_NAME)->get(self::BROKER_PASSWORD);
    $this->entityTypeManager = $entity_type_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
          $container->get('config.factory'),
          $container->get('entity_type.bundle.info'),
          $container->get('entity_type.manager')
      );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_admin_settings';
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      self::CONFIG_NAME,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $config = $this->config(self::CONFIG_NAME);

    $form['broker_info'] = [
      '#type' => 'details',
      '#title' => $this->t('Broker'),
      '#open' => TRUE,
    ];
    $form['broker_info'][self::BROKER_URL] = [
      '#type' => 'textfield',
      '#title' => $this->t('URL'),
      '#default_value' => $config->get(self::BROKER_URL),
    ];
    $broker_user = $config->get(self::BROKER_USER);
    $form['broker_info']['provide_user_creds'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Provide user identification'),
      '#default_value' => $broker_user ? TRUE : FALSE,
    ];
    $state_selector = 'input[name="provide_user_creds"]';
    $form['broker_info'][self::BROKER_USER] = [
      '#type' => 'textfield',
      '#title' => $this->t('User'),
      '#default_value' => $broker_user,
      '#states' => [
        'visible' => [
          $state_selector => ['checked' => TRUE],
        ],
        'required' => [
          $state_selector => ['checked' => TRUE],
        ],
      ],
    ];
    $form['broker_info'][self::BROKER_PASSWORD] = [
      '#type' => 'password',
      '#title' => $this->t('Password'),
      '#description' => $this->t('If this field is left blank and the user is filled out, the current password will not be changed.'),
      '#states' => [
        'visible' => [
          $state_selector => ['checked' => TRUE],
        ],
      ],
    ];
    $form[self::JWT_EXPIRY] = [
      '#type' => 'textfield',
      '#title' => $this->t('JWT Expiry'),
      '#default_value' => $config->get(self::JWT_EXPIRY),
      '#description' => $this->t('A positive time interval expression. Eg: "60 secs", "2 days", "10 hours", "7 weeks". Be sure you provide the time units (@unit), plurals are accepted.',
        ['@unit' => implode(", ", self::TIME_INTERVALS)]
      ),
    ];

    $form[self::UPLOAD_FORM] = [
      '#type' => 'fieldset',
      '#title' => $this->t('Add Children / Media Form'),
    ];

    $form[self::UPLOAD_FORM][self::UPLOAD_FORM_LOCATION] = [
      '#type' => 'textfield',
      '#title' => $this->t('Upload location'),
      '#description' => $this->t('Tokenized URI pattern where the uploaded file should go.  You may use tokens to provide a pattern (e.g. "fedora://[current-date:custom:Y]-[current-date:custom:m]")'),
      '#default_value' => $config->get(self::UPLOAD_FORM_LOCATION),
      '#element_validate' => ['token_element_validate'],
      '#token_types' => ['system'],
    ];

    $form[self::UPLOAD_FORM]['TOKEN_HELP'] = [
      '#theme' => 'token_tree_link',
      '#token_type' => ['system'],
    ];

    $form[self::UPLOAD_FORM][self::UPLOAD_FORM_ALLOWED_MIMETYPES] = [
      '#type' => 'textarea',
      '#title' => $this->t('Allowed Mimetypes'),
      '#description' => $this->t('Add mimetypes as a space delimited list with no periods before the extension.'),
      '#default_value' => $config->get(self::UPLOAD_FORM_ALLOWED_MIMETYPES),
    ];

    $flysystem_config = Settings::get('flysystem');
    if ($flysystem_config != NULL) {
      $fedora_url = $flysystem_config['fedora']['config']['root'];
    }
    else {
      $fedora_url = NULL;
    }

    $form[self::FEDORA_URL] = [
      '#type' => 'textfield',
      '#title' => $this->t('Fedora URL'),
      '#attributes' => ['readonly' => 'readonly'],
      '#default_value' => $fedora_url,
    ];

    $selected_bundles = $config->get(self::GEMINI_PSEUDO);

    $options = [];
    foreach (['node', 'media', 'taxonomy_term'] as $content_entity) {
      $bundles = $this->entityTypeBundleInfo->getBundleInfo($content_entity);
      foreach ($bundles as $bundle => $bundle_properties) {
        $options["{$bundle}:{$content_entity}"] =
                $this->t('@label (@type)', [
                  '@label' => $bundle_properties['label'],
                  '@type' => $content_entity,
                ]);
      }
    }

    $form['bundle_container'] = [
      '#type' => 'details',
      '#title' => $this->t('Fedora URL Display'),
      '#description' => $this->t('Selected bundles can display the Fedora URL of repository content.'),
      '#open' => TRUE,
      self::GEMINI_PSEUDO => [
        '#type' => 'checkboxes',
        '#options' => $options,
        '#default_value' => $selected_bundles,
      ],
    ];

    $form['rdf_namespaces'] = [
      '#type' => 'link',
      '#title' => $this->t('Update RDF namespace configurations in the JSON-LD module settings.'),
      '#url' => Url::fromRoute('system.jsonld_settings'),
    ];

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    // Validate broker url by actually connecting with a stomp client.
    $brokerUrl = $form_state->getValue(self::BROKER_URL);
    // Attempt to subscribe to a dummy queue.
    try {
      $client = new Client($brokerUrl);
      if ($form_state->getValue('provide_user_creds')) {
        $broker_password = $form_state->getValue(self::BROKER_PASSWORD);
        // When stored password type fields aren't rendered again.
        if (!$broker_password) {
          // Use the stored password if it exists.
          if (!$this->brokerPassword) {
            $form_state->setErrorByName(self::BROKER_PASSWORD, $this->t('A password must be supplied'));
          }
          else {
            $broker_password = $this->brokerPassword;
          }
        }
        $client->setLogin($form_state->getValue(self::BROKER_USER), $broker_password);
      }
      $stomp = new StatefulStomp($client);
      $stomp->subscribe('dummy-queue-for-validation');
      $stomp->unsubscribe();
    }
    // Invalidate the form if there's an issue.
    catch (StompException $e) {
      $form_state->setErrorByName(
        self::BROKER_URL,
        $this->t(
          'Cannot connect to message broker at @broker_url',
          ['@broker_url' => $brokerUrl]
        )
      );
    }

    // Validate jwt expiry as a valid time string.
    $expiry = trim($form_state->getValue(self::JWT_EXPIRY));
    $expiry = strtolower($expiry);
    if (strtotime($expiry) === FALSE) {
      $form_state->setErrorByName(
        self::JWT_EXPIRY,
        $this->t(
          '"@expiry" is not a valid time or interval expression.',
          ['@expiry' => $expiry]
        )
      );
    }
    elseif (substr($expiry, 0, 1) == "-") {
      $form_state->setErrorByName(
        self::JWT_EXPIRY,
        $this->t('Time or interval expression cannot be negative')
          );
    }
    elseif (intval($expiry) === 0) {
      $form_state->setErrorByName(
        self::JWT_EXPIRY,
        $this->t('No numeric interval specified, for example "1 day"')
          );
    }
    else {
      if (!preg_match("/\b(" . implode("|", self::TIME_INTERVALS) . ")s?\b/", $expiry)) {
        $form_state->setErrorByName(
          self::JWT_EXPIRY,
          $this->t("No time interval found, please include one of (@int). Plurals are also accepted.",
            ['@int' => implode(", ", self::TIME_INTERVALS)]
          )
        );
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $config = $this->configFactory->getEditable(self::CONFIG_NAME);

    $new_pseudo_types = array_filter($form_state->getValue(self::GEMINI_PSEUDO));

    $broker_password = $form_state->getValue(self::BROKER_PASSWORD);

    // If there's no user set delete what may have been here before as password
    // fields will also be blank.
    if (!$form_state->getValue('provide_user_creds')) {
      $config->clear(self::BROKER_USER);
      $config->clear(self::BROKER_PASSWORD);
    }
    else {
      $config->set(self::BROKER_USER, $form_state->getValue(self::BROKER_USER));
      // If the password has changed update it as well.
      if ($broker_password && $broker_password != $this->brokerPassword) {
        $config->set(self::BROKER_PASSWORD, $broker_password);
      }
    }

    // Check for types being unset and remove the field from them first.
    $current_pseudo_types = $config->get(self::GEMINI_PSEUDO);
    $this->updateEntityViewConfiguration($current_pseudo_types, $new_pseudo_types);

    $config
      ->set(self::BROKER_URL, $form_state->getValue(self::BROKER_URL))
      ->set(self::JWT_EXPIRY, $form_state->getValue(self::JWT_EXPIRY))
      ->set(self::UPLOAD_FORM_LOCATION, $form_state->getValue(self::UPLOAD_FORM_LOCATION))
      ->set(self::UPLOAD_FORM_ALLOWED_MIMETYPES, $form_state->getValue(self::UPLOAD_FORM_ALLOWED_MIMETYPES))
      ->set(self::GEMINI_PSEUDO, $new_pseudo_types)
      ->save();

    parent::submitForm($form, $form_state);
  }

  /**
   * Removes the Fedora URI field from entity bundles that have be unselected.
   *
   * @param array $current_config
   *   The current set of entity types & bundles to have the pseudo field,
   *   format {bundle}:{entity_type}.
   * @param array $new_config
   *   The new set of entity types & bundles to have the pseudo field, format
   *   {bundle}:{entity_type}.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  private function updateEntityViewConfiguration(array $current_config, array $new_config) {
    $removed = array_diff($current_config, $new_config);
    $added = array_diff($new_config, $current_config);
    $entity_view_display = $this->entityTypeManager->getStorage('entity_view_display');
    foreach ($removed as $bundle_type) {
      [$bundle, $type_id] = explode(":", $bundle_type);
      $results = $entity_view_display->getQuery()
        ->condition('bundle', $bundle)
        ->condition('targetEntityType', $type_id)
        ->exists('content.' . self::GEMINI_PSEUDO_FIELD . '.region')
        ->execute();
      $entities = $entity_view_display->loadMultiple($results);
      foreach ($entities as $entity) {
        $entity->removeComponent(self::GEMINI_PSEUDO_FIELD);
        $entity->save();
      }
    }
    if (count($removed) > 0 || count($added) > 0) {
      // If we added or cleared a type then clear the extra_fields cache.
      // @see Drupal/Core/Entity/EntityFieldManager::getExtraFields
      Cache::invalidateTags(["entity_field_info"]);
    }
  }

}
