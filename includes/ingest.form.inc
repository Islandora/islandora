<?php

/**
 * @file
 * Defines the multi-page ingest form and any relevant hooks and functions.
 */

/**
 * Checks if the given configuration can be used to display the ingest form.
 *
 * @param array $configuration
 *   The list of key/value pairs of configuration.
 *
 * @return bool
 *   TRUE if the give configuration defines one or more form steps, FALSE
 *   otherwise.
 */
function islandora_ingest_can_display_ingest_form(array $configuration) {
  $form_state = array();
  islandora_ingest_form_init_form_state_storage($form_state, $configuration);
  $form_steps = islandora_ingest_form_get_form_steps($form_state);
  // Forget the stubbed steps for the remainder of this request.
  drupal_static_reset('islandora_ingest_form_get_steps');
  return count($form_steps) > 0;
}

/**
 * Ingest form build function.
 *
 * Initializes the form state, and builds the initial list of steps, excutes
 * the current step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $configuration
 *   An associative array of configuration values that are used to build the
 *   list of steps to be executed, including:
 *   - id: The PID with which the object should be created.
 *   - namespace: The PID namespace in which the object should be created.
 *     (id is used first, if it is given).
 *   - label: The initial label for the object. Defaults to "New Object".
 *   - collections: An array of collection PIDs, to which the new object should
 *     be related.
 *   - models: An array of content model PIDs, to which the new object might
 *     subscribe
 *   - parent: The parent of the child to be ingested. This is needed for XACML
 *     to correctly apply the parent's POLICY to children.
 *
 * @return array
 *   The form definition of the current step.
 */
function islandora_ingest_form(array $form, array &$form_state, array $configuration) {
  try {
    islandora_ingest_form_init_form_state_storage($form_state, $configuration);
    return islandora_ingest_form_execute_step($form, $form_state);
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    return array(array(
        '#markup' => l(t('Back'), 'javascript:window.history.back();', array('external' => TRUE))));
  }
}

/**
 * Initializes the form_state storage for use in the ingest multi-page forms.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param array $configuration
 *   A list of key value pairs that are used to build the list of steps to be
 *   executed.
 */
function islandora_ingest_form_init_form_state_storage(array &$form_state, array $configuration) {
  if (empty($form_state['islandora'])) {
    $objects = isset($configuration['objects']) ? $configuration['objects'] : array();
    if (empty($objects)) {
      $objects[] = islandora_ingest_form_prepare_new_object($configuration);
    }
    $configuration['models'] = isset($configuration['models']) ? $configuration['models'] : array();
    // Make sure the models actually exist.
    foreach ($configuration['models'] as $key => $model) {
      if (!islandora_object_load($model)) {
        unset($configuration['models'][$key]);
      }
    }
    // No need to persist the 'objects' within the configuration.
    unset($configuration['objects']);
    // Required for step hooks.
    $form_state['islandora'] = array(
      'step_id' => NULL,
      'objects' => $objects,
      'shared_storage' => $configuration,
      'step_storage' => array(),
    );
    // Must be called after $form_state['islandora'] is initialized, otherwise,
    // the values in 'islandora' would not be availible to the step hooks.
    $form_state['islandora']['step_id'] = islandora_ingest_form_get_first_step_id($form_state);
  }
}

/**
 * Get the first step ID.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return string
 *   The 'id' of the very first step.
 */
function islandora_ingest_form_get_first_step_id(array &$form_state) {
  $steps = islandora_ingest_form_get_steps($form_state);
  $keys = array_keys($steps);
  return array_shift($keys);
}

/**
 * Get the last step ID.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return string
 *   The 'id' of the very last step.
 */
function islandora_ingest_form_get_last_step_id(array &$form_state) {
  $steps = islandora_ingest_form_get_steps($form_state);
  $keys = array_keys($steps);
  return array_pop($keys);
}

