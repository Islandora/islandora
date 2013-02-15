<?php

/**
 * @file
 * Ingest Menu callback hooks.
 */

/**
 * Menu callback, Renders the multi-page ingest form if possible.
 *
 * @return string
 *   HTML representing the mult-page ingest form.
 */
function islandora_ingest_callback() {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  try {
    $configuration = islandora_ingest_get_configuration();
    return drupal_get_form('islandora_ingest_form', $configuration);
  }
  catch(Exception $e) {
    $redirect = isset($_SERVER['HTTP_REFERER']) ? $_SERVER['HTTP_REFERER'] : '<front>';
    // Redirect back to referer or top level collection.
    drupal_goto($redirect);
  }
}

/**
 * Fetches the ingest configuration from the $_GET parameters.
 *
 * Generic parameters as accepted by all ingest processes, other modules may
 * add to this list.
 *   id -> The pid of the object to create. optional.
 *   models -> Comma delimited list of all the content models the created object
 *             should have.
 *   collections -> Comma delimited list of all the collections the created
 *                  object should belong to.
 *
 * @return array
 *   The configuration options used to build the multi-paged ingest process.
 */
function islandora_ingest_get_configuration() {
  $configuration = $_GET;
  unset($configuration['q']);
  $convert_to_array_keys = array_intersect(array('models', 'collections'), array_keys($configuration));
  foreach ($convert_to_array_keys as $key) {
    $configuration[$key] = explode(',', $configuration[$key]);
  }
  return $configuration;
}
