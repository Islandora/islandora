<?php

/**
 * @file
 * The confirmation forms used to regenerate derivatives.
 */

/**
 * Regenerate a single datastream derivative confirm form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractDatastream $datastream
 *   The datastream to have derivatives regenerated for.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_regenerate_datastream_derivative_form(array $form, array &$form_state, AbstractDatastream $datastream) {
  $form_state['datastream'] = $datastream;
  return confirm_form($form,
    t('Are you sure you want to regenerate the derivative for the %dsid datastream?', array('%dsid' => $datastream->id)),
    "islandora/object/{$datastream->parent->id}/manage/datastreams",
    t('This will create a new version of the datastream. Please wait while this happens.'),
    t('Regenerate'),
    t('Cancel')
  );
}

/**
 * Submit handler for the regenerate datastream derivative form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_regenerate_datastream_derivative_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/derivatives');
  $datastream = $form_state['datastream'];
  $batch = islandora_regenerate_datastream_derivative_batch($datastream);
  batch_set($batch);
  $form_state['redirect'] = "islandora/object/{$datastream->parent->id}/manage/datastreams";
}

/**
 * Regenerate all derivatives on an object.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The object that is having its derivatives regenerated.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_regenerate_object_derivatives_form(array $form, array &$form_state, AbstractObject $object) {
  $form_state['object'] = $object;
  return confirm_form($form,
    t('Are you sure you want to regenerate all the derivatives for %title?', array('%title' => $object->label)),
    "islandora/object/{$object->id}/manage/properties",
    t('This will create a new version for every datastream on the object. Please wait while this happens.'),
    t('Regenerate'),
    t('Cancel')
  );
}

/**
 * Submit handler for the regenerate object derivativse form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_regenerate_object_derivatives_form_submit(array $form, array &$form_state) {
  $object = $form_state['object'];
  $batch = islandora_regenerate_object_derivatives_batch($object);
  batch_set($batch);
  $form_state['redirect'] = "islandora/object/{$object->id}/manage/properties";
}

/**
 * Creates a batch to go out and re-create all of the derivatives for an object.
 *
 * @param AbstractObject $object
 *   A AbstractObject representing an object within Fedora.
 *
 * @return array
 *   An array specifying the Drupal batch.
 */
function islandora_regenerate_object_derivatives_batch(AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/derivatives');
  return array(
    'title' => t('Regenerating all derivatives for @label', array('@label' => $object->label)),
    'operations' => islandora_do_batch_derivatives($object, array('force' => TRUE)),
    'init_message' => t('Preparing to regenerate derivatives...'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora') . '/includes/regenerate_derivatives.form.inc',
    'finished' => 'islandora_regenerate_derivative_batch_finished',
  );
}

/**
 * Creates a batch to go out and re-create the derivative for a datastream.
 *
 * @param AbstractDatastream $datastream
 *   A AbstractDatastream representing a datastream on an object within Fedora.
 *
 * @return array
 *   An array specifying the Drupal batch.
 */
function islandora_regenerate_datastream_derivative_batch(AbstractDatastream $datastream) {
  module_load_include('inc', 'islandora', 'includes/derivatives');
  return array(
    'title' => t('Regenerating derivatives for the @dsid datastream', array('@dsid' => $datastream->id)),
    'operations' => islandora_do_batch_derivatives($datastream->parent, array(
        'force' => TRUE,
        'destination_dsid' => $datastream->id,
    )),
    'init_message' => t('Preparing to regenerate derivatives...'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora') . '/includes/regenerate_derivatives.form.inc',
    'finished' => 'islandora_regenerate_derivative_batch_finished',
  );
}

/**
 * Wrapper to call out to batch operations.
 *
 * @param string $function
 *   The name of the function we are calling for derivatives.
 * @param bool|string $file
 *   FALSE if there is no file to load, the path to require otherwise
 * @param string $pid
 *   The pid of the object we are performing.
 * @param bool $force
 *   Whether we are forcing derivative regeneration or not.
 * @param array $context
 *   The context of the current batch operation.
 */
function islandora_derivative_perform_batch_operation($function, $file, $pid, $force, &$context) {
  if ($file) {
    require_once $file;
  }
  if (function_exists($function)) {
    $logging = call_user_func($function, islandora_object_load($pid), $force);
    if (!empty($logging)) {
      $context['results']['logging'][] = $logging;
    }
  }
  else {
    watchdog('islandora', 'Unable to call derivative function @function as it was not found!', array('@function' => $function), WATCHDOG_ERROR);
  }
}

/**
 * Finished function for derivative batch regeneration.
 *
 * @param array $success
 *   An array of success passed from the batch.
 * @param array $results
 *   An array of results passed from the batch.
 * @param array $operations
 *   An array of operations passed from the batch.
 */
function islandora_regenerate_derivative_batch_finished($success, $results, $operations) {
  module_load_include('inc', 'islandora', 'includes/derivatives');
  if (!empty($results['logging'])) {
    islandora_derivative_logging($results['logging']);
  }
}