/**
 * Prepares a new object based on the given configuration.
 *
 * @param array $configuration
 *   The list of key/value pairs of configuration.
 *
 * @return NewFedoraObject
 *   The new object.
 */
function islandora_ingest_form_prepare_new_object(array $configuration) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (empty($configuration['object'])) {
    $message = islandora_deprecated('7.x-1.2', t('Please use "objects" as the default ingest form configuration property.'));
    trigger_error(filter_xss($message), E_USER_DEPRECATED);
    // ID is more specific than namespace so it will take precedence.
    $id = isset($configuration['namespace']) ? $configuration['namespace'] : 'islandora';
    $id = isset($configuration['id']) ? $configuration['id'] : $id;
    $label = isset($configuration['label']) ? $configuration['label'] : 'New Object';
    $relationship_map = function($o) {
      return array('relationship' => 'isMemberOfCollection', 'pid' => $o);
    };
    $relationships = empty($configuration['collections']) ? array() : array_map($relationship_map, $configuration['collections']);
    return islandora_prepare_new_object($id, $label, array(), array(), $relationships);
  }
  return $configuration['object'];
}

/**
 * Gets the given/current step.
 *
 * If the current step is not defined it's assumed that all steps have executed.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param string $step_id
 *   The ID of the step. The current step is returned if no step ID is given.
 *
 * @return array
 *   The given/current step if found, NULL otherwise.
 */
function islandora_ingest_form_get_step(array &$form_state, $step_id = NULL) {
  $step_id = isset($step_id) ? $step_id : islandora_ingest_form_get_current_step_id($form_state);
  $steps = islandora_ingest_form_get_steps($form_state);
  if (isset($step_id) && isset($steps[$step_id])) {
    return $steps[$step_id];
  }
  return NULL;
}

/**
 * Gets the next step.
 *
 * If the current step is not defined it's assumed that all steps have executed.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   The step relative to the result, if not provided the current step is used.
 *
 * @return array|null
 *   The next step if found, NULL otherwise.
 */
function islandora_ingest_form_get_next_step(array &$form_state, array $step = NULL) {
  $step = isset($step) ? $step : islandora_ingest_form_get_step($form_state);
  $next_step_id = islandora_ingest_form_get_next_step_id($form_state, $step['id']);
  return isset($next_step_id) ? islandora_ingest_form_get_step($form_state, $next_step_id) : NULL;
}

/**
 * Gets the previous step.
 *
 * If the current step is not defined it's assumed that all steps have executed.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   The step relative to the result, if not provided the current step is used.
 *
 * @return array|null
 *   The next step if found, NULL otherwise.
 */
function islandora_ingest_form_get_previous_step(array &$form_state, array $step = NULL) {
  $step = isset($step) ? $step : islandora_ingest_form_get_step($form_state);
  $previous_step_id = islandora_ingest_form_get_previous_step_id($form_state, $step['id']);
  return isset($previous_step_id) ? islandora_ingest_form_get_step($form_state, $previous_step_id) : NULL;
}

/**
 * Gets the ID of the current step.
 *
 * If the current step is not defined it's assumed that all steps have executed.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return string
 *   The step ID.
 */
function islandora_ingest_form_get_current_step_id(array &$form_state) {
  return $form_state['islandora']['step_id'];
}

/**
 * Gets the ID of the next step.
 *
 * If the current step is not defined it's assumed that all steps have executed.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param string $step_id
 *   The ID of the  step relative to the result, if not provided the current
 *   step_id is used.
 *
 * @return string
 *   The next step ID if found, NULL otherwise.
 */
function islandora_ingest_form_get_next_step_id(array &$form_state, $step_id = NULL) {
  $step_id = isset($step_id) ? $step_id : islandora_ingest_form_get_current_step_id($form_state);
  $step_ids = array_keys(islandora_ingest_form_get_steps($form_state));
  $index = array_search($step_id, $step_ids);
  $count = count($step_ids);
  if ($index !== FALSE && ++$index < $count) {
    return $step_ids[$index];
  }
  return NULL;
}

