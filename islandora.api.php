<?php

/**
 * @file
 * This file lists and documents all available hook functions to manipulate data.
 */

/**
 * remove a datastream from a repository object
 * @param object $fedora_object
 *  tuque FedoraObject
 * @param string $datastream_id
 */
function hook_islandora_purge_datastream($fedora_object, $datastream_id) {}

/**
 *
 * @param type $object
 *   tuque FedoraObject
 */
function hook_islandora_purge_object($islandora_object) {}

/**
 * allows modules to add to a repository objects display.  If you implement this
 * hook you should also register your module for view with the get types hook.
 *
 * islandora gets all displays back in an array and iterates over them.  the order
 * they are displayed is based on the order of the key of the array that each
 * module returns.
 *
 * your module may also want to register a varible that says whether or not it
 * should be part of the default display. Modules can also add secondary tabs as
 * a way to add there output to an islandora display.  the basic image module has
 * samples of both (the secondary tabs examples are commented out)
 *
 * @param type $islandora_object
 *   tuque FedoraObject
 */
function hook_islandora_view_object($islandora_object) {}

/**
 * returns an array listing object types provided by sub modules
 *  Ex.  array($types['islandora:collectionCModel'][ISLANDORA_VIEW_HOOK] = variable_get('islandora_basic_collection_use_for_default_tab', TRUE);
 * $types['islandora:collectionCModel'][ISLANDORA_EDIT_HOOK] = FALSE;
 *
 * @return array
 */
function hook_islandora_get_types() {}

/**
 * allows modules to define an object edit page by cmodel
 *
 * your module should return true for ISLANDORA_EDIT_HOOK in its get_types function
 *
 * islandora provides a default implementation that should work for most use cases
 * @param string $islandora_object
 * @return string
 *
 */
function hook_islandora_edit_object($islandora_object) {}

/**
 * allows modules to alter the fedora object before it is pass through the edit
 * hooks
 * @param type $islandora_object
 *   a tugue FedoraObject
 */
function islandora_islandora_edit_object_alter($islandora_object) {}


/**
 * creates and populates a php Fedora object.
 */
function hook_islandora_preingest_alter() {}

/**
 * modules can implement this hook to add or remove datastreams after an
 * object has been ingested.
 *
 * Each module should check for the newly ingested repository objects content
 * model to make sure it is a type of object they want to act on.
 *
 * @param type $islandora_object
 *   tugue FeodoraObject
 */
function hook_islandora_postingest($islandora_object) {}

/**
 * Register potential ingest routes. Implementations should return an array containing possible routes.
 * Ex. array(
 *       array('name' => t('Ingest Route Name'), 'url' => 'ingest_route/url', 'weight' => 0),
 *     );
 */
function hook_islandora_ingest_registry($collection_object) {}

/**
 * Register a datastream edit route/form.
 * @param $islandora_object
 * @param $ds_id
 */
function hook_islandora_edit_datastream_registry($islandora_object, $ds_id) {}

/**
 * alter an object before it gets used further down the stack
 * @param type $object
 *   a tuque FedoraObject
 */
function hook_islandora_object_alter($fedora_object) {}

/**
 * insert or remove rendered elements by implementing this function
 * in your module
 * @param type $arr
 *   an arr of rendered views
 */
function hook_islandora_display_alter($arr) {}

/**
 *
 * @param type $islandora_object
 *   a tuque FedoraObject
 * @param array $content_models
 * @param string $collection_pid
 */
function hook_islandora_ingest_pre_ingest($islandora_object, $content_models, $collection_pid) {}

