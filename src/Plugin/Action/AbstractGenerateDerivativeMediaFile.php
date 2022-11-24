<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;

/**
 * Emits a Media for generating derivatives event.
 *
 * Attaches the result as a file in a file field on the emitting
 * Media ("multi-file media").
 *
 * @Action(
 *   id = "generate_derivative_file",
 *   label = @Translation("Generate a Derivative File for Media Attachment"),
 *   type = "media"
 * )
 */
class AbstractGenerateDerivativeMediaFile extends AbstractGenerateDerivativeBase {

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
      throw new \RuntimeException("Entity {$entity->getEntityTypeId()} {$entity->id()} is not a media", 500);
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
    $destination_field = $this->configuration['destination_field_name'];
    $field = \Drupal::entityTypeManager()
      ->getStorage('field_storage_config')
      ->load("media.$destination_field");
    $scheme = $field->getSetting('uri_scheme');
    $path = $this->token->replace($data['path'], $token_data);
    $data['file_upload_uri'] = $scheme . '://' . $path;
    $allowed = [
      'queue',
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
    $form = parent::buildConfigurationForm($form, $form_state);

    $map = $this->entityFieldManager->getFieldMapByFieldType('file');
    $file_fields = $map['media'];
    $file_options = array_combine(array_keys($file_fields), array_keys($file_fields));

    $map = $this->entityFieldManager->getFieldMapByFieldType('image');
    $image_fields = $map['media'];
    $image_options = array_combine(array_keys($image_fields), array_keys($image_fields));

    $file_options = array_merge(['' => ''], $file_options, $image_options);

    // @todo figure out how to write to thumbnail, which is not a real field.
    //   see https://github.com/Islandora/islandora/issues/891.
    unset($file_options['thumbnail']);

    $form['event']['#disabled'] = 'disabled';

    $form['destination_field_name'] = [
      '#required' => TRUE,
      '#type' => 'select',
      '#options' => $file_options,
      '#title' => $this->t('Destination File field'),
      '#default_value' => $this->configuration['destination_field_name'],
      '#description' => $this->t('This Action stores a derivative file
       in a File or Image field on a media. The destination field
       must be an additional field, not the media\'s main storage field.
       Selected destination field must be present on the media.'),
    ];

    $form['args'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Additional arguments'),
      '#default_value' => $this->configuration['args'],
      '#rows' => '8',
      '#description' => $this->t('Additional command line arguments'),
    ];

    $form['mimetype'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Mimetype'),
      '#default_value' => $this->configuration['mimetype'],
      '#required' => TRUE,
      '#rows' => '8',
      '#description' => $this->t('Mimetype to convert to (e.g. image/jpeg, video/mp4, etc...)'),
    ];

    $form['path'] = [
      '#type' => 'textfield',
      '#title' => $this->t('File path'),
      '#default_value' => $this->configuration['path'],
      '#description' => $this->t('Path within the upload destination where files will be stored. Includes the filename and optional extension.'),
    ];
    $form['queue'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Queue name'),
      '#default_value' => $this->configuration['queue'],
      '#description' => $this->t('Queue name to send along to help routing events, CHANGE WITH CARE. Defaults to :queue', [
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
    $exploded = explode('/', $form_state->getValue('mimetype'));
    if (count($exploded) != 2) {
      $form_state->setErrorByName(
        'mimetype',
        $this->t('Please enter a mimetype (e.g. image/jpeg, video/mp4, audio/mp3, etc...)')
      );
    }

    if (empty($exploded[1])) {
      $form_state->setErrorByName(
        'mimetype',
        $this->t('Please enter a mimetype (e.g. image/jpeg, video/mp4, audio/mp3, etc...)')
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