/**
 * Gets the ID of the previous step.
 *
 * If the current step is not defined it's assumed that all steps have executed.
 * In such cases the last step will be returned.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param string $step_id
 *   The ID of the step relative to the result, if not provided the current
 *   step_id is used.
 *
 * @return string
 *   The previous step ID if found, NULL otherwise.
 */
function islandora_ingest_form_get_previous_step_id(array &$form_state, $step_id = NULL) {
  $step_id = isset($step_id) ? $step_id : islandora_ingest_form_get_current_step_id($form_state);
  // If the current step is not defined it's assumed that all steps have
  // executed. So return the very last step.
  if ($step_id == NULL) {
    return islandora_ingest_form_get_last_step_id($form_state);
  }
  $step_ids = array_keys(islandora_ingest_form_get_steps($form_state));
  $index = array_search($step_id, $step_ids);
  if ($index !== FALSE && --$index >= 0) {
    return $step_ids[$index];
  }
  return NULL;
}

/**
 * Increments the current step if possible.
 *
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_increment_step(array &$form_state) {
  // When going to the next step rebuild the list of steps as the submit
  // of the current step could have added/removed a step.
  drupal_static_reset('islandora_ingest_form_get_steps');
  $next_step_id = islandora_ingest_form_get_next_step_id($form_state);
  islandora_ingest_form_stash_info($form_state);
  $form_state['islandora']['step_id'] = $next_step_id;
  islandora_ingest_form_grab_info($form_state);
}

/**
 * Decrement the current step if possible.
 *
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_decrement_step(array &$form_state) {
  $previous_step_id  = islandora_ingest_form_get_previous_step_id($form_state);
  // Don't decrement passed the first step.
  if (isset($previous_step_id)) {
    islandora_ingest_form_stash_info($form_state);
    $form_state['islandora']['step_id'] = $previous_step_id;
    islandora_ingest_form_grab_info($form_state);
  }
}

/**
 * Executes the current step.
 *
 * Builds the form definition and appends on any additonal elements required
 * for the step to function.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param string $step_id
 *   The ID of the step relative to the result, if not provided the current
 *   step_id is used.
 *
 * @return array
 *   The form definition of the current step.
 */
function islandora_ingest_form_execute_step(array $form, array &$form_state, $step_id = NULL) {
  // Load any required files for the current step.
  islandora_ingest_form_load_include($form_state);
  $step = isset($step_id) ? islandora_ingest_form_get_step($form_state) : islandora_ingest_form_get_step($form_state, $step_id);
  switch ($step['type']) {
    case 'callback':
      // Execute all the consecutive callbacks, and move then attempt to process
      // the next step.
      islandora_ingest_form_execute_consecutive_callback_steps($form, $form_state, $step);
      return islandora_ingest_form_execute_step($form, $form_state);

    case 'form':
      return islandora_ingest_form_execute_form_step($form, $form_state, $step);

    case 'batch':
      // @todo Implement if possible.
      break;
  }
  return array();
}

/**
 * Execute the given 'form' step.
 *
 * Assumes the given step is a 'form' step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   The step we are executing.
 *
 * @return array
 *   The form definition of the given step.
 */
function islandora_ingest_form_execute_form_step(array $form, array &$form_state, array $step) {
  $args = array($form, &$form_state);
  $args = isset($step['args']) ? array_merge($args, $step['args']) : $args;
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);
  // Build the form step.
  $form = call_user_func_array($step['form_id'], $args);
  // Since the list of steps depends on the shared storage we will rebuild the
  // list of steps if the shared storage has changed. This must be done before
  // stepifying, so the prev/next buttons get updated.
  if ($shared_storage != islandora_ingest_form_get_shared_storage($form_state)) {
    drupal_static_reset('islandora_ingest_form_get_steps');
  }
  return islandora_ingest_form_stepify($form, $form_state, $step);
}

