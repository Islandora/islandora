<?php

namespace Drupal\islandora_text_extraction\Plugin\Action;

use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\Plugin\Action\AbstractGenerateDerivative;

/**
 * Emits a Node for generating OCR derivatives event.
 *
 * @Action(
 *   id = "generate_ocr_derivative",
 *   label = @Translation("Get OCR from image"),
 *   type = "node"
 * )
 */
class GenerateOCRDerivative extends AbstractGenerateDerivative {

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    $config = parent::defaultConfiguration();
    $config['path'] = '[date:custom:Y]-[date:custom:m]/[node:nid]-[term:name].txt';
    $config['event'] = 'Generate Derivative';
    $config['source_term_uri'] = 'http://pcdm.org/use#OriginalFile';
    $config['derivative_term_uri'] = 'http://pcdm.org/use#ExtractedText';
    $config['mimetype'] = 'text/plain';
    $config['queue'] = 'islandora-connector-ocr';
    $config['destination_media_type'] = 'extracted_text';
    $config['scheme'] = 'fedora';
    return $config;
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildConfigurationForm($form, $form_state);

    $form['args']['#description'] = $this->t("Arguments to send to Tesseract. To generate hOCR, use:<br /><code>-c tessedit_create_hocr=1 -c hocr_font_info=0</code>");

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
        $this->t('Please enter file mimetype (e.g. text/plain.)')
      );
    }
  }

}
