<?php

/**
 * @file
 * This file contains ingest callback functions
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
 * @TODO: needs documentation
 */
function islandora_ingest_get_object($content_models, $collection_pid, $relationship, $namespace) {
  module_load_include('inc', 'islandora', 'includes/tuque');
  global $user;
  $connection = new IslandoraTuque($user);
  $object = $connection->repository->constructObject($namespace);
  foreach ($content_models as $content_model) {
    $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', $content_model['pid']);
  }
  $object->relationships->add(FEDORA_RELS_EXT_URI, $relationship, $collection_pid);
  module_invoke_all('islandora_ingest_pre_ingest', $object, $content_models, $collection_pid);
  return $object;
}

/**
 * @TODO: needs documentation
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

  module_invoke_all('islandora_ingest_pre_ingest', $object, $content_models, $collection_pid);
  return $object;
}

/**
 * @TODO: needs documentation
 */
function islandora_ingest_add_object(&$object) {
  $object->repository->ingestObject($object);
  module_invoke_all('islandora_ingest_post_ingest', $object);
  return $object;
}


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