/**
 * Execute the given 'callback' step and any consecutive 'callback' steps.
 *
 * Assumes the given step is a 'callback' step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   The step that execution begins from.
 */
function islandora_ingest_form_execute_consecutive_callback_steps(array $form, array &$form_state, array $step) {
  do {
    islandora_ingest_form_execute_callback_step($form, $form_state, $step);
    islandora_ingest_form_increment_step($form_state);
    $step = islandora_ingest_form_get_step($form_state);
  } while (isset($step) && $step['type'] == 'callback');
}

/**
 * Execute the given 'callback' step.
 *
 * Assumes the given step is a 'callback' step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   The step currently being executed.
 */
function islandora_ingest_form_execute_callback_step(array $form, array &$form_state, array $step) {
  $args = array(&$form_state);
  $args = isset($step['do_function']['args']) ? array_merge($args, $step['do_function']['args']) : $args;
  call_user_func_array($step['do_function']['function'], $args);
}

/**
 * Undo the given 'callback' step and any consecutive 'callback' steps.
 *
 * Assumes the given $step is a 'callback' step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   The step that execution begins from.
 */
function islandora_ingest_form_undo_consecutive_callback_steps(array $form, array &$form_state, array $step) {
  do {
    islandora_ingest_form_undo_callback_step($form, $form_state, $step);
    islandora_ingest_form_decrement_step($form_state);
    $step = islandora_ingest_form_get_step($form_state);
  } while (isset($step) && $step['type'] == 'callback');
}

/**
 * Undo the given 'callback' step.
 *
 * Assumes the given $step is a 'callback' step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   The step which the undo callback is being called on.
 */
function islandora_ingest_form_undo_callback_step(array $form, array &$form_state, array $step) {
  $args = array(&$form_state);
  $args = isset($step['undo_function']['args']) ? array_merge($args, $step['undo_function']['args']) : $args;
  call_user_func_array($step['undo_function']['function'], $args);
}

/**
 * Append Prev/Next buttons submit/validation handlers etc.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step
 *   An array defining a ingest step.
 *
 * @return array
 *   The stepified Drupal form definition for the given step.
 */
function islandora_ingest_form_stepify(array $form, array &$form_state, array $step) {
  $first_form_step = islandora_ingest_form_on_first_form_step($form_state);
  $last_form_step = islandora_ingest_form_on_last_form_step($form_state);
  $form['form_step_id'] = array(
    '#type' => 'hidden',
    '#value' => islandora_ingest_form_get_current_step_id($form_state),
  );
  $form['prev'] = $first_form_step ? NULL : islandora_ingest_form_previous_button($form_state);
  $form['next'] = $last_form_step ? islandora_ingest_form_ingest_button($form_state) : islandora_ingest_form_next_button($form_state);
  // Allow for a hook_form_FORM_ID_alter().
  drupal_alter(array('form_' . $step['form_id'], 'form'), $form, $form_state, $step['form_id']);
  return $form;
}

/**
 * Checks if we are on the first form step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return bool
 *   TRUE if we are currently on the first step, FALSE otherwise.
 */
function islandora_ingest_form_on_first_form_step(array &$form_state) {
  return !islandora_ingest_form_get_previous_form_step($form_state);
}

/**
 * Checks if we are on the last form step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return bool
 *   TRUE if we are currently on the last step, FALSE otherwise.
 */
function islandora_ingest_form_on_last_form_step(array &$form_state) {
  return !islandora_ingest_form_get_next_form_step($form_state);
}

/**
 * Get the previous form step relative to the current step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The previous form step if one exists, NULL otherwise.
 */
function islandora_ingest_form_get_previous_form_step(array &$form_state) {
  $step = NULL;
  do {
    $step = islandora_ingest_form_get_previous_step($form_state, $step);
  } while (isset($step) && $step['type'] != 'form');
  return $step;
}

/**
 * Get the next form step relative to the current step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The next form step if one exists, NULL otherwise.
 */
