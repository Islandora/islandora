<?php

/**
 * @file
 *
 * Defines the multi-page ingest form and any relevant hooks and functions for
 * defining the multi-page ingest forms.
 */

/**
 * Ingest form build function. Initializes the form state, and builds the
 * initial list of steps, excutes the current step.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
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
  islandora_ingest_form_init_form_state($form_state, $configuration);

  $form = islandora_ingest_form_execute_step($form, $form_state);

  return $form;
}

/**
 * Prepares/Initializes the form state for use.
 *
 * Also cleans up or loads any data required.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param array $configuration
 *   A list of key value pairs that are used to build the list of steps to be
 *   executed.
 */
function islandora_ingest_form_init_form_state(array &$form_state, array $configuration) {
  // First time initialization of storage.
  islandora_ingest_form_init_form_state_storage($form_state, $configuration);
}

/**
 * Initializes the form_state storage for use in the ingest multi-page forms.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param array $configuration
 *   A list of key value pairs that are used to build the list of steps to be
 *   executed.
 */
function islandora_ingest_form_init_form_state_storage(array &$form_state, array $configuration) {
  if (empty($form_state['islandora'])) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    // Use ID if given.
    $id = isset($configuration['id']) ? $configuration['id'] : NULL;
    // Use namespace if ID not given.
    $namespace = isset($configuration['namespace']) && !isset($id) ? $configuration['namespace'] : $id;
    $label = isset($configuration['label']) ? $configuration['label'] : 'New Object';
    $relationship_map = function($o) { return array('relationship' => 'isMemberOfCollection', 'pid' => $o); };
    $relationships = empty($configuration['collections']) ? array() : array_map($relationship_map, $configuration['collections']);
    $object = islandora_prepare_new_object($namespace, $label, array(), array(), $relationships);
    $form_state['islandora'] = array(
      'step_id' => NULL,
      'objects' => array($object),
      'configuration' => $configuration,
      'step_storage' => array(),
    );
  }

  $steps = islandora_ingest_get_steps($form_state);
  if ($form_state['islandora']['step_id'] === NULL || !array_key_exists(
    $form_state['islandora']['step_id'], $steps)) {
    reset($steps);
    $form_state['islandora']['step_id'] = key($steps);
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
  $fake_form_state = array(
    'islandora' => array(
      'configuration' => $configuration,
    ),
  );
  $steps = islandora_ingest_get_steps($fake_form_state);
  drupal_static_reset('islandora_ingest_get_steps');
  return $steps;
}

/**
 * Executes the current step, building the form definition and appending on any
 * additonal elements required for the step to function.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The form definition of the current step.
 */
