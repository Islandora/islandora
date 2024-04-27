<?php

namespace Drupal\islandora_iiif\Form;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Component\Utility\UrlHelper;
use GuzzleHttp\ClientInterface;
use GuzzleHttp\Exception\ClientException;
use GuzzleHttp\Exception\ConnectException;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Form to configure IslandoraIIIF.
 */
class IslandoraIIIFConfigForm extends ConfigFormBase {

  /**
   * The HTTP client to fetch the files with.
   *
   * @var \GuzzleHttp\ClientInterface
   */
  protected $httpClient;

  /**
   * IslandoraIIIFConfigForm constructor.
   *
   * @param \GuzzleHttp\ClientInterface $http_client
   *   A Guzzle client object.
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   The config factory service.
   */
  public function __construct(ClientInterface $http_client, ConfigFactoryInterface $config_factory) {
    parent::__construct($config_factory);
    $this->httpClient = $http_client;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('http_client'),
      $container->get('config.factory')
    );
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      'islandora_iiif.settings',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_iiif_config_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $options = [
      'none' => $this->t('None'),
      'view' => $this->t("From view title"),
      'node' => $this->t("From node title"),
    ];
    $config = $this->config('islandora_iiif.settings');
    $form['iiif_server'] = [
      '#type' => 'url',
      '#title' => $this->t('IIIF Image server location'),
      '#description' => $this->t('Please enter the image server location without trailing slash. e.g. http://www.example.org/iiif/2.'),
      '#default_value' => $config->get('iiif_server'),
      '#config' => [
        'key' => 'islandora_iiif.settings:iiif_server',
      ],
    ];

    $form['use_relative_paths'] = [
      '#type' => 'checkbox',
      '#title' => $this->t("Use relative file paths in manifest."),
      '#description' => $this->t("Check this if your IIIF Server is configured to access files locally. If unchecked, the absolute URL will be given and the IIIF server will make requests to this site to retrieve images."),
      '#default_value' => $config->get('use_relative_paths'),
    ];

    $form['show_title'] = [
      '#type' => 'select',
      '#options' => $options,
      '#title' => $this->t("Show title in viewer."),
      '#description' => $this->t("Show title on your viewer, if viewer allows"),
      '#default_value' => $config->get('show_title'),
    ];

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    if (!empty($form_state->getValue('iiif_server'))) {
      $server = $form_state->getValue('iiif_server');
      if (!UrlHelper::isValid($server, UrlHelper::isExternal($server))) {
        $form_state->setErrorByName('iiif_server', "IIIF Server address is not a valid URL");
      }
      elseif (!$this->validateIiifUrl($server)) {
        $form_state->setErrorByName('iiif_server', "IIIF Server does not seem to be accessible.");
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);

    $this->config('islandora_iiif.settings')
      ->set('iiif_server', $form_state->getValue('iiif_server'))
      ->set('use_relative_paths', $form_state->getValue('use_relative_paths'))
      ->set('show_title', $form_state->getValue('show_title'))
      ->save();
  }

  /**
   * Ensure the IIIF server is accessible.
   *
   * @param string $server_uri
   *   The absolute or relative URI to the server.
   *
   * @return bool
   *   True if server returns 200 on a HEAD request.
   */
  private function validateIiifUrl($server_uri) {
    try {
      $result = $this->httpClient->head($server_uri);
      return ($result->getStatusCode() == 200);
    }
    catch (ClientException $e) {
      return FALSE;
    }
    catch (ConnectException $e) {
      return FALSE;
    }

  }

}
