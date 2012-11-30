<?php

/**
 * @file
 * Contains islandora utility functions
 *
 * @todo this file should be broken out into other files.
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
 * Checks if the given pid is valid.
 *
 * @param string $pid
 *   The object id to check.
 *
 * @return boolean
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
 * @return boolean
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
 * @return
 *   Returns an array describing the repository. Returns FALSE if Fedora is down
 *   or if the url is incorrect.
 */
function islandora_describe_repository($url = NULL) {
  $url = isset($url) ? $url : variable_get('islandora_base_url', 'http://localhost:8080/fedora');
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

  $pids = array_unique($pids);
  foreach ($pids as $pid) {
    $hooks[] = islandora_escape_pid_for_function($pid) . '_' . $hook;
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
 * Checks the given namespace or PID is/has an accessible namespace as defined
 * by the "islandora_pids_allowed" variable.
 *
 * @param string $namespace
 *   Either a PID or namespace to check for accessibility. Any string like those
 *   below are fine.
 *
 * @code
 *  'islandora',
 *  'islandora:',
 *  'islandora:1234',
 * @endcode
 *
 * @return boolean
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
 * Gets any objects that the given object has a
 * (isMemberOf, isMemberOfCollection) relationship with.
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
  $collections = array_map(function($o) { return islandora_object_load($o['object']['value']); }, $collections);
  return array_filter($collections);
}

/**
 * Checks what datastreams the object already has against its required
 * datastreams as defined by its content models, and returns their intersection.
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
 * Checks the object's content model's for which datastream are expected to be
 * used with this object, as defined by the DS-COMPOSITE-MODEL datastreams.
 *
 * For duplicate datastreams in the models, the first model defines the
 * datastreams attributes regardless of what other models define.
 * This should be undefined behavior according to the documentation.
 * @see https://wiki.duraspace.org/display/FEDORA34/Fedora+Digital+Object+Model#FedoraDigitalObjectModel-ContentModelObjectCMODEL
 *
 * @param FedoraObject $object
 *   The object which models will be used to determine what datastreams it
 *   should have.
 *
 * @see islandora_get_required_datastreams_from_content_model() from more
 *   details on the return value.
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
  foreach ($models as  $model) {
    $model = islandora_object_load($model);
    $dsids += islandora_get_datastreams_requirements_from_content_model($model);
  }
  // The AUDIT Datastream can not really be added, so it can't really be missing.
  unset($dsids['AUDIT']);
  return $dsids;
}

/**
 * Checks the given content model for which datastreams are required for
 * subscribing objects, as defined by it's DS-COMPOSITE-MODEL datastream.
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
      'optional' => ($optional == 'true') ? TRUE : FALSE
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
  module_load_include('inc', 'islandora', 'includes/IslandoraTuque');
  global $user;
  $tuque = islandora_get_tuque_connection();
  $object = isset($namespace) ? $tuque->repository->constructObject($namespace) : new NewFedoraObject(NULL, $tuque->repository);
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
    $control_group = (isset($ds['control_group']) && in_array($ds['control_group'], array('X', 'M', 'R', 'E'))) ? $ds['control_group'] : 'M';
    $datastream_file = url($ds['datastream_file'], array('absolute' => TRUE));
    $datastream = $object->constructDatastream($dsid, $control_group);
    $datastream->label = $label;
    $datastream->mimetype = $mimetype;
    switch ($control_group) {
      case 'M':
        $datastream->setContentFromUrl($datastream_file);
        break;
      case 'X':
        $datastream->setContentFromString(file_get_contents($datastream_file));
        break;
    }
    $object->ingestDatastream($datastream);
  }
  return $object;
}
