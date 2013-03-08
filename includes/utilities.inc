<?php

/**
 * @file
 * Utility functions.
 *
 * @todo this file should be broken out into other files.
 */

/**
 * Convert bytes to human readable format.
 *
 * XXX: Shouldn't Drupal's format_size() be preferred?
 *
 * @param int $bytes
 *   Size in bytes to convert
 *
 * @return string
 *   Human readable size.
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
 * Checks if the given pid is valid.
 *
 * @param string $pid
 *   The object id to check.
 *
 * @return bool
 *   TRUE if valid, FALSE otherwise.
 */
function islandora_is_valid_pid($pid) {
  return drupal_strlen(trim($pid)) <= 64 && preg_match('/^([A-Za-z0-9]|-|\.)+:(([A-Za-z0-9])|-|\.|~|_|(%[0-9A-F]{2}))+$/', trim($pid));
}

/**
 * Checks if the given datastream id is valid.
 *
 * @param string $dsid
 *   The datastream id to check.
 *
 * @return bool
 *   TRUE if valid, FALSE otherwise.
 */
function islandora_is_valid_dsid($dsid) {
  return drupal_strlen(trim($dsid)) <= 64 && preg_match('/^[a-zA-Z0-9\_\-\.]+$/', trim($dsid));
}

/**
 * Helper function to describe a Fedora repository.
 *
 * Can be used to check if Fedora is available.
 *
 * @param string $url
 *   A url to a Fedora repository, if NULL the default is used.
 *
 * @return array
 *   Returns an array describing the repository. Returns FALSE if Fedora is down
 *   or if the url is incorrect.
 */
function islandora_describe_repository($url = NULL) {
  $url = isset($url) ? $url : variable_get('islandora_base_url', 'http://localhost:8080/fedora');
  $connection = islandora_get_tuque_connection();
  if ($connection) {
    try {
      $info = $connection->api->a->describeRepository();
      return $info;
    }
    catch (RepositoryException $e) {
      return FALSE;
    }
  }
  return FALSE;
}

/**
 * Build and invoke a list of hooks by combining the given hook and refinements.
 *
 * The given hook will be called as MODULE_HOOK, and for each hook refinement
 * as MODULE_REFINEMENT_HOOK. Any additional arguments passed to this function
 * will be passed as arguments to module_invoke_all().
 *
 * @see islandora_build_hook_list()
 *
 * @param string $hook
 *   A hook to call.
 * @param array $refinements
 *   An array of strings, that will be escaped and concatinated with the given
 *   hook. This will most likely be PIDs/DSIDs/Labels etc. We often refine our
 *   hooks using an objects model.
 * @param array $args
 *   Any arguments to pass onto module_invoke_all().
 *
 * @return array
 *   The merged results from all the hooks.
 */
function islandora_invoke_hook_list($hook, array $refinements, array $args) {
  $return = array();
  foreach (islandora_build_hook_list($hook, $refinements) as $hook) {
    array_unshift($args, $hook);
    $result = call_user_func_array('module_invoke_all', $args);
    $return = array_merge_recursive($return, $result);
    array_shift($args);
  }
  return $return;
}

/**
 * Build a list of all the hooks to call.
 *
 * Concatenates each hook $refinement (escaped) to the hook name, for calling
 * with module_invoke_all().
 *
 * Any non-valid PHP function characters in the given refinements are
 * converted to "_" characters.
 *
 * @param string $hook
 *   The base hook to concatenate.
 * @param array $refinements
 *   An array of strings, that will be escaped and concatinated with the given
 *   hook. This will most likely be PIDs/DSIDs/Labels etc. We often refine our
 *   hooks using an objects model.
 *
 * @return array
 *   An array with each refinement escaped and concatenated with the base hook
 *   name, in addition to the base hook name.
 */