function islandora_ingest_form_execute_step(array $form, array &$form_state) {
  // Load any required files for the current step.
  islandora_ingest_form_step_form_load_include($form_state);

  $step_info = islandora_ingest_form_get_step_info($form_state);

  switch ($step_info['type']) {
    case 'form':
      $args = array($form, &$form_state);
      $args = isset($step_info['args']) ? array_merge($args, $step_info['args']) : $args;
      $form = call_user_func_array($step_info['form_id'], $args);
      return islandora_ingest_form_stepify($form, $form_state, $step_info);
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
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 * @param array $step_info
 *   The info for building the current form step.
 *
 * @return array
 *   The stepified drupal form definition for the given step.
 */
function islandora_ingest_form_stepify(array $form, array &$form_state, array $step_info) {
  $step = islandora_ingest_form_get_step_number($form_state);
  
  $first_step = $step > 0;
  $last_step = $step == (islandora_ingest_form_get_step_count($form_state) - 1);
  $form['prev'] = $first_step ? islandora_ingest_form_previous_button() : NULL;
  $form['next'] = $last_step ? islandora_ingest_form_ingest_button($step_info) : islandora_ingest_form_next_button($step_info);
  return $form;
}

/**
 * Defines the previous button for the ingest form.
 *
 * @return array
 *   The previous button for the ingest form.
 */
function islandora_ingest_form_previous_button() {
  return array(
    '#type' => 'submit',
    '#value' => t('Previous'),
    '#name' => 'prev',
    '#submit' => array('islandora_ingest_form_previous_submit'),
    // #limit_validation_errors, is why when the previous button is pressed no
    // values persisted in the form_state, but its also what allows us to go
    // back when validation errors occur. To have a better solution going
    // forward we can either limit validation only on required fields, or we can
    // convert all required fields to use #element_validation functions, and
    // Remove the need for #limit_validation_errors. Or maybe there is some
    // other solution, regardless of what it is, it won't be standard.
    '#limit_validation_errors' => array()
  );
}

function islandora_ingest_form_get_step_number($form_state, $step_id = NULL) {
  if ($step_id === NULL) {
    $step_id = islandora_ingest_form_get_step_id($form_state);
  }

  $steps = array_flip(array_keys(islandora_ingest_form_get_steps($form_state)));
  return $steps[$step_id];
}

/**
 * The submit handler for the ingest form previous button.
 *
 * Stores the current form steps values in the form storage.
 * Moves the focus of the multi-page ingest form back one step.
 * Restores the form values for the previous step.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_ingest_form_previous_submit(array $form, array &$form_state) {
  $step = islandora_ingest_form_get_step_number($form_state);
  islandora_ingest_form_stash_info($form_state);
  $step = max(array($step - 1, 0)) ;
  $form_state['islandora']['step_id'] = islandora_ingest_form_get_step_id($form_state, $step);
  islandora_ingest_form_grab_info($form_state);
  $form_state['rebuild'] = TRUE;
}

/**
 * Defines the next button for the ingest form.
 *
 * Adds submit/validate handlers for the form step if they exist.
 *
 * @return array
 *   The next button for the ingest form.
 */
function islandora_ingest_form_next_button(array $step_info) {
  $form_id = $step_info['form_id'];
  $validate_callback = $form_id . '_validate';
  $validate = function_exists($validate_callback) ? array($validate_callback) : NULL;
  $submit_callback = $form_id . '_submit';
  $submit = function_exists($submit_callback) ? array($submit_callback, 'islandora_ingest_form_next_submit') : array('islandora_ingest_form_next_submit');
  return array(
    '#type' => 'submit',
    '#value' => t('Next'),
    '#name' => 'next',
    '#validate' => $validate,
    '#submit' => $submit
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
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_ingest_form_next_submit(array $form, array &$form_state) {
  $step = islandora_ingest_form_get_step_number($form_state);
  drupal_static_reset('islandora_ingest_form_get_steps');
  islandora_ingest_form_stash_info($form_state);
  $step = min(array($step + 1, islandora_ingest_form_get_step_count($form_state) - 1)) ;
  $form_state['islandora']['step_id'] = islandora_ingest_form_get_step_id($form_state, $step);
  islandora_ingest_form_grab_info($form_state);
  $form_state['rebuild'] = TRUE;
}

/**
 * Push current info into the current step's storage.
 */
function islandora_ingest_form_stash_info(array &$form_state) {
  $storage = &islandora_ingest_form_get_step_storage($form_state);
  $storage['values'] = $form_state['values'];
  $storage['configuration'] = $form_state['islandora']['configuration'];
  unset($form_state['values']); 
}

/**
 * Pops the info for the given step from storage into the form_state.
 */
function islandora_ingest_form_grab_info(array &$form_state) {
  $storage = islandora_ingest_form_get_step_storage($form_state);
  $form_state['values'] = isset($storage['values'])?$storage['values']:array();
  $form_state['islandora']['configuration'] = array_merge(
    $form_state['islandora']['configuration'],
    isset($storage['configuration'])?
      $storage['configuration']:
      array()
  );
}

/**
 * Defines the ingest button for the ingest form.
 *
 * This button is only shown on the last page of the multi-page ingest form.
 *
 * @return array
 *   The ingest button for the ingest form.
 */
function islandora_ingest_form_ingest_button(array $step_info) {
  $form_id = $step_info['form_id'];
  $validate_callback = $form_id . '_validate';
  $validate = function_exists($validate_callback) ? array($validate_callback) : NULL;
  $submit_callback = $form_id . '_submit';
  $submit = function_exists($submit_callback) ? array($submit_callback, 'islandora_ingest_form_submit') : array('islandora_ingest_form_submit');
  return array(
    '#type' => 'submit',
    '#name' => 'ingest',
    '#value' => t('Ingest'),
    '#validate' => $validate,
    '#submit' => $submit
  );
}

/**
 * The submit handler for the ingest form.
 *
 * Attempts to ingest every object built by the previous steps.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_ingest_form_submit(array $form, array &$form_state) {
  foreach ($form_state['islandora']['objects'] as $object) {
    try {
      islandora_add_object($object);
      $form_state['redirect'] = "islandora/object/{$object->id}";
    } catch (Exception $e) {
      // If post hooks throws it may already exist at this point but may be invalid, so don't say failed :P
      watchdog('islandora', $e->getMessage(), NULL, WATCHDOG_ERROR);
      drupal_set_message(t('A problem occured while ingesting "@label" (ID: @pid), please notifiy the administrator.', array('@label' => $object->label, '@pid' => $object->id)), 'error');
    }
  }
}

/**
 * Gets the configuration used to create the multi-page ingest form.
 *
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The configuration used to generate the multi-page ingest forms.
 */
function &islandora_ingest_form_get_configuration(array $form_state) {
  return $form_state['islandora']['configuration'];
}

/**
 * Gets a reference to the stored NewFedoraObject's which are to be ingested
 * when the final step submits.
 *
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   A reference to the stored NewFedoraObjects to be ingested when the final
 *   step submits.
 */
function &islandora_ingest_form_get_objects(array $form_state) {
  return $form_state['islandora']['objects'];
}

/**
 * Gets a single object from the stored NewFedoraObject's.
 *
 * @note - In our current use case we are only dealing with a single object
 *   ingest, this makes it convenient to access it. Ideally the steps
 *   implementations will be abstracted to be indifferent to what object it's
 *   currently working on. This will act as a placeholder for such
 *   functionality.
 *
 * @param array $form_state
 *   The drupal form state.
 *
 * @return NewFedoraObject
 *   Returns the 'current' object in the array of NewFedoraObjects, generally
 *   this is only used when there is one object in the list of objects.
 */
function islandora_ingest_form_get_object(array $form_state) {
  $objects = &islandora_ingest_form_get_objects($form_state);
  return current($objects);
}

/**
 * Get a reference to the current step index.
 *
 * @param array $form_state
 *   The drupal form state.
 *
 * @return string
 *   The current step index.
 */
function islandora_ingest_form_get_step_id(array &$form_state, $step_number = NULL) {
  if ($step_number === NULL) {
    return $form_state['islandora']['step_id'];
  }
  else {
    $steps = array_keys(islandora_ingest_form_get_steps($form_state));
    return $steps[$step_number];
  }
}

/**
 * Get a reference to the step info of the given step or the current step if
 * none is given.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param int $step
 *   The index of the step to get.
 *
 * @return integer
 *   The step info of the requested step if found, NULL otherwise.
 */
function islandora_ingest_form_get_step_info(array &$form_state, $step = NULL) {
  $step = isset($step) ? $step : islandora_ingest_form_get_step_id($form_state);
  $steps = islandora_ingest_form_get_steps($form_state);
  if (!empty($steps[$step])) {
    return $steps[$step];
  }
  // @todo deal with unknown case.
  return NULL;
}

/**
 * Get general storage for the given step. (or the current step)
 */
function &islandora_ingest_form_get_step_storage(array &$form_state, $step_id = NULL) {
  if ($step_id === NULL) {
    $step_id = islandora_ingest_form_get_step_id($form_state);
  }

  if (!isset($form_state['islandora']['step_storage'][$step_id])) {
    $form_state['islandora']['step_storage'][$step_id] = array();
  }

  return $form_state['islandora']['step_storage'][$step_id];
}

/**
 * Get a reference to the steps stored in the form state.
 *
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   All the steps to be used in the ingest process.
 */
function islandora_ingest_form_get_steps(array &$form_state) {
  return islandora_ingest_get_steps($form_state);
}

/**
 * Call form_load_include, for the current step if it defines a required file.
 *
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_ingest_form_step_form_load_include(array &$form_state) {
  form_load_include($form_state, 'inc', 'islandora', 'includes/ingest.form');
  $step_info = islandora_ingest_form_get_step_info($form_state);
  // Load include files
  if (isset($step_info['file']) && isset($step_info['module'])) {
    $matches = array();
    preg_match('/^(.*)\.(.*)$/', $step_info['file'], $matches);
    list($file, $name, $type) = $matches;
    form_load_include($form_state, $type, $step_info['module'], $name);
  }
}

/**
 * Get the number of steps.
 *
 * @param array $form_state
 *   The drupal form state.
 *
 * @return int
 *   The number of steps.
 */
function islandora_ingest_form_get_step_count(array $form_state) {
  $steps = islandora_ingest_form_get_steps($form_state);
  return count($steps);
}

/**
 * Buildes the initial list of ingest steps.
 *
 * Sorted by weight expected range between -50 to 50.
 *
 * The sort order is undefined for steps which have the same weight.
 *
 * @param array $form_state
 *   All available form_state options used to build the multi-paged ingest process.
 *
 * @return array
 *   The list of sorted ingest steps as defined by all implementers
 *   of ISLANDORA_INGEST_STEP_HOOK.
 */
function &islandora_ingest_get_steps(array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $steps = &drupal_static(__FUNCTION__);

  if (!isset($steps)) {
    $steps = array();
    $configuration = islandora_ingest_form_get_configuration($form_state);
    $models = array();
    if (isset($configuration['selected_models'])) {
      $models = $configuration['selected_models'];
    }
    elseif (isset($configuration['models'])) {
      $models = $configuration['models'];
    }

    foreach (islandora_build_hook_list(ISLANDORA_INGEST_STEP_HOOK, $models) as $hook) {
      // Need to pass by ref...  But don't really want an alter.
      foreach (module_implements($hook) as $module) {
        $function = $module . '_' . $hook;
        $steps = array_merge($steps, (array)$function($form_state));
      }
    }

    // Remove NULL values.
    $steps = array_filter($steps);
    uasort($steps, 'drupal_sort_weight');
    foreach (islandora_build_hook_list(ISLANDORA_INGEST_STEP_HOOK, $models) as $hook) {
      $form_state_copy = $form_state;
      drupal_alter($hook, $steps, $form_state_copy);
    }

    uasort($steps, 'drupal_sort_weight');
  }

  return $steps;
}
