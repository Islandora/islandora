<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;
use Drupal\islandora\Exception\IslandoraDerivativeException;

/**
 * Emits a Node event.
 */
class AbstractGenerateDerivative extends AbstractGenerateDerivativeBase {

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return [
      'queue' => 'islandora-connector-houdini',
      'event' => 'Generate Derivative',
      'source_term_uri' => '',
      'derivative_term_uri' => '',
      'mimetype' => '',
      'args' => '',
      'destination_media_type' => '',
      'scheme' => $this->config->get('default_scheme'),
      'path' => '[date:custom:Y]-[date:custom:m]/[node:nid].bin',
    ];
  }

  /**
   * Override this to return arbitrary data as an array to be json encoded.
   */
  protected function generateData(EntityInterface $entity) {
    $data = parent::generateData($entity);

    // Find media belonging to node that has the source term, and set its file
    // url in the data array.
    $source_term = $this->utils->getTermForUri($this->configuration['source_term_uri']);
    if (!$source_term) {
      throw new \RuntimeException("Could not locate source term with uri: " . $this->configuration['source_term_uri'], 500);
    }

    $source_media = $this->utils->getMediaWithTerm($entity, $source_term);
    if (!$source_media) {
      throw new \RuntimeException("Could not locate source media", 500);
    }

    $source_file = $this->mediaSource->getSourceFile($source_media);
    if (!$source_file) {
      throw new \RuntimeException("Could not locate source file for media {$source_media->id()}", 500);
    }

    $data['source_uri'] = $this->utils->getDownloadUrl($source_file);

    // Find the term for the derivative and use it to set the destination url
    // in the data array.
    $derivative_term = $this->utils->getTermForUri($this->configuration['derivative_term_uri']);
    if (!$derivative_term) {
      throw new \RuntimeException("Could not locate taxonomy term with uri: " . $this->configuration['derivative_term_uri'], 500);
    }

    // See if there is a destination media already set, and abort if it's the
    // same as the source media. Dont cause an error, just don't continue.
    $derivative_media = $this->utils->getMediaWithTerm($entity, $derivative_term);
    if (!is_null($derivative_media) && $derivative_media->id() == $source_media->id()) {
      throw new IslandoraDerivativeException("Halting derivative, as source and target media are the same. Derivative term: [" . $this->configuration['derivative_term_uri'] . "] Source term: [" . $this->configuration['source_term_uri'] . "] Node id: [" . $entity->id() . "].", 500);
    }

    $route_params = [
      'node' => $entity->id(),
      'media_type' => $this->configuration['destination_media_type'],
      'taxonomy_term' => $derivative_term->id(),
    ];
    $data['destination_uri'] = Url::fromRoute('islandora.media_source_put_to_node', $route_params)
      ->setAbsolute()
      ->toString();

    $token_data = [
      'node' => $entity,
      'media' => $source_media,
      'term' => $derivative_term,
    ];
    $path = $this->token->replace($data['path'], $token_data);
    $data['file_upload_uri'] = $data['scheme'] . '://' . $path;

    // Get rid of some config so we just pass along
    // what the camel route and microservice need.
    unset($data['source_term_uri']);
    unset($data['derivative_term_uri']);
    unset($data['path']);
    unset($data['scheme']);
    unset($data['destination_media_type']);

    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $schemes = $this->utils->getFilesystemSchemes();
    $scheme_options = array_combine($schemes, $schemes);

    $form = parent::buildConfigurationForm($form, $form_state);
    $form['event']['#disabled'] = 'disabled';

    $form['source_term'] = [
      '#type' => 'entity_autocomplete',
      '#target_type' => 'taxonomy_term',
      '#title' => $this->t('Source term'),
      '#default_value' => $this->utils->getTermForUri($this->configuration['source_term_uri']),
      '#required' => TRUE,
      '#description' => $this->t('Term indicating the source media'),
    ];
    $form['derivative_term'] = [
      '#type' => 'entity_autocomplete',
      '#target_type' => 'taxonomy_term',
      '#title' => $this->t('Derivative term'),
      '#default_value' => $this->utils->getTermForUri($this->configuration['derivative_term_uri']),
      '#required' => TRUE,
      '#description' => $this->t('Term indicating the derivative media'),
    ];
    $form['destination_media_type'] = [
      '#type' => 'entity_autocomplete',
      '#target_type' => 'media_type',
      '#title' => $this->t('Derivative media type'),
      '#default_value' => $this->getEntityById($this->configuration['destination_media_type']),
      '#required' => TRUE,
      '#description' => $this->t('The Drupal media type to create with this derivative, can be different than the source'),
    ];
    $form['mimetype'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Mimetype'),
      '#default_value' => $this->configuration['mimetype'],
      '#required' => TRUE,
      '#rows' => '8',
      '#description' => $this->t('Mimetype to convert to (e.g. image/jpeg, video/mp4, etc...)'),
    ];
    $form['args'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Additional arguments'),
      '#default_value' => $this->configuration['args'],
      '#rows' => '8',
      '#description' => $this->t('Additional command line arguments'),
    ];
    $form['scheme'] = [
      '#type' => 'select',
      '#title' => $this->t('File system'),
      '#options' => $scheme_options,
      '#default_value' => $this->configuration['scheme'],
      '#required' => TRUE,
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

    $exploded_mime = explode('/', $form_state->getValue('mimetype'));

    if (count($exploded_mime) != 2) {
      $form_state->setErrorByName(
        'mimetype',
        $this->t('Please enter a mimetype (e.g. image/jpeg, video/mp4, audio/mp3, etc...)')
      );
    }

    if (empty($exploded_mime[1])) {
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

    $tid = $form_state->getValue('source_term');
    $term = $this->entityTypeManager->getStorage('taxonomy_term')->load($tid);
    $this->configuration['source_term_uri'] = $this->utils->getUriForTerm($term);

    $tid = $form_state->getValue('derivative_term');
    $term = $this->entityTypeManager->getStorage('taxonomy_term')->load($tid);
    $this->configuration['derivative_term_uri'] = $this->utils->getUriForTerm($term);

    $this->configuration['mimetype'] = $form_state->getValue('mimetype');
    $this->configuration['args'] = $form_state->getValue('args');
    $this->configuration['scheme'] = $form_state->getValue('scheme');
    $this->configuration['path'] = trim($form_state->getValue('path'), '\\/');
    $this->configuration['destination_media_type'] = $form_state->getValue('destination_media_type');
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
