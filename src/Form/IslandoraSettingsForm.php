<?php

namespace Drupal\islandora\Form;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityTypeBundleInfo;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Site\Settings;
use Drupal\Core\Url;
use GuzzleHttp\Exception\ConnectException;
use Islandora\Crayfish\Commons\Client\GeminiClient;
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
  const JWT_EXPIRY = 'jwt_expiry';
  const GEMINI_URL = 'gemini_url';
  const GEMINI_PSEUDO = 'gemini_pseudo_bundles';
  const FEDORA_URL = 'fedora_url';

  /**
   * To list the available bundle types.
   *
   * @var \Drupal\Core\Entity\EntityTypeBundleInfo
   */
  private $entityTypeBundleInfo;

  /**
   * Constructs a \Drupal\system\ConfigFormBase object.
   *
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   The factory for configuration objects.
   * @param \Drupal\Core\Entity\EntityTypeBundleInfo $entity_type_bundle_info
   *   The EntityTypeBundleInfo service.
   */
  public function __construct(ConfigFactoryInterface $config_factory, EntityTypeBundleInfo $entity_type_bundle_info) {
    $this->setConfigFactory($config_factory);
    $this->entityTypeBundleInfo = $entity_type_bundle_info;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
          $container->get('config.factory'),
          $container->get('entity_type.bundle.info')
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

    $form[self::BROKER_URL] = [
      '#type' => 'textfield',
      '#title' => $this->t('Broker URL'),
      '#default_value' => $config->get(self::BROKER_URL),
    ];

    $form[self::JWT_EXPIRY] = [
      '#type' => 'textfield',
      '#title' => $this->t('JWT Expiry'),
      '#default_value' => $config->get(self::JWT_EXPIRY),
      '#description' => 'Eg: 60, "2 days", "10h", "7d". A numeric value is interpreted as a seconds count. If you use a string be sure you provide the time units (days, hours, etc), otherwise milliseconds unit is used by default ("120" is equal to "120ms").',
    ];

    $form[self::GEMINI_URL] = [
      '#type' => 'textfield',
      '#title' => $this->t('Gemini URL'),
      '#default_value' => $config->get(self::GEMINI_URL),
    ];

    $flysystem_config = Settings::get('flysystem');
    $fedora_url = $flysystem_config['fedora']['config']['root'];

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
      $stomp = new StatefulStomp(
        new Client(
          $brokerUrl
        )
      );
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
    $expiry = $form_state->getValue(self::JWT_EXPIRY);
    if (strtotime($expiry) === FALSE) {
      $form_state->setErrorByName(
        self::JWT_EXPIRY,
        $this->t(
          '"@expiry" is not a valid time or interval expression.',
          ['@expiry' => $expiry]
        )
      );
    }

    // Needed for the elseif below.
    $pseudo_types = array_filter($form_state->getValue(self::GEMINI_PSEUDO));

    // Validate Gemini URL by validating the URL.
    $geminiUrlValue = trim($form_state->getValue(self::GEMINI_URL));
    if (!empty($geminiUrlValue)) {
      try {
        $geminiUrl = Url::fromUri($geminiUrlValue);
        $client = GeminiClient::create($geminiUrlValue, $this->logger('islandora'));
        $client->findByUri('http://example.org');
      }
      // Uri is invalid.
      catch (\InvalidArgumentException $e) {
        $form_state->setErrorByName(
          self::GEMINI_URL,
          $this->t(
            'Cannot parse URL @url',
            ['@url' => $geminiUrlValue]
          )
        );
      }
      // Uri is not available.
      catch (ConnectException $e) {
        $form_state->setErrorByName(
              self::GEMINI_URL,
              $this->t(
                  'Cannot connect to URL @url',
                  ['@url' => $geminiUrlValue]
              )
          );
      }
    }
    elseif (count($pseudo_types) > 0) {
      $form_state->setErrorByName(
        self::GEMINI_URL,
        $this->t('Must enter Gemini URL before selecting bundles to display a pseudo field on.')
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $config = $this->configFactory->getEditable(self::CONFIG_NAME);

    $pseudo_types = array_filter($form_state->getValue(self::GEMINI_PSEUDO));

    $config
      ->set(self::BROKER_URL, $form_state->getValue(self::BROKER_URL))
      ->set(self::JWT_EXPIRY, $form_state->getValue(self::JWT_EXPIRY))
      ->set(self::GEMINI_URL, $form_state->getValue(self::GEMINI_URL))
      ->set(self::GEMINI_PSEUDO, $pseudo_types)
      ->save();

    parent::submitForm($form, $form_state);
  }

}
