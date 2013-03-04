<?php

/**
 * @file
 *   This file defines the hooks that fedora_repository (islandora)
 *   makes available.
 */

/**
 * Implements hook_islandora_tabs().
 * This hook lets one add tabs to the object page in Islandora.
 *
 * @param array $content_models
 *   An Array of content model objects.  A content model is only included
 *   if the object actualy exists with a ISLANDORACM datastream.
 * @param string $pid
 *   The Fedora PID of the object who's page is firing the hook.
 * @param int $page_number
 *   Page number for collection views.
 *
 * @return array
 *   $tabset a tab definition.
 */
function hook_islandora_tabs($content_models, $pid, $page_number) {

  $tabset['A TAB'] = array(
    '#type' => 'tabpage',
    '#title' => t('A TITLE'),
    '#content' => 'content'
  );

  return $tabset;
}

/**
 * Implements hook_islandora_tabs_alter().
 * This fires after the tabs have been populated.
 *
 * @param array $tabs
 *   The tab set to be altered.
 * @param array $params
 *   $params = array(
 *     'content_models' => $content_models,
 *     'pid' => $pid,
 *     'page' => $page_number,
 *   );
 */
function hook_islandora_tabs_alter($tabs, $params) {
  $tabs['fedora_object_details']['altered']['#value'] = '<H5>ALTERED</H5>';
}

/**
 * Implements hook_postprocess_solution_pack().
 * This hook fires after the batch job to ingest a solution pack finishes.
 *
 * @param string $module
 *   Name of the module that spcified the solution pack.
 */
function hook_fedora_repository_postprocess_solution_pack($module) {

  // Do something that requires the objects to be ingested ie. add XACML.
  return;

}

/**
 * Implements hook_required_fedora_objects().
 * This hook lets one add objects to the repository through the
 * solution pack interface.
 *
 * @return array
 *   array( 'path-to-foxml-file', 'pid', 'dsid', 'path-to-datastream-file',
 *   int dsversion, boolean required)
 */
function hook_required_fedora_objects() {
  return array(
    'fedora_repository' => array(
      'module' => 'fedora_repository',
      'title' => 'Islandora Core',
      'objects' => array(
        array(
          'pid' => 'islandora:collectionCModel',
          'label' => 'Islandora Collection Content Model',
          'dsid' => 'ISLANDORACM',
          'datastream_file' => "./$module_path/content_models/COLLECTIONCM.xml",
          'dsversion' => 2,
          'cmodel' => 'fedora-system:ContentModel-3.0',
        ),
        array(
          'pid' => 'islandora:root',
          'label' => 'Islandora Top-level Collection',
          'cmodel' => 'islandora:collectionCModel',
          'datastreams' => array(
            array(
              'dsid' => 'COLLECTION_POLICY',
              'datastream_file' => "./$module_path/collection_policies/COLLECTION-COLLECTION POLICY.xml",
            ),
            array(
              'dsid' => 'TN',
              'datastream_file' => "./$module_path/images/Gnome-emblem-photos.png",
              'mimetype' => 'image/png',
            ),
          ),
        ),
      ),
    ),
  );
}

/**
 * Implements hook_fedora_repository_can_ingest().
 * Override ingest permissions.
 * (from islandora_workflow)
 *
 * @deprecated
 *   Deprecated in favour of hook_fedora_repository_check_perm().
 * @param string $collection_pid
 *   The PID of the collection
 *
 * @return boolean
 *   TRUE if the user can ingest into the specified collection, FALSE otherwise.
 */
function hook_fedora_repository_can_ingest($collection_pid) {

  module_load_include('inc', 'islandora_workflow', 'islandora_workflow.permissions');
  return (islandora_workflow_user_collection_permission_check($collection_pid) !== FALSE);

}

/**
 * Implements hook_fedora_repository_check_perm().
 *
 * Hook to allow other modules to allow or deny operations on conditions other
 * than the explicit Drupal permissions.
 *
 * @param string $op
 *   A string representing the operation to be performed.
 * @param string|null $pid
 *   A string containing the Fedora PID on which the operation is to be
 *   performed. The (default) value of NULL will use the PID indicated by the
 *   fedora_repository_pid Drupal variable.
 * @param object|null $as_user
 *   An object representing the user for whom to check the permissions (as
 *   given by user_load or the $user global).  The (default) value of NULL will
 *   cause permissions to be evaluated for the current user (from the $user
 *   global).
 *
 * @return boolean|null
 *   Either a boolean permitting (TRUE) or forbidding (FALSE) an operation, or
 *   NULL to make no assertion.
 */
function hook_fedora_repository_check_perm($op, $pid = NULL, $as_user = NULL) {
  return NULL;
}
