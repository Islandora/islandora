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
