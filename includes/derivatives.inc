<?php
/**
 * @file
 * Defines functions used when constructing derivatives.
 */

// Relations.
define('ISLANDORA_DEFER_DERIVATIVES_FLAG', 'deferDerivatives');

/**
 * Decides which derivative function to call and runs it.
 *
 * @param AbstractObject $object
 *   The object to run the derivative function for.
 * @param string $dsid
 *   The DSID to run the derivative function for.
 */
function islandora_run_derivatives(AbstractObject $object, $dsid) {
  $batch_array = batch_get();
  if (empty($batch_array)) {
    $logging_results = islandora_do_derivatives($object, array(
      'source_dsid' => $dsid,
    ));
    islandora_derivative_logging($logging_results);
  }
  else {
    $operations = islandora_do_batch_derivatives(
      $object,
      array(
        'source_dsid' => $dsid,
      )
    );
    if ($operations) {
      batch_set(
        // Title won't show for batch in a batch.
        array(
          'init_message' => t('Preparing derivatives for @label', array('@label' => $object->label)),
          'error_message' => t('An error occured creating derivatives.'),
          'progress_message' => t(
            'Creating derivatives for @label <br/>Time elapsed: @elapsed <br/>
              Estimated time remaining @estimate.',
            array('@label' => $object->label)
          ),
          'file' => drupal_get_path('module', 'islandora') . '/includes/regenerate_derivatives.form.inc',
          'operations' => $operations,
        )
      );
    }
  }
}

/**
 * Kicks off derivative functions based upon hooks and conditions.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a FedoraObject.
 * @param array $options
 *   An array of parameters containing:
 *   - force: Bool denoting whether we are forcing the generation of
 *     derivatives.
 *   - source_dsid: (Optional) String of the datastream id we are generating
 *     from or NULL if it's the object itself.
 *   - destination_dsid: (Optional) String of the datastream id that is being
 *     created. To be used in the UI.
 *
 * @return array
 *   An array of messages describing the outcome of the derivative events.
 *   Each individual message array has the following structure:
 *   - success: Bool denoting whether the operation was successful.
 *   - messages: An array structure containing:
 *     - message: A string passed through t() describing the
 *       outcome of the operation.
 *     - message_sub: (Optional) Substitutions to be passed along to t() or
 *       watchdog.
 *     - type: A string denoting whether the output is to be
 *       drupal_set_messaged (dsm) or watchdogged (watchdog).
 *     - severity: (Optional) A severity level / status to be used when
 *       logging messages. Uses the defaults of drupal_set_message and
 *       watchdog if not defined.
 */
function islandora_do_derivatives(AbstractObject $object, array $options) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $options += array(
    'force' => FALSE,
  );
  $hooks = islandora_invoke_hook_list(ISLANDORA_DERIVATIVE_CREATION_HOOK, $object->models, array($object));
  uasort($hooks, 'drupal_sort_weight');
  $results = array();
  $hooks = islandora_filter_derivatives($hooks, $options, $object);

  foreach ($hooks as $hook) {
    if (isset($hook['file'])) {
      require_once $hook['file'];
    }
    foreach ($hook['function'] as $function) {
      if (function_exists($function)) {
        $logging = call_user_func($function, $object, $options['force'], $hook);
        if (!empty($logging)) {
          $results[] = $logging;
        }
      }
      else {
        watchdog('islandora', 'Unable to call derivative function @function as it was not found!', array('@function' => $function), WATCHDOG_ERROR);
      }
    }
  }
  return $results;
}

/**
 * Handles the logging of derivative messages.
 *
 * @param array $logging_results
 *   An array of messages describing the outcome of the derivative events.
 *   Each individual message array has the following structure:
 *   - success: Bool denoting whether the operation was successful.
 *   - messages: An array structure containing:
 *     - message: A string passed through t() describing the
 *       outcome of the operation.
 *     - message_sub: (Optional) Substitutions to be passed along to t() or
 *       watchdog.
 *     - type: A string denoting whether the output is to be
 *       drupal_set_messaged (dsm) or watchdogged (watchdog).
 *     - severity: (Optional) A severity level / status to be used when
 *       logging messages. Uses the defaults of drupal_set_message and
 *       watchdog if not defined.
 */
