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
  $results = islandora_query_content_models();
  $ret = array();
  foreach ($results as $result) {
    $ret[$result['model']['value']] = "{$result['label']['value']} ({$result['model']['value']})";
  }
  return $ret;
}

/**
 * Perform a resource index query to determine get a list of content models.
 *
 * Only returns content models with at least one subscribing object.
 *
 * @return array
 *   An array of RI results, as given by the Tuque RI query interface.
 */
function islandora_query_content_models() {
  $connection = islandora_get_tuque_connection();
  if ($connection) {
    $query = 'select $model $label from <#ri> where
            $model <fedora-model:hasModel> <info:fedora/fedora-system:ContentModel-3.0> and $model <fedora-model:label> $label
            minus $model <mulgara:is><info:fedora/fedora-system:FedoraObject-3.0>
            minus $model <mulgara:is><info:fedora/fedora-system:ContentModel-3.0>
            minus $model <mulgara:is><info:fedora/fedora-system:ServiceDefinition-3.0>
            minus $model <mulgara:is><info:fedora/fedora-system:ServiceDeployment-3.0>';
    $results = $connection->repository->ri->itqlQuery($query);
    return $results;
  }
  return array();
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
