<?php

/**
 * @file
 * This file contains the admin (confirmation) form and callback functions to purge an object.
 */

/**
 * Gives the option of deleting or purging and object.
 *
 * The default behaviour is to purge the object to reduce maintenance.
 * If a solution pack wants to change this behaviour and have the object set to deleted then
 * it can respond to the 'islandora_pre_purge_object' hook with an array containing the pair
 * 'delete' => TRUE.
 * Once the object has been deleted/purged then a second call lets the solution packs know that
 * the object has been dealt with. In this call the object id and content models are sent out so
 * that the solution packs can act on this news. There is no guarantee that the object still exists
 * and so the object object isn't sent.
 *
 * @param string $object_id
 *   ID of the object
 * @return type
 */
function islandora_purge_object_submit($form, &$form_state) {

  $object_id = $form_state['values']['pid'];
  $collection = $form_state['values']['col'];

  // purge object
  islandora_object_purge($object_id);

  drupal_goto($collection);
}

function islandora_purge_object($form, &$form_state, $object) {
  module_load_include('inc', 'islandora', 'includes/datastream');

  $parent = islandora_datastream_get_parents($object);
  $key = array_keys($parent);

  if (count($key) > 0) {
    $redirect = "islandora/object/$key[0]";
  }
  else {
    $redirect = "islandora";
  }

  $form['pid'] = array('#type' => 'value', '#value' => $object->id);
  $form['col'] = array('#type' => 'value', '#value' => $redirect);

  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $object->label)),
    "islandora/object/$object->id",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function islandora_purge_datastream($form, &$form_state, $object, $datastream_id) {
  module_load_include('inc', 'islandora', 'includes/datastream');

  $datastream = $object->getDatastream($datastream_id);

  $redirect = "islandora/object/$object->id";

  $form['pid'] = array('#type' => 'value', '#value' => $object->id);
  $form['dsid'] = array('#type' => 'value', '#value' => $datastream_id);
  $form['col'] = array('#type' => 'value', '#value' => $redirect);

  return confirm_form($form,
    t('Are you sure you want to delete the %dsid datastream?', array('%dsid' => $datastream->id)),
    "islandora/object/$object->id",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Gives the option of purging or deleting a datastream.
 *
 * The default behaviour is to purge the datastream but this can be overridden using the
 * 'islandora_pre_purge_datastream' hook. The returned array can include a 'block' => TRUE
 * pair which will prevent the datastream from being deleted if it particularly needed for
 * a certain function. Returning 'delete' => TRUE will cause the datastream to be put into
 * a deleted state.
 *
 * @param string $object_id
 *   ID of the object
 * @param string $datastream_id
 *   ID of the datastream
 *
 */
function islandora_purge_datastream_submit($form, &$form_state) {
  $object_id = $form_state['values']['pid'];
  $datastream_id = $form_state['values']['dsid'];

  if (!isset($datastream_id)) {
    drupal_set_message(t('Cannot remove datastream, datastream id not set'));
    return;
  }

  $object = islandora_object_load($object_id);

  if (!isset($object)) {
    drupal_set_message(t('Could not remove object, object not found'));
    return;
  }

  //notify modules of pending deletion so we can update rels etc
  $arr = module_invoke_all('islandora_pre_purge_datastream', $object[$datastream_id]);

  if (isset($arr['block']) && $arr['block']) {
    drupal_set_message(t('Purging of the %d datastream was blocked', array('%d' => $datastream_id)), 'warning');
    return;
  }
  if (isset($arr['delete']) && $arr['delete']) {
    try {
      $object[$datastream_id]->state = 'D';
    } catch (Exception $e) {
      drupal_set_message(t('Error deleting %s datastream from Islandora object %o %e', array('%s' => $datastream_id, '%o' => $object->id, '%e' => $e)), 'error');
      return;
    }
  }
  else {
    try {
      $object->purgeDatastream($datastream_id);
    } catch (Exception $e) {
      drupal_set_message(t('Error purging %s datastream from Islandora object %o %e', array('%s' => $datastream_id, '%o' => $object_id, '%e' => $e)), 'error');
      return;
    }
  }
  //notify modules post deletion
  module_invoke_all('islandora_post_purge_datastream', $object, $datastream_id);
  drupal_set_message(t('%d datastream sucessfully purged from Islandora object %o', array('%d' => $datastream_id, '%o' => $object->label)));
  drupal_goto('islandora/object/' . $object->id);
}
