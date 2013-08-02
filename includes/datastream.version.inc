<?php
/**
 * @file
 * The form displayed that shows the datastream version history.
 */

/**
 * Function that returns markup for the datastream versions page.
 */
function islandora_datastream_version_table($datastream) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');

  drupal_set_title(t("@dsid Previous Versions", array('@dsid' => $datastream->id)));

  $header = array();
  $header[] = array('data' => t('Created Date'));
  $header[] = array('data' => t('Size'));
  $header[] = array('data' => t('Label'));
  $header[] = array('data' => t('Mime type'));
  $header[] = array('data' => t('Operations'));
  $rows = array();

  foreach ($datastream as $version => $datastream_version) {
    $row = array();
    $row[] = array(
      'class' => 'datastream-date',
      'data' => theme('islandora_datastream_view_link', array(
        'datastream' => $datastream,
        'label' => $datastream_version->createdDate->format(DATE_RFC850),
        'version' => $version,
      )),
    );
    $row[] = array(
      'class' => 'datastream-size',
      'data' => islandora_datastream_get_human_readable_size($datastream_version),
    );
    $row[] = array(
      'class' => 'datastream-label',
      'data' => $datastream_version->label,
    );
    $row[] = array(
      'class' => 'datastream-mime',
      'data' => $datastream_version->mimeType,
    );
    $row[] = array(
      'class' => 'datastream-delete',
      'data' => theme('islandora_datastream_delete_link', array(
        'datastream' => $datastream,
        'version' => $version,
      )),
    );
    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

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
function islandora_delete_datastream_version_form(array $form, array &$form_state, AbstractDatastream $datastream, $version) {
  if (!isset($datastream[$version]) || count($datastream) < 2) {
    return drupal_not_found();
  }

  $form_state['datastream'] = $datastream;
  $form_state['version'] = $version;
  return confirm_form($form,
    t('Are you sure you want to delete version @version of the @dsid datastream?', array('@dsid' => $datastream->id, '@version' => $version)),
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
function islandora_delete_datastream_version_form_submit(array $form, array &$form_state) {
  $datastream = $form_state['datastream'];
  $version = $form_state['version'];

  $datastream_id = $datastream->id;
  $object = $datastream->parent;

  try {
    unset($datastream[$version]);
  }
  catch (Exception $e) {
    drupal_set_message(t('Error deleting version %v of %s datastream from object %o %e', array(
          '%v' => $version,
          '%s' => $datastream_id,
          '%o' => $object->label,
          '%e' => $e->getMessage())), 'error');
  }

  drupal_set_message(t('%d datastream version successfully deleted from Islandora object %o', array(
        '%d' => $datastream_id,
        '%o' => $object->label)));

  $form_state['redirect'] = "islandora/object/{$object->id}/datastream/{$datastream->id}/version";
}
