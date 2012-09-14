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
    '#content' => 'content')
  );

  return $tabset;
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
