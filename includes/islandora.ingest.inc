<?php

/**
 * @file
 * This file contains ingest callback functions
 */

function islandora_ingest_get_information(AbstractFedoraObject $collection_object) {
  $models = $collection_object->models;
  $collection_info = module_invoke_all('islandora_ingest_get_information', $models, $collection_object);

  return $collection_info;
}

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

function islandora_ingest_add_object(&$object) {
  $object->repository->ingestObject($object);
  module_invoke_all('islandora_ingest_post_ingest', $object);
  return $object;
}