function islandora_ingest_form_get_next_form_step(array &$form_state) {
  $step = NULL;
  do {
    $step = islandora_ingest_form_get_next_step($form_state, $step);
  } while (isset($step) && $step['type'] != 'form');
  return $step;
}

/**
 * Defines the previous button for the ingest form.
 *
 * Adds submit handlers for the form step if they exist.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The previous button for the ingest form.
 */
function islandora_ingest_form_previous_button(array &$form_state) {
  // Before we move back to the previous step we should tell the previous steps
  // to undo whatever its submit handler did.
  $prev_form_step = islandora_ingest_form_get_previous_form_step($form_state);
  $form_id = $prev_form_step['form_id'];
  $submit_callback = $form_id . '_undo_submit';
  $submit = function_exists($submit_callback) ? array('islandora_ingest_form_previous_submit', $submit_callback) : array('islandora_ingest_form_previous_submit');
  return array(
    '#type' => 'submit',
    '#value' => t('Previous'),
    '#name' => 'prev',
    '#submit' => $submit,
    // #limit_validation_errors, is why when the previous button is pressed no
    // values persisted in the form_state, but its also what allows us to go
    // back when validation errors occur. To have a better solution going
    // forward we can either limit validation only on required fields, or we can
    // convert all required fields to use #element_validation functions, and
    // Remove the need for #limit_validation_errors. Or maybe there is some
    // other solution, regardless of what it is, it won't be standard.
    '#limit_validation_errors' => array(),
  );
}

/**
 * The submit handler for the ingest form previous button.
 *
 * Stores the current form steps values in the form storage.
 * Moves the focus of the multi-page ingest form back one step.
 * Restores the form values for the previous step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_previous_submit(array $form, array &$form_state) {
  islandora_ingest_form_decrement_step($form_state);
  $step = islandora_ingest_form_get_step($form_state);
  // Undo all callbacks that occured after the previous step.
  if ($step['type'] == 'callback') {
    islandora_ingest_form_undo_consecutive_callback_steps($form, $form_state, $step);
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Defines the next button for the ingest form.
 *
 * Adds submit/validate handlers for the form step if they exist.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The next button for the ingest form.
 */
function islandora_ingest_form_next_button(array &$form_state) {
  $step = islandora_ingest_form_get_step($form_state);
  $form_id = $step['form_id'];
  $validate_callback = $form_id . '_validate';
  $validate = function_exists($validate_callback) ? array($validate_callback) : NULL;
  $submit_callback = $form_id . '_submit';
  $submit = function_exists($submit_callback) ? array($submit_callback, 'islandora_ingest_form_next_submit') : array('islandora_ingest_form_next_submit');
  return array(
    '#type' => 'submit',
    '#value' => t('Next'),
    '#name' => 'next',
    '#validate' => $validate,
    '#submit' => $submit,
  );
}

/**
 * The submit handler for the ingest form next button.
 *
 * Stores the current form steps values in the form storage.
 * Moves the focus of the multi-page ingest form forward one step.
 * Restores the form values for the next step if present.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_next_submit(array $form, array &$form_state) {
  islandora_ingest_form_increment_step($form_state);
  $form_state['rebuild'] = TRUE;
}

/**
 * Push current info into the current step's storage.
 *
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_stash_info(array &$form_state) {
  $storage = &islandora_ingest_form_get_step_storage($form_state);
  if ($storage && isset($form_state['values'])) {
    $storage['values'] = $form_state['values'];
    unset($form_state['values']);
  }
}

/**
 * Pops the info for the given step from storage into the form_state.
 *
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_grab_info(array &$form_state) {
  $storage = islandora_ingest_form_get_step_storage($form_state);
  $form_state['values'] = isset($storage['values']) ? $storage['values'] : array();
}

/**
 * Defines the ingest button for the ingest form.
 *
 * This button is only shown on the last page of the multi-page ingest form.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The ingest button for the ingest form.
 */