function islandora_build_hook_list($hook, $refinements = array()) {
  $refinements = array_unique($refinements);
  $hooks = array($hook);
  foreach ($refinements as $refinement) {
    $refinement = preg_replace('/[^a-zA-Z0-9_]/', '_', $refinement);
    $hooks[] = "{$refinement}_{$hook}";
  }
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
  // really worry about changing the case.
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

/**
 * Gets the namespace of the given id.
 *
 * @param string $id
 *   Either a PID or namespace to check for accessibility. Any string like those
 *   below are fine.
 *
 * @code
 *  'islandora',
 *  'islandora:',
 *  'islandora:1234',
 * @endcode
 *
 * @return string
 *   The namespace portion of the given string.
 */
function islandora_get_namespace($id) {
  $matches = array();
  preg_match('/^([^:]*)/', $id, $matches);
  return $matches[0];
}

/**
 * Checks the given namespace/PID is/has an  accessible namespace.
 *
 * Accessible is defined by the "islandora_pids_allowed" variable.
 *
 * @param string $id
 *   Either a PID or namespace to check for accessibility. Any string like those
 *   below are fine.
 *
 * @code
 *  'islandora',
 *  'islandora:',
 *  'islandora:1234',
 * @endcode
 *
 * @return bool
 *   TRUE if accessible, FALSE otherwise.
 */
function islandora_namespace_accessible($id) {
  if (variable_get('islandora_namespace_restriction_enforced', FALSE)) {
    $namespace = islandora_get_namespace($id) . ':';
    $allowed_namespaces = explode(" ", variable_get('islandora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '));
    return in_array($namespace, $allowed_namespaces);
  }
  return TRUE;
}

/**
 * Gets any objects that the given object has a parent relationship with.
 *
 * Parent relationships are defined as (isMemberOf, isMemberOfCollection).
 *
 * This function gets its info from the RELS-EXT directly rather than through an
 * risearch.
 *
 * @param FedoraObject $object
 *   The object whose parents will be returned.
 *
 * @return array
 *   An array of FedoraObject's that the given object has a
 *   (isMemberOf, isMemberOfCollection) relationship with.
 */
function islandora_get_parents_from_rels_ext(FedoraObject $object) {
  try {
    $collections = array_merge(
      $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection'),
      $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf'));
  }
  catch (RepositoryException $e) {
    // @todo some logging would be nice, not sure what this throws.
    return array();
  }
  $map = function($o) {
    return islandora_object_load($o['object']['value']);
  };
  $collections = array_map($map, $collections);
  return array_filter($collections);
}

/**
 * Gets the datastreams requirments that are missing.
 *
 * @param FedoraObject $object
 *   The object which models will be used to determine what datastreams it
 *   should have.
 *
 * @return array
 *   The DS-COMPOSITE-MODEL defined datastreams that are required for the given
 *   object, but not already present.
 */
function islandora_get_missing_datastreams_requirements(FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $datastreams = islandora_get_datastreams_requirements($object);
  foreach ($datastreams as $dsid => $requirements) {
    if (isset($object[$dsid])) {
      unset($datastreams[$dsid]);
    }
  }
  return $datastreams;
}

/**
 * Gets the required datastreams for the given object.
 *
 * Checks the object's content model's for which datastream are expected to be
 * used with this object, as defined by the DS-COMPOSITE-MODEL datastreams.
 *
 * For duplicate datastreams in the models, the first model defines the
 * datastreams attributes regardless of what other models define.
 * This should be undefined behavior according to the documentation.
 *
 * @link https://wiki.duraspace.org/display/FEDORA34/Fedora+Digital+Object+Model#FedoraDigitalObjectModel-ContentModelObjectCMODEL @endlink
 *
 * @see islandora_get_required_datastreams_from_content_model()
 *
 * @param FedoraObject $object
 *   The object which models will be used to determine what datastreams it
 *   should have.
 *
 * @return array
 *   The DS-COMPOSITE-MODEL defined datastreams that are required for the given
 *   object.
 */
function islandora_get_datastreams_requirements(FedoraObject $object) {
  return islandora_get_datastreams_requirements_from_models($object->models);
}

/**
 * Get the list of which datastreams are valid in the given set of models.
 *
 * @param array $models
 *   An array of content models PIDs from which to parse the DS-COMPOSITE-MODEL
 *   stream.
 *
 * @return array
 *   An associative array of associative arrays, merged from calls to
 *   islandora_get_datastreams_requirements_from_content_model().
 */
function islandora_get_datastreams_requirements_from_models(array $models) {
  $dsids = array();
  foreach ($models as $model_pid) {
    $model = islandora_object_load($model_pid);
    if (isset($model) && $model) {
      $dsids += islandora_get_datastreams_requirements_from_content_model($model);
    }
  }
  // The AUDIT Datastream can not really be added, so it can't really be
  // missing.
  unset($dsids['AUDIT']);
  return $dsids;
}

/**
 * Checks the given content model for which datastreams are required.
 *
 * As defined by it's DS-COMPOSITE-MODEL datastream.
 *
 * @todo Add support for fetching the schema information.
 *
 * @param FedoraObject $object
 *   The content model whose DS-COMPOSITE-MODEL datastream will be used to
 *   determine what datastreams are required.
 *
 * @return array
 *   The DS-COMPOSITE-MODEL defined datastreams that are required for the given
 *   object.
 *
 * @code
 *   array(
 *     'DC' => array(
 *       'id' => 'DC',
 *       'mime' => 'text/xml',
 *       'optional' => FALSE,
 *     )
 *   )
 * @endcode
 */
function islandora_get_datastreams_requirements_from_content_model(FedoraObject $object) {
  if (empty($object[DS_COMP_STREAM])) {
    return array();
  }
  $xml = new SimpleXMLElement($object[DS_COMP_STREAM]->content);
  foreach ($xml->dsTypeModel as $ds) {
    $dsid = (string) $ds['ID'];
    $optional = strtolower((string) $ds['optional']);
    $mime = array();
    foreach ($ds->form as $form) {
      $mime[] = (string) $form['MIME'];
    }
    $dsids[$dsid] = array(
      'id' => $dsid,
      'mime' => $mime,
      'optional' => ($optional == 'true') ? TRUE : FALSE,
    );
  }
  return $dsids;
}

/**
 * Prepare an ingestable object.
 *
 * @param string $namespace
 *   The namespace in which the PID for the new object will be created.
 * @param string $label
 *   An optional label to apply to the object.
 * @param array $datastreams
 *   A array of datastreams to add, where each datastream definition is an
 *   associative array containing:
 *   - dsid: The datastream ID.
 *   - label: An optional label for the datastream.
 *   - mimetype: A MIMEtype for the datastream; defaults to text/xml.
 *   - control_group: One of X, M, R and E; defaults to M.
 *   - datastream_file: A web-accessible path, for which we try to get an
 *     absolute path using url().
 * @param array $content_models
 *   An array of content model PIDs to which the new object should subscribe.
 * @param array $relationships
 *   An array of relationships, where each relationship is an associative array
 *   containing:
 *   - relationship: The predicate for the relationship, from the Fedora
 *     RELS-EXT namespace.
 *   - pid: The object for the relationship, to which we are creating the
 *     relationhsip.
 *
 * @return NewFedoraObject
 *   An ingestable NewFedoraObject.
 */
function islandora_prepare_new_object($namespace = NULL, $label = NULL, $datastreams = array(), $content_models = array(), $relationships = array()) {
  global $user;
  $tuque = islandora_get_tuque_connection();
  $object = isset($namespace) ? $tuque->repository->constructObject($namespace) : new IslandoraNewFedoraObject(NULL, $tuque->repository);
  $object->owner = isset($user->name) ? $user->name : $object->owner;
  $object->label = isset($label) ? $label : $object->label;
  foreach ($content_models as $content_model) {
    $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', $content_model);
  }
  foreach ($relationships as $relationship) {
    $object->relationships->add(FEDORA_RELS_EXT_URI, $relationship['relationship'], $relationship['pid']);
  }
  foreach ($datastreams as $ds) {
    $dsid = $ds['dsid'];
    $label = isset($ds['label']) ? $ds['label'] : '';
    $mimetype = isset($ds['mimetype']) ? $ds['mimetype'] : 'text/xml';
    // Default 'Managed'
    $control_group = 'M';
    $groups = array('X', 'M', 'R', 'E');
    if (isset($ds['control_group']) && in_array($ds['control_group'], $groups)) {
      $control_group = $ds['control_group'];
    }
    $ds_uri = FALSE;
    if (file_valid_uri($ds['datastream_file'])) {
      $datastream_file = $ds['datastream_file'];
      $ds_uri = TRUE;
    }
    else {
      $datastream_file = url($ds['datastream_file'], array('absolute' => TRUE));
    }
    $datastream = $object->constructDatastream($dsid, $control_group);
    $datastream->label = $label;
    $datastream->mimetype = $mimetype;
    switch ($control_group) {
      case 'M':
        if ($ds_uri) {
          $datastream->setContentFromFile($datastream_file);
        }
        else {
          $datastream->setContentFromUrl($datastream_file);
        }
        break;

      case 'X':
        $datastream->setContentFromString(file_get_contents($datastream_file));
        break;
    }
    $object->ingestDatastream($datastream);
  }
  return $object;
}

/**
 * Displays the repository is inaccessible message.
 *
 * Use anywhere we want to ensure a consitent error message when the repository
 * is not accessible.
 */
function islandora_display_repository_inaccessible_message() {
  $text = t('Islandora configuration');
  $link = l($text, 'admin/islandora/configure', array('attributes' => array('title' => $text)));
  $message = t('Could not connect to the repository. Please check the settings on the !link page.',
             array('!link' => $link));
  drupal_set_message($message, 'error', FALSE);
}

/**
 * Create a message stating if the given executable is available.
 *
 * If the both the version and required version are given then only if the
 * version is equal to or greater than the required version the executable
 * will be considered correct.
 *
 * @param string $path
 *   The absolute path to an executable to check for availability.
 * @param string $version
 *   The version of exectuable.
 * @param string $required_version
 *   The required version of exectuable.
 *
 * @return string
 *   A message in html detailing if the given executable is accessible.
 */
function islandora_executable_available_message($path, $version = NULL, $required_version = NULL) {
  $available = is_executable($path);
  if ($available) {
    $image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
    $message = t('Executable found at @path', array('@path' => $path));
    if ($version) {
      $message .= t('<br/>Version: @version', array('@version' => $version));
    }
    if ($required_version) {
      $message .= t('<br/>Required Version: @version', array('@version' => $required_version));
      if (version_compare($version, $required_version) < 0) {
        $image = theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array()));
      }
    }
  }
  else {
    $image = theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array()));
    $message = t('Unable to locate executable at @path', array('@path' => $path));
  }
  return $image . $message;
}

