<?php

namespace Drupal\islandora_microservice_rewrite\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Configure Islandora microservice rewrite settings.
 */
class MicroserviceRewriteSettingsForm extends ConfigFormBase {

  /**
   * Config settings.
   *
   * @var string
   */
  const SETTINGS = 'islandora_microservice_rewrite.settings';

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_microservice_rewrite_settings';
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      static::SETTINGS,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $config = $this->config(static::SETTINGS);

    $form['rewrite_rules'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Microservice URL rewrites'),
      '#default_value' => $config->get('rewrite_rules'),
      '#rows' => 8,
      '#description' => $this->t('Enter one rewrite rule per line in the format find|replace. These replacements are applied to generated derivative message fields such as source_uri, destination_uri, and file_upload_uri before the message is serialized. Example: https://repository.example.edu|http://drupal.internal'),
    ];

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $this->configFactory->getEditable(static::SETTINGS)
      ->set('rewrite_rules', $form_state->getValue('rewrite_rules'))
      ->save();

    parent::submitForm($form, $form_state);
  }

}
