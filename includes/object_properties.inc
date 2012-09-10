<?php

/**
 * @file
 * Contains admin form functions for object properties
 */

/**
 * Validate function for object properties admin form.
 *
 * @TODO: may want more validation here the only restrictions i see on
 * the object label and owner is the foxml schema says they should be
 * an xsd:string there maybe further restrictions such as length but they aren't
 * defined in the schema.
 * @param array $form
 * @param array $form_state
 * @return boolean
 */
function islandora_edit_properties_form_validate($form, &$form_state) {
  $islandora_object = islandora_object_load($form_state['values']['pid']);
  if (!isset($islandora_object)) {
    form_set_error('', t('Could not update properties object not found.'));
    return FALSE;
  }
}

/**
 * Submit function for object properties admin form.
 *
 * @param array $form
 * @param array $form_state
 */
function islandora_edit_properties_form_submit($form, &$form_state) {
  $islandora_object = islandora_object_load($form_state['values']['pid']);
  $owner = $form_state['values']['object_owner'];
  $state = $form_state['values']['object_state'];
  $label = $form_state['values']['object_label'];
  if (isset($owner) && $owner != $islandora_object->owner) {
    try {
      $islandora_object->owner = $owner;
      drupal_set_message(t('Successfully updated owner %s', array('%s' => $owner)));
    } catch (Exception $e) {
      form_set_error('object_owner', t('Error updating owner %s', array('%s' => $e->getMessage())));
    }
  }
  if (isset($state) && $state != $islandora_object->state) {
    try {
      $islandora_object->state = $state;
      drupal_set_message(t('Successfully updated state %s', array('%s' => $state)));
    } catch (Exception $e) {
      form_set_error('object_state', t('Error updating state %s', array('%s' => $e->getMessage())));
    }
  }
  if (isset($label) && $label != $islandora_object->label) {
    try {
      $islandora_object->label = $label;
      drupal_set_message(t('Successfully updated label %s', array('%s' => check_plain($label))));
    } catch (Exception $e) {
      form_set_error(t('Error updating label %s', array('%s' => $e->getMessage())));
    }
  }
}

/**
 * Callback function for object properties admin form delete button.
 */
function islandora_edit_properties_form_delete($form, &$form_state) {
  $islandora_object = $form_state['values']['pid'];
  drupal_goto("islandora/object/$islandora_object/delete");
}

/**
 * Object properties admin form.
 *
 * @param array $form
 * @param array $form_state
 * @param string $object_id
 *   an object id
 * @return array
 */
function islandora_edit_properties_form($form, &$form_state, $object) {
  $form = array();
  if (!isset($object)) {
    return NULL;
  }
  drupal_set_title($object->label);
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );

  $form['object_label'] = array(
    '#title' => t('Item Label'),
    '#default_value' => $object->label,
    '#required' => 'TRUE',
    '#description' => t('A Human readable label'),
    '#type' => 'textfield'
  );
  $form['object_owner'] = array(
    '#title' => t('Owner'),
    '#default_value' => $object->owner,
    '#required' => FALSE,
    '#description' => t('The owner id'),
    '#type' => 'textfield',
  );
  $form['object_state'] = array(
    '#title' => t('State'),
    '#default_value' => $object->state,
    '#required' => TRUE,
    '#description' => t('The items state one of active, inactive or deleted'),
    '#type' => 'select',
    '#options' => array('A' => 'Active', 'I' => 'Inactive', 'D' => 'Deleted'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Update Properties',
  );
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#submit' => array('islandora_edit_properties_form_delete'),
    '#limit_validation_errors' => array(array('pid')),
  );
  return $form;
}
