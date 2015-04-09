<?php

/**
 * @file
 * The admin/confirmation form to delete/purge a datastream.
 */

/**
 * The admin delete datastream form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractDatastream $datastream
 *   The datastream to be deleted.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_delete_datastream_form(array $form, array &$form_state, AbstractDatastream $datastream) {
  // XXX: Stashed version of datastream is deprecated... Use object and
  // datastream IDs from 'datastream_info' to acquire.
  $form_state['datastream'] = $datastream;
  $form_state['datastream_info'] = array(
    'object_id' => $datastream->parent->id,
    'datastream_id' => $datastream->id,
  );
  $object = $datastream->parent;
  $dsid = $datastream->id;
  $dsids = array_merge(array($dsid), islandora_datastream_to_purge($object, $dsid));
  $dsids = array_unique($dsids);
  $form['delete_derivatives'] = array(
    '#title' => t('Delete Derivatives'),
    '#type' => 'checkbox',
    '#default_value' => 0,
        '#description' => t('Derivatives can be regenerated at a later time.'),
  );
  $form['base_info'] = array(
    '#type' => 'item',
    '#title' => t('Datastream to be purged'),
    '#markup' => $dsid,
    '#states' => array(
      'invisible' => array(
        ':input[name="delete_derivatives"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['derivative_info'] = array(
    '#type' => 'item',
    '#title' => t('Datastream(s) to be purged'),
    '#description' => t('Including detectable derivatives.'),
    '#markup' => implode(', ', $dsids),
    '#states' => array(
      'visible' => array(
        ':input[name="delete_derivatives"]' => array('checked' => TRUE),
      ),
    ),
  );
  return confirm_form($form,
    t('Are you sure you want to delete the %dsid datastream?', array('%dsid' => $datastream->id)),
    "islandora/object/{$datastream->parent->id}",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Return $dsids based on a given DSID and filter.
 */
function islandora_datastream_to_purge(AbstractObject $object, $dsid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $hooks = islandora_invoke_hook_list(ISLANDORA_DERIVATIVE_CREATION_HOOK, $object->models, array($object));
  $hook_filter = function ($hook_def) use ($dsid) {
    return isset($hook_def['source_dsid']) && isset($hook_def['destination_dsid']) ?
      $hook_def['source_dsid'] == $dsid :
      FALSE;
  };
  $hooks = array_filter($hooks, $hook_filter);
  $dsid_map = function ($hook_definition) {
    return $hook_definition['destination_dsid'];
  };
  $dsids = array();
  $derived_dsids = array_map($dsid_map, $hooks);
  while ($current = array_pop($derived_dsids)) {
    $dsids[] = $current;
    $current_derived = islandora_datastream_to_purge($object, $current);
    $current_diff = array_diff($current_derived, $derived_dsids, $dsids);
    $derived_dsids = array_merge($derived_dsids, $current_diff);
  }
  return $dsids;
}

/**
 * Purge a given DSID's derivative datastreams on an object.
 */
function islandora_datastream_derivatives_purged(AbstractObject $object, $dsid) {
  $dsids = islandora_datastream_to_purge($object, $dsid);
  array_map(array($object, 'purgeDatastream'), $dsids);
}

/**
 * Submit handler for the delete datastream form.
 *
 * Purges/Delete's the given AbstractDatastream if possible.
 *
 * The ISLANDORA_PRE_PURGE_DATASTREAM_HOOK will query other modules as to
 * whether the given FedoraDatastream
 * should be: blocked from purging; state set to 'Deleted'; or purged.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_delete_datastream_form_submit(array $form, array &$form_state) {
  $object = islandora_object_load($form_state['datastream_info']['object_id']);
  $datastream_id = $form_state['datastream_info']['datastream_id'];
  $datastream = $object[$datastream_id];
  $deleted = FALSE;
  if ($form_state['values']['delete_derivatives']) {
    islandora_datastream_derivatives_purged($object, $datastream_id);
  }
  try {
    $deleted = islandora_delete_datastream($datastream);
  }
  catch (Exception $e) {
    drupal_set_message(t('Error deleting %s datastream from object %o %e', array(
          '%s' => $datastream_id,
          '%o' => $object->label,
          '%e' => $e->getMessage())), 'error');
  }
  if ($deleted) {
    drupal_set_message(t('%d datastream sucessfully deleted from Islandora object %o', array(
          '%d' => $datastream_id,
          '%o' => $object->label)));
  }
  else {
    drupal_set_message(t('Error deleting %s datastream from object %o', array(
          '%s' => $datastream_id,
          '%o' => $object->label)), 'error');
  }
  $form_state['redirect'] = "islandora/object/{$object->id}";
}
