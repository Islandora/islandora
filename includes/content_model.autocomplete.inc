<?php
/**
 * @file
 * Autocomplete functionality for content models in Islandora.
 */

/**
 * Autocomplete the content model name.
 *
 * @param string $string
 *   A search string.
 *
 * @return string
 *   The rendered JSON results.
 */
function islandora_content_model_autocomplete($string) {
  $content_models = islandora_get_content_model_names();
  $output = array();
  foreach ($content_models as $model => $label) {
    if (preg_match("/{$string}/i", $label) !== 0) {
      $output[$model] = $label;
    }
  }
  return drupal_json_output($output);
}

/**
 * Gets a map of form names suitable for use as select #options.
 */
function islandora_get_content_model_names() {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $results = islandora_get_content_models();
  $ret = array();
  foreach ($results as $result) {
    $ret[$result['pid']] = "{$result['label']} ({$result['pid']})";
  }
  return $ret;
}

/**
 * Minor array transformation.
 *
 * @param array $content
 *   The array of results as returned from Tuque's RI query interface.
 *
 * @return array
 *   An array of results in a more usable format.
 */
function islandora_parse_query($content) {
  $content_models = array();
  foreach ($content as $model) {
    $content_models[] = $model['object']['value'];
  }
  $content_models = array_unique($content_models);
  $content_models = array_values($content_models);
  return $content_models;
}
