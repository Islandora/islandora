<?php

/**
 * @file
 * This file lists and documents all available hook functions to manipulate data.
 */

/**
 * Generate a repository objects view.
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject being operated on.
 * @param object $user
 *   The user accessing the object.
 * @param string $page_number
 *   The page in the content.
 * @param string $page_size: The size of the page.
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_islandora_view_object($fedora_object, $user, $page_number, $page_size) {}

/**
 * Generate an object's display for the given content model.
 *
 * Content models PIDs have colons and hyphens changed to underscores, to
 * create the hook name.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_CMODEL_PID_islandora_view_object($fedora_object) {}


/**
 * Alter display output after it has been generated.
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject being operated on.
 * @param array $arr
 *   An arr of rendered views.
 */
function hook_islandora_view_object_alter(&$fedora_object, &$arr) {}

/**
 * Generate an object's management display.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_islandora_edit_object($fedora_object) {}

/**
 * Generate an object's management display for the given content model.
 *
 * Content models PIDs have colons and hyphens changed to underscores, to
 * create the hook name.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_CMODEL_PID_islandora_edit_object($fedora_object) {}

/**
 * Allow management display output to be altered.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 * @param type $arr
 *   an arr of rendered views
 */
function hook_islandora_edit_object_alter(&$fedora_object, &$arr) {}

/**
 * Allows modules to add to an objects ingest process.
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject.
 */
function hook_islandora_ingest_post_ingest($fedora_object) {}

/**
 * Allow modules to add to the ingest process of a specific content model.
 */
function hook_CMODEL_PID_islandora_ingest_post_ingest($fedora_object) {}

/**
 * Allows modules to add to a repository objects view/edit(/misc) process.
 *
 * If you implement this hook you must also register your module with
 * hook_islandora_hook_info().
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject.
 *
 * @return array|null
 *   An associative array with 'deleted' mapped to TRUE--indicating that the
 *   object should just be marked as deleted, instead of actually being
 *   purged--or NULL/no return if we just need to do something before the
 *   is purged.
 */
function hook_islandora_pre_purge_object($fedora_object) {}

/**
 * Allow modules to react to the purge process of a specific content model.
 *
 * @see hook_islandora_pre_purge_object()
 */
function hook_CMODEL_PID_islandora_pre_purge_object($fedora_object) {}

/**
 * Register potential ingest routes.
 *
 * Implementations should return an array containing possible routes.
 */
function hook_islandora_ingest_registry($collection_object) {
  $reg = array(
    array(
      'name' => t('Ingest Route Name'),
      'url' => 'ingest_route/url',
      'weight' => 0,
    ),
  );
  return $reg;
}

/**
 * Register a datastream edit route/form.
 *
 * @param $islandora_object
 * @param $ds_id
 */
function hook_islandora_edit_datastream_registry($islandora_object, $ds_id) {}

/**
 * Alter an object before it gets used further down the stack.
 *
 * @param type $object
 *   A Tuque FedoraObject
 */
function hook_islandora_object_alter($fedora_object) {}

/**
 * Allow modification of an object before ingesting.
 *
 * @param type $islandora_object
 *   A Tuque FedoraObject
 * @param array $content_models
 * @param string $collection_pid
 */
function hook_islandora_ingest_pre_ingest($islandora_object, $content_models, $collection_pid) {}

/**
 * Allow modification of objects of a certain content model before ingesting.
 *
 * @see hook_islandora_ingest_pre_ingest()
 */
function hook_CMODEL_PID_islandora_ingest_pre_ingest($islandora_object, $content_models, $collection_pid) {}

/**
 * Allow modules to setup for the purge of a datastream.
 *
 * @param object $datastream
 *   A Tuque FedoraDatastream object.
 */
function hook_islandora_pre_purge_datastream($datastream) {}

/**
 * Allow modules to react after a datastream is purged.
 *
 * @param object $object
 *   A Tuque FedoraObject.
 * @param string $dsid
 *   A id of the former datastream.
 */
function hook_islandora_post_purge_datastream($object, $dsid) {}

/**
 * Allow modules to react post-purge.
 *
 * @param string $object_id
 *   The former object's PID.
 * @param array $content_models
 *   An array containing the models to which the former object.
 */
function hook_islandora_post_purge_object($object_id, $content_models) {}

