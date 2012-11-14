<?php

/**
 * @file
 * This file contains ingest callback functions.
 */

/**
 * @TODO: needs documentation
 */
function islandora_ingest_get_information(AbstractFedoraObject $collection_object) {
  $models = $collection_object->models;
  $collection_info = module_invoke_all('islandora_ingest_get_information', $models, $collection_object);

  return $collection_info;
}

/**
 * Get an ingestable object.
 * 
 * @deprecated
 *   Deprecated in favour of the more flexible
 *   islandora_ingest_new_object_prepare()--which this function has been made
 *   to call behind the scenes anyway.
 *
 * @param array $content_models
 *   An array of content models to which the new object should subscribe, where
 *   each content model is described by an associative array containing:
 *   - pid: The Fedora PID of the content model.
 * @param string $collection_pid
 *   The collection to which the new object should belong.
 * @param string $relationship
 *   The relationship this object will have to the collection.
 * @param string $namespace
 *   The namespace in which the PID for the new object will be created.
 *
 * @return NewFedoraObject
 *   A NewFedoraObject which may be adjusted before ingesting.
 */
function islandora_ingest_get_object($content_models, $collection_pid, $relationship, $namespace) {
  $models = array();
  foreach ($content_models as $relation) {
    $models[] = $relation['pid'];
  }

  return islandora_ingest_new_object_prepare($namespace, NULL, array(), $models, array(
    array(
      'pid' => $collection_pid,
      'relationship' => $relationship,
    ),
  ), $collection_pid);
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
function islandora_ingest_new_object_prepare($namespace = NULL, $label = NULL, $datastreams = array(), $content_models = array(), $relationships = array(), $collection_pid = NULL) {
  // include Tuque library
  module_load_include('inc', 'islandora', 'includes/tuque');
  global $user;
  // new connection
  try {
    $connection = new IslandoraTuque($user);
  } catch (Exception $e) {
    drupal_set_message(t('Unable to connect to the repository %e', array('%e' => $e)), 'error');
    return;
  }
  // construct new object
  $object = $connection->repository->constructObject($namespace);

  // add label
  if (!empty($label)) {
    $object->label = $label;
  }
  // add content model relationship(s)
  foreach ($content_models as $content_model) {
    $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', $content_model);
  }
  // add collection relationship(s)
  if (!empty($relationships)) {
    foreach ($relationships as $relationship) {
      $object->relationships->add(FEDORA_RELS_EXT_URI, $relationship['relationship'], $relationship['pid']);
    }
  }
  // add datastreams
  foreach ((array) $datastreams as $ds) {
    // variables
    $ds_id = $ds['dsid'];
    $ds_label = isset($ds['label']) ? $ds['label'] : '';
    $ds_mimetype = isset($ds['mimetype']) ? $ds['mimetype'] : 'text/xml';
    $ds_control_group = (isset($ds['control_group']) AND in_array($ds['control_group'], array('X', 'M', 'R', 'E'))) ? $ds['control_group'] : 'M';
    $ds_datastream_file = url($ds['datastream_file'], array('absolute' => TRUE));

    // datastream object
    $datastream = $object->constructDatastream($ds_id, $ds_control_group);
    $datastream->label = $ds_label;
    $datastream->mimetype = $ds_mimetype;
    switch ($ds_control_group) {
      case 'M':
        $datastream->setContentFromUrl($ds_datastream_file);
        break;
      case 'X':
        $datastream->setContentFromString(file_get_contents($ds_datastream_file));
        break;
    }
    $object->ingestDatastream($datastream);
  }

  module_load_include('inc', 'islandora', 'includes/utilities');
  foreach (islandora_build_hook_list('islandora_ingest_pre_ingest', $content_models) as $hook) {
    module_invoke_all($hook, $object, $content_models, $collection_pid);
  }

  return $object;
}

/**
 * Ingest the given object into Fedora.
 *
 * @param NewFedoraObject $object
 *   An ingestable FedoraObject.
 *
 * @return FedoraObject
 *   The ingested FedoraObject, after running the post ingest hooks.
 */
function islandora_ingest_add_object(&$object) {
  drupal_alter('islandora_pre_ingest', $object);

  $object->repository->ingestObject($object);

  module_load_include('inc', 'islandora', 'includes/utilities');

  foreach (islandora_build_hook_list(ISLANDORA_POST_INGEST_HOOK, $object->models) as $hook) {
    module_invoke_all($hook, $object);
  }

  return $object;
}

/**
 * Ingest an object.
 *
 * @param array $object_model
 *   An associative array containing the necessary parameters to create the
 *   desired object:
 *   - pid: The PID with which the object will be created.
 *   - label: An optional label to apply to the object.
 *   - datastreams: Same as the "datastreams" array accepted by
 *     islandora_ingest_new_object_prepare().
 *   - cmodel: Either an array of content models as accepted by
 *     islandora_ingest_new_object_prepare(), or a single content model PID to add
 *     to the object.
 *   - parent: Either an array of parents, or a single parent PID to which to
 *     relate to; uses isMemberOfCollection by default.
 *   - relationships: An array of relationships as accepted by
 *     islandora_ingest_new_object_prepare().
 *
 * @return FedoraObject
 *   An FedoraObject which has been ingested into Fedora.
 */
function islandora_ingest_new_object($object_model) {
  // prepare variables
  // namespace
  $namespace = $object_model['pid'];
  // label
  $label = !empty($object_model['label']) ? $object_model['label'] : NULL;
  // datastreams
  $datastreams = array();
  if (!empty($object_model['datastreams']) AND is_array($object_model['datastreams'])) {
    $datastreams = $object_model['datastreams'];
  }
  // content models
  $content_models = array();
  if (!empty($object_model['cmodel'])) {
    if (is_array($object_model['cmodel'])) {
      $content_models = $object_model['cmodel'];
    }
    else {
      $content_models[] = $object_model['cmodel'];
    }
  }
  // relationships
  $relationships = array();
  // single parent
  if (!empty($object_model['parent']) AND !is_array($object_model['parent'])) {
    $relationships[] = array('relationship' => 'isMemberOfCollection', 'pid' => $object_model['parent']);
  }
  // parents array
  if (!empty($object_model['parents']) AND is_array($object_model['parents'])) {
    foreach ($object_model['parents'] as $parent) {
      $relationships[] = array('relationship' => 'isMemberOfCollection', 'pid' => $parent);
    }
  }
  // other relationships
  if (!empty($object_model['relationships']) AND is_array($object_model['relationships'])) {
    foreach ($object_model['relationships'] as $relationship) {
      $relationships[] = array('relationship' => $relationship['relationship'], 'pid' => $relationship['pid']);
    }
  }

  // build new object
  $object = islandora_ingest_new_object_prepare($namespace, $label, $datastreams, $content_models, $relationships);

  // ingest (and return) new object
  return islandora_ingest_add_object($object);
}





