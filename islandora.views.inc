<?php

/**
 * @file
 * Provide Views data for integer-weight fields.
 */

/**
 * Implements hook_views_data_alter().
 */
function islandora_views_data_alter(&$data) {
  // For now only support Nodes.
  $fields = \Drupal::service('entity_field.manager')->getFieldStorageDefinitions('node');
  foreach ($fields as $field => $field_storage_definition) {
    if ($field_storage_definition->getType() == 'integer' && strpos($field, "field_") === 0) {
      $prefixed_field = 'node__' . $field;
      if (isset($data[$prefixed_field])) {
        $data[$prefixed_field][$field . '_value']['field'] = $data[$prefixed_field][$field]['field'];
        $data[$prefixed_field][$field]['title'] = t('Integer Weight Selector (@field)', [
          '@field' => $field,
        ]);
        $data[$prefixed_field][$field]['help'] = t('Provides a drag-n-drop reordering of integer-based weight fields.');
        $data[$prefixed_field][$field]['title short'] = t('Integer weight selector');
        $data[$prefixed_field][$field]['field']['id'] = 'integer_weight_selector';
      }
    }
  }
}
