<?php
/**
 * @file
 * Defines functions used when constructing derivatives.
 */

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
  $options += array(
    'force' => FALSE,
  );
  $hooks = islandora_invoke_hook_list(ISLANDORA_DERVIATIVE_CREATION_HOOK, $object->models, array($object));
  uasort($hooks, 'drupal_sort_weight');
  $results = array();

  if (array_key_exists('source_dsid', $options)) {
    $hooks = array_filter($hooks, function($filter_hook) use($options) {
      return array_key_exists('source_dsid', $filter_hook) &&
      $filter_hook['source_dsid'] == $options['source_dsid'];
    });
  }

  if (array_key_exists('destination_dsid', $options)) {
    $hooks = array_filter($hooks, function($filter_hook) use($options) {
      return array_key_exists('destination_dsid', $filter_hook) &&
      $filter_hook['destination_dsid'] == $options['destination_dsid'];
    });
  }

  foreach ($hooks as $hook) {
    if (isset($hook['file'])) {
      require_once $hook['file'];
    }
    foreach ($hook['function'] as $function) {
      if (function_exists($function)) {
        $logging = call_user_func($function, $object, $options['force']);
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
        drupal_set_message(filter_xss(format_string($message['message'], isset($message['message_sub']) ? $message['message_sub'] : array())), isset($message['severity']) ? $message['severity'] : 'status');
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
