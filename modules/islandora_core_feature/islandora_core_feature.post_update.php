<?php

/**
 * @file
 * Post-update hooks.
 */

/**
 * Add index to field_weight.
 */
function islandora_core_feature_post_update_add_index_to_field_weight() {
  $storage = \Drupal::entityTypeManager()->getStorage('field_storage_config');
  $field = $storage->load('node.field_weight');
  $indexes = $field->getIndexes();
  $indexes += [
    'value' => ['value'],
  ];
  $field->setIndexes($indexes);
  $field->save();
}