function islandora_derivative_logging(array $logging_results) {
  foreach ($logging_results as $result) {
    foreach ($result['messages'] as $message) {
      if ($message['type'] === 'dsm') {
        if (isset($message['severity']) && $message['severity'] != 'status') {
          drupal_set_message(filter_xss(format_string($message['message'], isset($message['message_sub']) ? $message['message_sub'] : array())), $message['severity']);
        }
        else {
          if (!isset($_SESSION['islandora_event_messages'])) {
            $_SESSION['islandora_event_messages'] = array();
          }
          $_SESSION['islandora_event_messages'][] = array(
            'message' => filter_xss(format_string($message['message'], isset($message['message_sub']) ? $message['message_sub'] : array())),
            'severity' => 'status',
          );
          drupal_set_message(l(t('Derivatives successfully created.'), 'islandora/event-status'), 'status', FALSE);
        }
      }
      else {
        // We know what we are doing here. Passing through the translated
        // message and the substitutions needed. We are using
        // call_user_func until such time as the @ignore changes
        // are merged into the standard release for Coder.
        call_user_func('watchdog', 'islandora_derivatives', $message['message'], isset($message['message_sub']) ? $message['message_sub'] : array(), isset($message['severity']) ? $message['severity'] : WATCHDOG_NOTICE);
      }
    }
  }
}

/**
 * Kicks off derivative functions based upon hooks and conditions.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a FedoraObject.
 * @param array $options
 *   An array of parameters containing:
 *   - force: Bool denoting whether we are forcing the generation of
 *     derivatives.
 *   - source_dsid: (Optional) String of the datastream id we are generating
 *     from or NULL if it's the object itself.
 *   - destination_dsid: (Optional) String of the datastream id that is being
 *     created. To be used in the UI.
 *
 * @return array
 *   An array of operations to be called from within a batch.
 */
function islandora_do_batch_derivatives(AbstractObject $object, array $options) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $options += array(
    'force' => FALSE,
  );
  $hooks = islandora_invoke_hook_list(ISLANDORA_DERIVATIVE_CREATION_HOOK, $object->models, array($object));
  uasort($hooks, 'drupal_sort_weight');
  $operations = array();

  $hooks = islandora_filter_derivatives($hooks, $options, $object);
  foreach ($hooks as $hook) {
    $file = FALSE;
    if (isset($hook['file'])) {
      $file = $hook['file'];
    }
    foreach ($hook['function'] as $function) {
      $operations[] = array(
        'islandora_derivative_perform_batch_operation',
        array(
          $function,
          $file,
          $object->id,
          $options['force'],
          $hook,
        ),
      );
    }
  }
  return $operations;
}

/**
 * Filter the derivative functions to only call those which are valid.
 *
 * @param array $hooks
 *   An array of hooks to be filtered depending on options.
 * @param array $options
 *   An array of options for the derivative generation.
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 *
 * @return array
 *   Returns the filtered array of hooks to be ran.
 */
function islandora_filter_derivatives($hooks, $options, AbstractObject $object) {
  if (array_key_exists('source_dsid', $options)) {
    $hooks = array_filter($hooks, function ($filter_hook) use ($options) {
      return array_key_exists('source_dsid', $filter_hook) &&
      $filter_hook['source_dsid'] == $options['source_dsid'];
    });
  }
  if (array_key_exists('destination_dsid', $options)) {
    $hooks = array_filter($hooks, function ($filter_hook) use ($options) {
      return array_key_exists('destination_dsid', $filter_hook) &&
      $filter_hook['destination_dsid'] == $options['destination_dsid'];
    });
  }
  // Do a final filtering to make sure that the source DSID exists on the object
  // where needed. Using a defined function as opposed to the way above as
  // it seems to break PHPCS as of 1.4.8.
  $filter_function = function ($filter_hook) use ($object) {
    $to_return = FALSE;
    if (array_key_exists('source_dsid', $filter_hook)) {
      if ($filter_hook['source_dsid'] != NULL) {
        if (isset($object[$filter_hook['source_dsid']])) {
          $to_return = TRUE;
        }
      }
      else {
        $to_return = TRUE;
      }
    }
    return $to_return;
  };
  $hooks = array_filter($hooks, $filter_function);
  return $hooks;
}

/**
 * Set the defer derivatives flag on an object.
 */
function islandora_set_defer_derivatives_flag(AbstractObject $object) {
  $object->relationships->add(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_DEFER_DERIVATIVES_FLAG,
    'true',
    RELS_TYPE_PLAIN_LITERAL
  );
}

/**
 * Get the defer derivatives flag on an object.
 */
function islandora_get_defer_derivatives_flag(AbstractObject $object) {
  return $object->relationships->get(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_DEFER_DERIVATIVES_FLAG,
    'true',
    RELS_TYPE_PLAIN_LITERAL
  );
}

/**
 * Remove the defer derivatives flag on an object.
 */
function islandora_remove_defer_derivatives_flag(AbstractObject $object) {
  $object->relationships->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_DEFER_DERIVATIVES_FLAG,
    'true',
    RELS_TYPE_PLAIN_LITERAL
  );
}
