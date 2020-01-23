<?php

namespace Drupal\islandora_text_extraction\Plugin\Action;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;
use Drupal\islandora\Plugin\Action\AbstractGenerateDerivativeMediaFile;

/**
 * Emits a Node for generating fits derivatives event.
 *
 * @Action(
 *   id = "generate_extracted_text_file",
 *   label = @Translation("Generate an Extracted Text derivative file"),
 *   type = "media"
 * )
 */
class GenerateOCRDerivativeFile extends AbstractGenerateDerivativeMediaFile {

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    $config = parent::defaultConfiguration();
    $config['path'] = '[date:custom:Y]-[date:custom:m]/[media:mid]-extracted_text.txt';
    $config['mimetype'] = 'application/xml';
    $config['queue'] = 'islandora-connector-ocr';
    $config['destination_media_type'] = 'file';
    $config['scheme'] = file_default_scheme();
    $config['destination_text_field_name'] = '';
    return $config;
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $map = $this->entityFieldManager->getFieldMapByFieldType('text_long');
    $file_fields = $map['media'];
    $field_options = array_combine(array_keys($file_fields), array_keys($file_fields));
    $form = parent::buildConfigurationForm($form, $form_state);
    $form['mimetype']['#description'] = t('Mimetype to convert to (e.g. application/xml, etc...)');
    $form['mimetype']['#value'] = 'text/plain';
    $form['mimetype']['#type'] = 'hidden';
    $position = array_search('destination_field_name', array_keys($form));
    $first = array_slice($form, 0, $position);
    $last = array_slice($form, count($form) - $position + 1);

    $middle['destination_text_field_name'] = [
      '#required' => TRUE,
      '#type' => 'select',
      '#options' => $field_options,
      '#title' => $this->t('Destination Text field Name'),
      '#default_value' => $this->configuration['destination_text_field_name'],
      '#description' => $this->t('Text field on Media Type to hold extracted text.'),
    ];
    $form = array_merge($first, $middle, $last);

    unset($form['args']);
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigurationForm(array &$form, FormStateInterface $form_state) {
    parent::validateConfigurationForm($form, $form_state);
    $exploded_mime = explode('/', $form_state->getValue('mimetype'));
    if ($exploded_mime[0] != 'text') {
      $form_state->setErrorByName(
        'mimetype',
        t('Please enter file mimetype (e.g. application/xml.)')
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    parent::submitConfigurationForm($form, $form_state);
    $this->configuration['destination_text_field_name'] = $form_state->getValue('destination_text_field_name');
  }

  /**
   * Override this to return arbitrary data as an array to be json encoded.
   */
  protected function generateData(EntityInterface $entity) {
    $data = parent::generateData($entity);
    $route_params = [
      'media' => $entity->id(),
      'destination_field' => $this->configuration['destination_field_name'],
      'destination_text_field' => $this->configuration['destination_text_field_name'],
    ];
    $data['destination_uri'] = Url::fromRoute('islandora_text_extraction.attach_file_to_media', $route_params)
      ->setAbsolute()
      ->toString();

    return $data;
  }

}
