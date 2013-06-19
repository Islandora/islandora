<?php

/**
 * @file
 * The admin/confirmation form to purge an object.
 */

/**
 * The admin delete object form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The object to be deleted.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_delete_object_form(array $form, array &$form_state, AbstractObject $object) {
  $form_state['object'] = $object;
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $object->label)),
    "islandora/object/$object->id",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete's the given object.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_delete_object_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $object = $form_state['object'];
  $parents = islandora_get_parents_from_rels_ext($object);
  $parent = array_pop($parents);
  $form_state['redirect'] = isset($parent) ? "islandora/object/{$parent->id}" : 'islandora';
  islandora_delete_object($object);
}
