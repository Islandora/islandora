<?php

namespace Drupal\islandora_image\Plugin\Action;

use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\Plugin\Action\AbstractGenerateDerivativeMediaFile;

/**
 * Emits a Node for generating derivatives event.
 *
 * @Action(
 *   id = "generate_image_derivative_file",
 *   label = @Translation("Generate an Image Derivative for Media Attachment"),
 *   type = "media"
 * )
 */
class GenerateImageDerivativeFile extends AbstractGenerateDerivativeMediaFile {

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    $config = parent::defaultConfiguration();
    $config['path'] = '[date:custom:Y]-[date:custom:m]/[media:mid]-ImageService.jpg';
    $config['mimetype'] = 'application/xml';
    $config['queue'] = 'islandora-connector-houdini';
    $config['destination_media_type'] = 'file';
    $config['scheme'] = $this->config->get('default_scheme');
    return $config;
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildConfigurationForm($form, $form_state);
    $map = $this->entityFieldManager->getFieldMapByFieldType('image');
    $file_fields = $map['media'];
    $file_options = array_combine(array_keys($file_fields), array_keys($file_fields));
    $file_options = array_merge(['' => ''], $file_options);
    // @todo figure out how to write to thumbnail, which is not a real field.
    //   see https://github.com/Islandora/islandora/issues/891.
    unset($file_options['thumbnail']);

    $form['destination_field_name'] = [
      '#required' => TRUE,
      '#type' => 'select',
      '#options' => $file_options,
      '#title' => $this->t('Destination Image field Name'),
      '#default_value' => $this->configuration['destination_field_name'],
      '#description' => $this->t('Image field on Media to hold derivative.  Cannot be the same as source'),
    ];

    $form['mimetype']['#value'] = 'image/jpeg';
    $form['mimetype']['#type'] = 'hidden';
    return $form;
  }

}
