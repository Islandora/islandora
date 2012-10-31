<?php

/**
 * @file
 * Contains islandora utility functions
 */

/**
 * Convert bytes to human readable format
 *
 * XXX: Shouldn't Drupal's format_size() be preferred?
 *
 * @param integer bytes Size in bytes to convert
 * @return string
 */
function islandora_convert_bytes_to_human_readable($bytes, $precision = 2) {
  $kilobyte = 1024;
  $megabyte = $kilobyte * 1024;
  $gigabyte = $megabyte * 1024;
  $terabyte = $gigabyte * 1024;

  if (($bytes >= 0) && ($bytes < $kilobyte)) {
    return $bytes . ' B';
  }
  elseif (($bytes >= $kilobyte) && ($bytes < $megabyte)) {
    return round($bytes / $kilobyte, $precision) . ' KB';
  }
  elseif (($bytes >= $megabyte) && ($bytes < $gigabyte)) {
    return round($bytes / $megabyte, $precision) . ' MB';
  }
  elseif (($bytes >= $gigabyte) && ($bytes < $terabyte)) {
    return round($bytes / $gigabyte, $precision) . ' GB';
  }
  elseif ($bytes >= $terabyte) {
    return round($bytes / $terabyte, $precision) . ' TB';
  }
  else {
    return $bytes . ' B';
  }
}

/**
 * Creates a label for control group symbols.
 */
function islandora_control_group_to_human_readable($control_group) {
  switch ($control_group) {
    case 'M':
      return '<b>M</b>anaged';
    case 'X':
      return 'Inline <b>X</b>ML';
    case 'R':
      return '<b>R</b>edirect';
    case 'E':
      return '<b>E</b>xternally Referenced';
    default:
      return $control_group;
  }
}

/**
 * valid pid ??
 * @param type $pid
 * @return boolean
 */
function islandora_validate_pid($pid) {
  $valid = FALSE;
  if (drupal_strlen(trim($pid)) <= 64 && preg_match('/^([A-Za-z0-9]|-|\.)+:(([A-Za-z0-9])|-|\.|~|_|(%[0-9A-F]{2}))+$/', trim($pid))) {
    $valid = TRUE;
  }

  return $valid;
}

/**
 * Valid Dsid ??
 * @param type $dsid
 * @return boolean
 */
function islandora_validate_dsid($dsid) {
  $valid = FALSE;
  if (drupal_strlen(trim($dsid)) <= 64 && preg_match('/^[a-zA-Z0-9\_\-\.]+$/', trim($dsid))) {
    $valid = TRUE;
  }

  return $valid;
}

/* Helper function to describe a Fedora repository.
 *
 * Can be used to check if Fedora is available.
 *
 * @param $url
 *   A url to a Fedora repository.
 * @return
 *   Returns an array describing the repository. Returns FALSE if Fedora is down
 *   or if the url is incorrect.
 */
function islandora_describe_repository($url) {
  module_load_include('inc', 'islandora', 'includes/tuque');
  $connection = new IslandoraTuque(NULL, $url);
  try {
    $info = $connection->api->a->describeRepository();
    return $info;
  }
  catch (RepositoryException $e) {
    return FALSE;
  }
}

/**
 * Build a list of all the hooks to call.
 *
 * Concatenates the each pid (escaped) to the hook name, for calling in
 * module_invoke_all().
 *
 * @param string $hook
 *   A hook to call.
 * @param array $pids
 *   An array of PIDs (probably content models).
 *
 * @return array
 *   An array with each PID escaped and concatenated with the base hook name,
 *   in addition to the base hook name at the end.
 */
function islandora_build_hook_list($hook, $pids = array()) {
  $hooks = array();

  foreach ($pids as $model) {
    $hooks[] = islandora_escape_pid_for_function($model) . '_' . $hook;
  }
  $hooks[] = $hook;

  return $hooks;
}

/**
 * Escape a Fedora PID to be valid inside of a PHP function name.
 *
 * Originally intended to allow inclusion of a PID in a module_invoke_all()
 * call.
 */
function islandora_escape_pid_for_function($pid) {
  // Apparently, case doesn't matter for function calls in PHP, so let's not
  //   really worry about changing the case.
  return str_replace(
    // Any PID characters which are not valid in the name of a PHP function.
    array(
      ':',
      '-',
    ),
    '_',
    $pid
  );
}
