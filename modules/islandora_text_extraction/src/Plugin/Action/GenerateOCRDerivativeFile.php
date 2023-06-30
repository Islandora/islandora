<?php

namespace Drupal\islandora_text_extraction\Plugin\Action;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;
use Drupal\islandora\Plugin\Action\AbstractGenerateDerivativeMediaFile;

/**
 * Generates OCR derivatives event.
 *
 * @Action(
 *   id = "generate_extracted_text_file",
 *   label = @Translation("Generate Extracted Text for Media Attachment"),
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
    $config['scheme'] = $this->config->get('default_scheme');
    $config['destination_text_field_name'] = '';
    $config['text_format'] = 'plain_text';
    return $config;
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $map = $this->entityFieldManager->getFieldMapByFieldType('text_long');
    $file_fields = $map['media'];
    $field_options = ['none' => $this->t('None')] + array_combine(array_keys($file_fields), array_keys($file_fields));
    $form = parent::buildConfigurationForm($form, $form_state);
    $form['mimetype']['#description'] = $this->t('Mimetype to convert to (e.g. application/xml, etc...)');
    $form['mimetype']['#value'] = 'text/plain';
    $form['mimetype']['#type'] = 'hidden';
    $position = array_search('destination_field_name', array_keys($form));
    $first = array_slice($form, 0, $position);
    $last = array_slice($form, count($form) - $position + 1);

    $middle['destination_text_field_name'] = [
      '#required' => FALSE,
      '#type' => 'select',
      '#options' => $field_options,
      '#title' => $this->t('Destination Text field Name'),
      '#default_value' => $this->configuration['destination_text_field_name'],
      '#description' => $this->t('Text field on Media Type to hold extracted text.'),
    ];
    $middle['text_format'] = [
      '#type' => 'select',
      '#title' => $this->t('Format'),
      '#options' => [
        'plain_text' => $this->t('Plain text'),
        'hocr' => $this->t('hOCR text with positional data'),
      ],
      '#default_value' => $this->configuration['text_format'],
      '#description' => $this->t("The type of text to be returned."),
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
        $this->t('Please enter file mimetype (e.g. application/xml.)')
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    parent::submitConfigurationForm($form, $form_state);
    $this->configuration['destination_text_field_name'] = $form_state->getValue('destination_text_field_name');
    $this->configuration['text_format'] = $form_state->getValue('text_format');
    switch ($form_state->getValue('text_format')) {
      case 'hocr':
        $this->configuration['args'] = '-c tessedit_create_hocr=1 -c hocr_font_info=0';
        break;

      case 'plain_text':
        $this->configuration['args'] = '';
        break;
    }
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
      'text_format' => $this->configuration['text_format'],
    ];
    $data['destination_uri'] = Url::fromRoute('islandora_text_extraction.attach_file_to_media', $route_params)
      ->setAbsolute()
      ->toString();

    return $data;
  }

}
