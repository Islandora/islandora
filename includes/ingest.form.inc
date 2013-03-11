<?php

/**
 * @file
 * Defines the multi-page ingest form and any relevant hooks and functions.
 */

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
 *
 * @return array
 *   The form definition of the current step.
 */
function islandora_ingest_form(array $form, array &$form_state, array $configuration) {
  try {
    islandora_ingest_form_init_form_state_storage($form_state, $configuration);
    return islandora_ingest_form_execute_step($form, $form_state);
  }
  catch(Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    return array(array(
        '#markup' => l(t('Back'), 'javascript:window.history.back();', array('external' => TRUE))));
  }
}

/**
 * Validates the given ingest configuration.
 *
 * At the moment it only requires that models are present.
 *
 * @todo Add hook for manipulating/validating the configuration.
 *
 * @see islandora_ingest_form()
 *
 * @throws InvalidArgumentException
 *
 * @param array $configuration
 *   The key value pairs that are used to build the multi-paged ingest process.
 */
function islandora_ingest_form_validate_configuration(array $configuration) {
  if (empty($configuration['models'])) {
    throw new InvalidArgumentException('Ingest configuration not vaild, no models were given');
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
    // Validate the configuration before we use it.
    islandora_ingest_form_validate_configuration($configuration);
    $object = islandora_ingest_form_prepare_new_object($configuration);
    $form_state['islandora'] = array(
      'step_id' => NULL,
      'objects' => array($object),
      'shared_storage' => $configuration,
      'step_storage' => array(),
    );
  }
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

/**
 * Gets the given/current step.
 *
 * The current step is returned if no step ID is given. If the current step is
 * not defined it's assume to be the first step.
 *
 * @param array $form_state
 *   The Drupal form state.
 * @param string $step_id
 *   The ID of the step.
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
 * Gets the ID of the current step.
 *
 * If a current step is not defined, its assumed to be the first step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return string
 *   The step ID.
 */
function islandora_ingest_form_get_current_step_id(array &$form_state) {
  if (empty($form_state['islandora']['step_id'])) {
    $steps = islandora_ingest_form_get_steps($form_state);
    return array_shift(array_keys($steps));
  }
  return $form_state['islandora']['step_id'];
}

/**
 * Gets the ID of the next step.
 *
 * If a current step is not defined, its assumed to be the first step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return string
 *   The next step ID if found, NULL otherwise.
 */
function islandora_ingest_form_get_next_step_id(array &$form_state) {
  $step_id = islandora_ingest_form_get_current_step_id($form_state);
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
 * If a current step is not defined, its assumed to be the first step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return string
 *   The previous step ID if found, NULL otherwise.
 */
function islandora_ingest_form_get_previous_step_id(array &$form_state) {
  $step_id = islandora_ingest_form_get_current_step_id($form_state);
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
  if (isset($next_step_id)) {
    islandora_ingest_form_stash_info($form_state);
    $form_state['islandora']['step_id'] = $next_step_id;
    islandora_ingest_form_grab_info($form_state);
  }
}

/**
 * Decrement the current step if possible.
 *
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ingest_form_decrement_step(array &$form_state) {
  $previous_step_id  = islandora_ingest_form_get_previous_step_id($form_state);
  if (isset($previous_step_id)) {
    islandora_ingest_form_stash_info($form_state);
    $form_state['islandora']['step_id'] = $previous_step_id;
    islandora_ingest_form_grab_info($form_state);
  }
}

/**
 * Build a list of steps given only configuration.
 *
 * XXX: This is used to give an indication of whether there are any steps for a
 *   given list of content models.
 *
 * @param array $configuration
 *   The list of key/value pairs of configuration.
 */
function islandora_ingest_get_approximate_steps(array $configuration) {
  try {
    islandora_ingest_form_validate_configuration($configuration);
  }
  catch(InvalidArgumentException $e) {
    // Don't log or display exception.
    return array();
  }
  $stubbed_form_state = array(
    'islandora' => array(
      'shared_storage' => $configuration,
    ),
  );
  $steps = islandora_ingest_form_get_steps($stubbed_form_state);
  drupal_static_reset('islandora_ingest_form_get_steps');
  return $steps;
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
 *
 * @return array
 *   The form definition of the current step.
 */
function islandora_ingest_form_execute_step(array $form, array &$form_state) {
  // Load any required files for the current step.
  islandora_ingest_form_load_include($form_state);
  $step = islandora_ingest_form_get_step($form_state);
  switch ($step['type']) {
    case 'form':
      $args = array($form, &$form_state);
      $args = isset($step['args']) ? array_merge($args, $step['args']) : $args;
      $form = call_user_func_array($step['form_id'], $args);
      return islandora_ingest_form_stepify($form, $form_state, $step);

    case 'batch':
      // @todo Implement if possible.
      break;
  }
  return array();
}

/**
 * Append Prev/Next buttons submit/validation handlers etc.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The stepified drupal form definition for the given step.
 */
function islandora_ingest_form_stepify(array $form, array &$form_state) {
  $first_step = islandora_ingest_form_on_first_step($form_state);
  $last_step = islandora_ingest_form_on_last_step($form_state);
  $form['prev'] = $first_step ? NULL : islandora_ingest_form_previous_button($form_state);
  $form['next'] = $last_step ? islandora_ingest_form_ingest_button($form_state) : islandora_ingest_form_next_button($form_state);
  return $form;
}

/**
 * Checks if we are on the first step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return bool
 *   TRUE if we are currently on the first step, FALSE otherwise.
 */
function islandora_ingest_form_on_first_step(array &$form_state) {
  $step_id = islandora_ingest_form_get_current_step_id($form_state);
  $step_ids = array_keys(islandora_ingest_form_get_steps($form_state));
  return array_search($step_id, $step_ids) == 0;
}

/**
 * Checks if we are on the last step.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return bool
 *   TRUE if we are currently on the last step, FALSE otherwise.
 */
function islandora_ingest_form_on_last_step(array &$form_state) {
  $step_id = islandora_ingest_form_get_current_step_id($form_state);
  $step_ids = array_keys(islandora_ingest_form_get_steps($form_state));
  $count = count($step_ids);
  return array_search($step_id, $step_ids) == --$count;
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
  // Before we move back to the previous step we should tell the previous step
  // to undo whatever its submit handler did.
  $prev_step_id = islandora_ingest_form_get_previous_step_id($form_state);
  $prev_step = islandora_ingest_form_get_step($form_state, $prev_step_id);
  $form_id = $prev_step['form_id'];
  $submit_callback = $form_id . '_undo_submit';
  $submit = function_exists($submit_callback) ? array($submit_callback, 'islandora_ingest_form_previous_submit') : array('islandora_ingest_form_undo_submit');
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
  $storage['values'] = $form_state['values'];
  unset($form_state['values']);
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
    '#validate' => $validate,
    '#submit' => $submit,
  );
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
  foreach ($form_state['islandora']['objects'] as $object) {
    try {
      islandora_add_object($object);
      $form_state['redirect'] = "islandora/object/{$object->id}";
    }
    catch (Exception $e) {
      // If post hooks throws it may already exist at this point but may be
      // invalid, so don't say failed.
      watchdog('islandora', $e->getMessage(), NULL, WATCHDOG_ERROR);
      drupal_set_message(t('A problem occured while ingesting "@label" (ID: @pid), please notifiy the administrator.', array('@label' => $object->label, '@pid' => $object->id)), 'error');
    }
  }
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
  return current($objects);
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
  return NULL;
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
  uasort($steps, 'drupal_sort_weight');
  return $steps;
}