/**
 * Create a message stating if the given directory exists.
 *
 * @param string $path
 *   The absolute path to an executable to check for availability.
 *
 * @return string
 *   A message in HTML detailing if the given directory is exists.
 */
function islandora_directory_exists_message($path) {
  $available = is_dir($path);
  if ($available) {
    $image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
    $message = t('Directory found at @path', array('@path' => $path));
  }
  else {
    $image = theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array()));
    $message = t('Unable to locate directory at @path', array('@path' => $path));
  }
  return $image . $message;
}

/**
 * Gets the default value for the given system_settings_form() element.
 *
 * Checks the $form_state for the default value if not it checks
 * variable_get(). Assumes #tree is FALSE. This is only really required
 * for elements that utilize AJAX, as their value can change before the
 * submit actually takes place.
 *
 * @param string $name
 *   The name of the system settings form element.
 * @param mixed $default_value
 *   The default value for the system settings form element.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return mixed
 *   The default value for use in a the system_settings_form().
 */
function islandora_system_settings_form_default_value($name, $default_value, array &$form_state) {
  return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default_value);
}

/**
 * Returns basic information about DS-COMPOSITE-MODEL.
 *
 * @param string $pid
 *   PID of content model containing DS_COMP stream.
 *
 * @return array
 *   array of values in the following form
 *
 * [DC] => Array
 *       (
 *           [mimetype] => text/xml
 *       )
 *
 *   [MODS] => Array
 *       (
 *           [optional] => true
 *           [mimetype] => text/xml
 *       )
 *
 *   [RELS-EXT] => Array
 *       (
 *           [mimetype] => application/rdf+xml
 *       )
 *
 *   [RELS-INT] => Array
 *       (
 *           [optional] => true
 *           [mimetype] => application/rdf+xml
 *      )
 */