function islandora_ingest_form_ingest_button(array &$form_state) {
  $step = islandora_ingest_form_get_step($form_state);
  $form_id = $step['form_id'];
  $validate_callback = $form_id . '_validate';
  $validate = function_exists($validate_callback) ? array($validate_callback) : NULL;
  $submit_callback = $form_id . '_submit';
  $submit = function_exists($submit_callback) ? array($submit_callback, 'islandora_ingest_form_submit') : array('islandora_ingest_form_submit');
  return array(
    '#type' => 'submit',
    '#name' => 'ingest',
    '#value' => t('Ingest'),
    '#process' => array('islandora_ingest_form_ingest_button_process'),
    '#validate' => $validate,
    '#submit' => $submit,
  );
}

/**
 * Process hook for the ingest button, adds the please wait spinning icon.
 */
function islandora_ingest_form_ingest_button_process(array $element) {
  $settings['spinner'][$element['#id']] = array(
    'message' => t('Please be patient while the the page loads.'),
    'options' => array(
      'lines' => 10,
      'length' => 20,
      'width' => 10,
      'radius' => 30,
      'corners' => 1,
      'rotate' => 0,
      'direction' => 1,
      'color' => '#000',
      'speed' => 1,
      'trail' => 60,
      'shadow' => FALSE,
      'hwaccel' => FALSE,
      'className' => 'spinner',
      'zIndex' => 2000000000,
      'top' => 'auto',
      'left' => 'auto',
    ),
  );
  drupal_add_js($settings, 'setting');
  $islandora_path = drupal_get_path('module', 'islandora');
  $element['#attached'] = array(
    'js' => array(
      "$islandora_path/js/spin/spin.min.js",
      "$islandora_path/js/spinner.js",
    ),
  );
  return $element;
}

/**
 * The submit handler for the ingest form.
 *
 * Attempts to ingest every object built by the previous steps.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_submit(array $form, array &$form_state) {
  // Execute any remaining callbacks.
  islandora_ingest_form_increment_step($form_state);
  $step = islandora_ingest_form_get_step($form_state);
  if (isset($step) && $step['type'] == 'callback') {
    islandora_ingest_form_execute_consecutive_callback_steps($form, $form_state, $step);
  }
  // Ingest the objects.
  foreach ($form_state['islandora']['objects'] as &$object) {
    try {
      islandora_add_object($object);
      $form_state['redirect'] = "islandora/object/{$object->id}";
    }
    catch (Exception $e) {
      // If post hooks throws it may already exist at this point but may be
      // invalid, so don't say failed.
      watchdog('islandora', 'Exception Message: @exception.', array('@exception' => $e->getMessage()), WATCHDOG_ERROR);
      drupal_set_message(t('A problem occured while ingesting "@label" (ID: @pid), please notifiy the administrator.', array('@label' => $object->label, '@pid' => $object->id)), 'error');
    }
  }
  // XXX: Foreaching with references can be weird... The reference exists in
  // the scope outside.
  unset($object);
}

/**
 * Gets a reference to the stored NewFedoraObject's.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   A reference to the stored NewFedoraObjects to be ingested when the final
 *   step submits.
 */
function &islandora_ingest_form_get_objects(array &$form_state) {
  return $form_state['islandora']['objects'];
}

/**
 * Gets a single object from the stored NewFedoraObject's.
 *
 * @note In our current use case we are only dealing with a single object
 *   ingest, this makes it convenient to access it. Ideally the steps
 *   implementations will be abstracted to be indifferent to what object it's
 *   currently working on. This will act as a placeholder for such
 *   functionality.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return NewFedoraObject
 *   Returns the 'current' object in the array of NewFedoraObjects, generally
 *   this is only used when there is one object in the list of objects.
 */
function islandora_ingest_form_get_object(array &$form_state) {
  $objects = &islandora_ingest_form_get_objects($form_state);
  return reset($objects);
}

