<?php

/**
 * @file
 * Post updates.
 */

/**
 * Set default value for delete_media_and_files field in settings.
 */
function islandora_post_update_delete_media_and_files() {
  $config_factory = \Drupal::configFactory();
  $config = $config_factory->getEditable('islandora.settings');
  $config->set('delete_media_and_files', TRUE);
  $config->save(TRUE);
}

/**
 * Ensure `fast_term_queries` exists.
 */
function islandora_post_update_fast_term_queries() : void {
  $config_factory = \Drupal::configFactory();
  $config = $config_factory->getEditable('islandora.settings');
  $config->set('fast_term_queries', TRUE);
  $config->save(TRUE);
}

/**
 * Ensure `allow_header_links` exists.
 */
function islandora_post_update_allow_header_links() : void {
  $config_factory = \Drupal::configFactory();
  $config = $config_factory->getEditable('islandora.settings');
  $config->set('allow_header_links', TRUE);
  $config->save(TRUE);
}