function islandora_get_comp_ds_mappings($pid) {
  $cm_object = islandora_object_load($pid);
  if (!isset($cm_object) || !isset($cm_object['DS-COMPOSITE-MODEL'])) {
    return FALSE;
  }
  $datastream = $cm_object['DS-COMPOSITE-MODEL'];
  $ds_comp_stream = $datastream->content;
  $sxml = new SimpleXMLElement($ds_comp_stream);
  $mappings = array();
  foreach ($sxml->dsTypeModel as $ds) {
    $dsid = (string) $ds['ID'];
    $optional = (string) $ds['optional'];
    foreach ($ds->form as $form) {
      $mime = (string) $form['MIME'];
      if ($optional) {
        $mappings[$dsid]['optional'] = $optional;
      }
      $mappings[$dsid]['mimetype'] = $mime;
    }
  }
  return $mappings;
}

/**
 * Checks that the given/current account has all the given permissions.
 *
 * @param array $perms
 *   The permissions to check.
 * @param mixed $account
 *   (optional) The account to check, if not given use currently logged in user.
 *
 * @return bool
 *   TRUE if the account has all the given permissions, FALSE otherwise.
 */
function islandora_user_access_all(array $perms, $account = NULL) {
  foreach ($perms as $perm) {
    if (!user_access($perm, $account)) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Checks that the given/current account has at one of the given permissions.
 *
 * @param array $perms
 *   The permissions to check.
 * @param mixed $account
 *   (optional) The account to check, if not given use currently logged in user.
 *
 * @return bool
 *   TRUE if the account has at least one of the given permissions, FALSE
 *   otherwise.
 */
function islandora_user_access_any(array $perms, $account = NULL) {
  foreach ($perms as $perm) {
    if (user_access($perm, $account)) {
      return TRUE;
    }
  }
  return FALSE;
}