/**
 * Get general storage for the given/current step.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param array $step_id
 *   The ID of the step.
 *
 * @return array
 *   The given/current step storage if found, NULL otherwise.
 */
function &islandora_ingest_form_get_step_storage(array &$form_state, $step_id = NULL) {
  $step_id = isset($step_id) ? $step_id : islandora_ingest_form_get_current_step_id($form_state);
  if (isset($step_id)) {
    if (!isset($form_state['islandora']['step_storage'][$step_id])) {
      $form_state['islandora']['step_storage'][$step_id] = array();
    }
    return $form_state['islandora']['step_storage'][$step_id];
  }
  $undefined_step_storage = array();
  return $undefined_step_storage;
}

/**
 * Gets the configuration used to create the multi-page ingest form.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The configuration used to generate the multi-page ingest forms.
 */
function &islandora_ingest_form_get_shared_storage(array &$form_state) {
  return $form_state['islandora']['shared_storage'];
}

/**
 * Call form_load_include, for the current step if it defines a required file.
 *
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_load_include(array &$form_state) {
  form_load_include($form_state, 'inc', 'islandora', 'includes/ingest.form');
  $step = islandora_ingest_form_get_step($form_state);
  // Load include files.
  if (isset($step['file']) && isset($step['module'])) {
    $matches = array();
    preg_match('/^(.*)\.(.*)$/', $step['file'], $matches);
    list($file, $name, $type) = $matches;
    form_load_include($form_state, $type, $step['module'], $name);
  }
}

/**
 * Buildes the initial list of ingest steps.
 *
 * Sorted by weight expected range between -50 to 50.
 *
 * The sort order is undefined for steps which have the same weight.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The list of sorted ingest steps as defined by all implementers
 *   of ISLANDORA_INGEST_STEP_HOOK.
 */
function islandora_ingest_form_get_steps(array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $steps = &drupal_static(__FUNCTION__);
  if (isset($steps)) {
    return $steps;
  }
  $steps = array();
  $shared_storage = &islandora_ingest_form_get_shared_storage($form_state);
  foreach (islandora_build_hook_list(ISLANDORA_INGEST_STEP_HOOK, $shared_storage['models']) as $hook) {
    // Required for pass by reference.
    // @todo Change this around so that it isn't passed by reference, there
    // Is an alter below that can handle that requirement.
    foreach (module_implements($hook) as $module) {
      $function = $module . '_' . $hook;
      $module_steps = (array) $function($form_state);
      $steps = array_merge($steps, $module_steps);
    }
  }
  $steps = array_filter($steps);
  foreach (islandora_build_hook_list(ISLANDORA_INGEST_STEP_HOOK, $shared_storage['models']) as $hook) {
    drupal_alter($hook, $steps, $form_state);
  }
  // Add any defaults.
  foreach ($steps as $key => &$step) {
    $step['id'] = $key;
  }
  uasort($steps, 'drupal_sort_weight');
  return $steps;
}

/**
 * Filter the ingest steps to only steps of type 'form'.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The list of sorted ingest form steps as defined by all implementers
 *   of ISLANDORA_INGEST_STEP_HOOK.
 */
function islandora_ingest_form_get_form_steps(array &$form_state) {
  $steps = islandora_ingest_form_get_steps($form_state);
  $form_step_filter = function($o) {
    return $o['type'] == 'form';
  };
  return array_filter($steps, $form_step_filter);
}

/**
 * Filter the ingest steps to only steps of type 'form'.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The list of sorted ingest callback steps as defined by all implementers
 *   of ISLANDORA_INGEST_STEP_HOOK.
 */
function islandora_ingest_form_get_callback_steps(array &$form_state) {
  $steps = islandora_ingest_form_get_steps($form_state);
  $callback_step_filter = function($o) {
    return $o['type'] == 'callback';
  };
  return array_filter($steps, $callback_step_filter);
}
