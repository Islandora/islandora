<?php

/**
 * @file
 * Contains admin form functions for editing an object's properties.
 */

/**
 * Object properties admin form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The object whose properties this form will modify.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_object_properties_form(array $form, array &$form_state, AbstractObject $object) {
  drupal_set_title($object->label);
  $form_state['object'] = $object;
  return array(
    'pid' => array(
      '#type' => 'hidden',
      '#value' => $object->id,
    ),
    'object_label' => array(
      '#title' => t('Item Label'),
      '#default_value' => $object->label,
      '#required' => 'TRUE',
      '#description' => t('A human-readable label'),
      // Double the normal length.
      '#size' => 120,
      // Max length for a Fedora Label.
      '#maxlength' => 255,
      '#type' => 'textfield',
    ),
    // @todo Make this into an autocomplete field that list the users in the
    // system as well.
    'object_owner' => array(
      '#title' => t('Owner'),
      '#default_value' => $object->owner,
      '#required' => FALSE,
      '#description' => t("The owner's account name"),
      '#type' => 'textfield',
    ),
    'object_state' => array(
      '#title' => t('State'),
      '#default_value' => $object->state,
      '#required' => TRUE,
      '#description' => t("The object's state (active, inactive or deleted)"),
      '#type' => 'select',
      '#options' => array('A' => 'Active', 'I' => 'Inactive', 'D' => 'Deleted'),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => 'Update Properties',
    ),
    'delete' => array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('islandora_object_properties_form_delete'),
      '#limit_validation_errors' => array(array('pid')),
    ),
  );
}

/**
 * Submit handler for object properties admin form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_object_properties_form_submit(array $form, array &$form_state) {
  $object = $form_state['object'];
  $owner = $form_state['values']['object_owner'];
  $state = $form_state['values']['object_state'];
  $label = $form_state['values']['object_label'];
  if (isset($owner) && $owner != $object->owner) {
    try {
      $object->owner = $owner;
      drupal_set_message(t('Successfully updated owner %s', array('%s' => $owner)));
    }
    catch (Exception $e) {
      form_set_error('object_owner', t('Error updating owner %s', array('%s' => $e->getMessage())));
    }
  }
  if (isset($state) && $state != $object->state) {
    try {
      $object->state = $state;
      drupal_set_message(t('Successfully updated state %s', array('%s' => $state)));
    }
    catch (Exception $e) {
      form_set_error('object_state', t('Error updating state %s', array('%s' => $e->getMessage())));
    }
  }
  if (isset($label) && $label != $object->label) {
    try {
      $object->label = $label;
      drupal_set_message(t('Successfully updated label %s', array('%s' => check_plain($label))));
    }
    catch (Exception $e) {
      form_set_error(t('Error updating label %s', array('%s' => $e->getMessage())));
    }
  }
}

/**
 * Callback function for object properties admin form delete button.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_object_properties_form_delete(array $form, array &$form_state) {
  drupal_goto("islandora/object/{$form_state['values']['pid']}/delete");
}
