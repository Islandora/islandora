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
  if (isset($form_state['islandora']['needs_confirmation'])) {
    return islandora_object_properties_confirm_form($form_state);
  }
  $form_state['object'] = $object;
  $temp = islandora_invoke_hook_list(ISLANDORA_UPDATE_RELATED_OBJECTS_PROPERTIES_HOOK, $object->models, array($object));
  $related_objects_pids = array();
  if (!empty($temp)) {
    $related_objects_pids = array_merge_recursive($related_objects_pids, $temp);
  }
  $regenerate_derivatives_access = FALSE;
  if (islandora_object_access(ISLANDORA_REGENERATE_DERIVATIVES, $object)) {
    module_load_include('inc', 'islandora', 'includes/derivatives');
    $hooks = islandora_invoke_hook_list(ISLANDORA_DERIVATIVE_CREATION_HOOK, $object->models, array($object));
    $hooks = islandora_filter_derivatives($hooks, array('force' => TRUE), $object);
    if (count($hooks) >= 1) {
      $regenerate_derivatives_access = TRUE;
    }
  }
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
    'propogate' => array(
      '#title' => t('Apply changes to related objects?'),
      '#default_value' => TRUE,
      '#description' => t("Changes to owner and state will applied to associated objects. ie page objects associated with a book object."),
      '#type' => 'checkbox',
      '#access' => count($related_objects_pids),
    ),
    'related_pids' => array(
      '#value' => $related_objects_pids,
      '#type' => 'hidden',
      '#access' => count($related_objects_pids),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => 'Update Properties',
    ),
    'delete' => array(
      '#type' => 'submit',
      '#access' => islandora_object_access(ISLANDORA_PURGE, $object),
      '#value' => t("Permanently remove '@label' from repository", array(
        '@label' => truncate_utf8($object->label, 32, TRUE, TRUE))
      ),
      '#submit' => array('islandora_object_properties_form_delete'),
      '#limit_validation_errors' => array(array('pid')),
    ),
    'regenerate' => array(
      '#type' => 'submit',
      '#access' => $regenerate_derivatives_access,
      '#value' => t("Regenerate all derivatives"),
      '#submit' => array('islandora_object_properties_regenerate_derivatives'),
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
  if (isset($form_state['islandora']['needs_confirmation'])) {
    $form_state['values'] = $form_state['islandora']['values'];
  }
  else {
    // Confirm if user is about to lock themselves out of this object.
    if (variable_get('islandora_deny_inactive_and_deleted', FALSE) && in_array($form_state['values']['object_state'], array('I', 'D'))) {
      if ($form_state['object']->state == 'A') {
        if (!user_access(ISLANDORA_ACCESS_INACTIVE_AND_DELETED_OBJECTS)) {
          $form_state['islandora']['needs_confirmation'] = TRUE;
          $form_state['islandora']['values'] = $form_state['values'];
          $form_state['rebuild'] = TRUE;
          return;
        }
      }
    }
  }
  $object = $form_state['object'];
  $owner = $form_state['values']['object_owner'];
  $state = $form_state['values']['object_state'];
  $label = $form_state['values']['object_label'];
  $propogate = $form_state['values']['propogate'];
  $update_owners = FALSE;
  $update_states = FALSE;
  if (isset($owner) && $owner != $object->owner) {
    try {
      $object->owner = $owner;
      $update_owners = TRUE;
      drupal_set_message(t('Successfully updated owner %s', array('%s' => $owner)));
    }
    catch (Exception $e) {
      form_set_error('object_owner', t('Error updating owner %s', array('%s' => $e->getMessage())));
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
  if (isset($state) && $state != $object->state) {
    try {
      $object->state = $state;
      $update_states = TRUE;
      drupal_set_message(t('Successfully updated state %s', array('%s' => $state)));
    }
    catch (Exception $e) {
      form_set_error('object_state', t('Error updating state %s', array('%s' => $e->getMessage())));
    }
  }
  if ($propogate && ($update_states || $update_owners)) {
    $related_objects_pids = $form_state['values']['related_pids'];
    $batch = array(
      'title' => t('Updating related objects'),
      'file' => drupal_get_path('module', 'islandora') . '/includes/object_properties.form.inc',
      'operations' => array(),
    );

    foreach ($related_objects_pids as $pid) {
      $batch['operations'][] = array(
        'islandora_update_object_properties',
        array($pid, $update_states, $state, $update_owners, $owner),
      );
    }
    batch_set($batch);
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

/**
 * Updates object state.
 *
 * @param string $pid
 *   PID of object to be updated
 * @param bool $update_states
 *   If TRUE, update object state
 * @param string $state
 *   Desired object state
 * @param bool $update_owners
 *   If TRUE, update Owner
 * @param string $owner
 *   New Owner
 */
function islandora_update_object_properties($pid, $update_states, $state, $update_owners, $owner) {
  $fedora_object = islandora_object_load($pid);
  if ($fedora_object) {
    if ($update_states) {
      $fedora_object->state = $state;
    }
    if ($update_owners) {
      $fedora_object->owner = $owner;
    }
  }
}

/**
 * Callback function for object properties regenerate all derivatives.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_object_properties_regenerate_derivatives(array $form, array &$form_state) {
  drupal_goto("islandora/object/{$form_state['object']}/regenerate");
}

/**
 * Confirmation form for object properties admin form.
 *
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_object_properties_confirm_form(array &$form_state) {
  $desc = t('You do not have permission to view Inactive or Deleted objects, so you will no longer be able to view or manage this object. Are you sure?');
  $path = "islandora/object/{$form_state['object']->id}/manage/properties";
  return confirm_form(array(),
    t('Are you sure you want to set the object state?'),
    $path,
    $desc,
    t('Continue'),
    t('Cancel'));
}
