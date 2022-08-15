<?php

/**
 * @file
 * Provide Views data for integer-weight fields.
 */

/**
 * Implements hook_views_data_alter().
 */
function islandora_views_data_alter(&$data) {
  // For now only support Nodes and Media.
  foreach (['node', 'media'] as $entity_type) {
    $fields = \Drupal::service('entity_field.manager')->getFieldStorageDefinitions($entity_type);
    foreach ($fields as $field => $field_storage_definition) {
      if ($field_storage_definition->getType() == 'integer' && strpos($field, "field_") === 0) {
        $prefixed_field = $entity_type . '__' . $field;
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

  // Add Has Media filter.
  $data['node_field_data']['islandora_has_media'] = [
    'title' => t('Node has Media Use'),
    'group' => t('Content'),
    'filter' => [
      'title' => t('Node has media use filter'),
      'help' => t('Provides a custom filter for nodes that do or do not have media with a given use.'),
      'field' => 'nid',
      'id' => 'islandora_node_has_media_use',
    ],
  ];

  // Add Is Islandora filter.
  $data['node_field_data']['islandora_node_is_islandora'] = [
    'title' => t('Node is Islandora'),
    'group' => t('Content'),
    'filter' => [
      'title' => t('Node is Islandora'),
      'help' => t('Node has a content type that possesses the mandatory Islandora fields.'),
      'field' => 'nid',
      'id' => 'islandora_node_is_islandora',
    ],
  ];
}
