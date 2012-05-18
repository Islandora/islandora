<?php

/**
 * @file
 * Menu callback functionality for islandora/ingest
 */

/**
 * Callback function for islandora/ingest/%collection_pid
 * @TODO: validate?: pid, registry return
 * @param string $pid
 */
function islandora_ingest_callback($collection_pid) {
  $ingest_registry = module_invoke_all('islandora_ingest_registry', $collection_pid);
  
  $registry_count = count($ingest_registry);
  
  if ($registry_count == 0) {
    // No ingest implementations.
    drupal_set_message(t('There are no ingest methods specified for this collection.'));
    drupal_goto('islandora/object/' . $collection_pid);
  }
  elseif ($registry_count == 1) {
    // One registry implementation, go there
    drupal_goto($ingest_registry[0]['url']);
  }
  else {
    // Multiple ingest routes registered
    return islandora_ingest_registry_render($ingest_registry);
  }
}

//@TODO: theme
function islandora_ingest_registry_render($ingest_registry) {
  $output = array(
    '#type' => 'markup',
    '#markup' => '',
  );
  foreach ($ingest_registry AS $ingest_route) {
    $output['#markup'] .= l($ingest_route['name'], $ingest_route['url']) . '<br/>';
  }
  return $output;
}