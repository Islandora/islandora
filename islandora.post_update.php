<?php

/**
 * @file
 * Post updates.
 */

use Islandora\Chullo\IFedoraApi;
use League\Flysystem\FilesystemAdapter;
use Islandora\EntityMapper\EntityMapper;
use Drupal\Core\Site\Settings;
use Drupal\Core\Utility\UpdateException;

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

/**
 * Enable optional Fedora storage only when existing files use it.
 */
function islandora_post_update_enable_fcrepo() {
  if (\Drupal::moduleHandler()->moduleExists('islandora_fcrepo')) {
    return;
  }
  $database = \Drupal::database();
  if (!$database->schema()->tableExists('file_managed')) {
    return;
  }
  // Include the historical default and any configured Fedora scheme aliases.
  $schemes = ['fcrepo', 'fedora'];
  foreach (Settings::get('flysystem', []) as $scheme => $settings) {
    if (($settings['driver'] ?? NULL) === 'fedora') {
      $schemes[] = $scheme;
    }
  }
  $query = $database->select('file_managed', 'f')->fields('f', ['fid']);
  $condition = $query->orConditionGroup();
  foreach (array_unique($schemes) as $scheme) {
    $condition->condition('uri', $database->escapeLike($scheme . '://') . '%', 'LIKE');
  }
  if (!$query->condition($condition)->range(0, 1)->execute()->fetchField()) {
    return;
  }
  $modules = \Drupal::service('extension.list.module')->getList();
  $flysystem_available = isset($modules['flysystem']) && file_exists($modules['flysystem']->getPath() . '/src/Adapter/AdapterDriverInterface.php');
  if (!$flysystem_available || !interface_exists(FilesystemAdapter::class) || !interface_exists(IFedoraApi::class) || !class_exists(EntityMapper::class)) {
    throw new UpdateException('Fedora files require Islandora Fedora Repository. Run composer require "drupal/flysystem:^3.0@beta" "league/flysystem:^3.35.3" "islandora/chullo:^2.0" "islandora/fedora-entity-mapper:^1.0" -W, then rerun database updates.');
  }
  \Drupal::service('module_installer')->install(['islandora_fcrepo']);
}
