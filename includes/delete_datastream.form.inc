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
  $form_state['datastream'] = $datastream;
  return confirm_form($form,
    t('Are you sure you want to delete the %dsid datastream?', array('%dsid' => $datastream->id)),
    "islandora/object/{$datastream->parent->id}",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
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
  $datastream = $form_state['datastream'];
  $datastream_id = $datastream->id;
  $object = $datastream->parent;
  $deleted = FALSE;
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
