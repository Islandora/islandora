<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Implements hook_preprocess_theme().
 */
function islandora_preprocess_islandora_default_edit(array &$variables) {
  global $base_url;
  $islandora_object = $variables['islandora_object'];
  $datastreams = array();
  $variables['islandora_editmetadata_url'] = $base_url . '/islandora/edit_form/' . $islandora_object->id;
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $header = array(
    array('data' => t('ID')),
    array('data' => t('Label')),
    array('data' => t('Type')),
    array('data' => t('Mime type')),
    array('data' => t('Size')),
    array('data' => t('Operations'), 'colspan' => '3'),
  );
  $table_attributes = array('class' => array('manage-datastreams'));
  $rows = array();
  foreach ($islandora_object as $ds) {
    $rows[] = array(
      array(
        'class' => 'datastream-id',
        'data' => l($ds->id, islandora_datastream_get_url($ds, 'view')),
      ),
      array(
        'class' => 'datastream-label',
        'data' => $ds->label,
      ),
      array(
        'class' => 'datastream-control',
        'data' => islandora_control_group_to_human_readable($ds->controlGroup),
      ),
      array(
        'class' => 'datastream-mime',
        'data' => $ds->mimeType,
      ),
      array(
        'class' => 'datastream-size',
        'data' => islandora_datastream_get_human_readable_size($ds),
      ),
      array(
        'class' => 'datastream-download',
        'data' => l(t('download'), islandora_datastream_get_url($ds, 'download')),
      ),
      array(
        'class' => 'datstream-edit',
        'data' => islandora_datastream_edit_get_link($ds),
      ),
      array(
        'class' => 'datastream-delete',
        'data' => islandora_datastream_get_delete_link($ds),
      ),
    );
  }
  $caption = $islandora_object->label . ' - ' . $islandora_object->id;
  $table = array(
    'colgroups' => NULL,
    'sticky' => TRUE,
    'empty' => 'Error loading datastreams',
    'caption' => $caption,
    'header' => $header,
    'rows' => $rows,
    'attributes' => $table_attributes,
  );
  $variables['datastream_table'] = $table;
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_preprocess_islandora_default(&$variables) {
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  $islandora_object = $variables['islandora_object'];
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/datastream');

  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);

  $datastreams = array();
  foreach ($islandora_object as $ds) {
    try {
      $pid = $islandora_object->id;
      $id = $ds->id;
      $label = $ds->label;
      $download_path = islandora_datastream_get_url($ds, 'download');
      $datastreams[$id]['id'] = $id;
      $datastreams[$id]['label'] = $label;
      $datastreams[$id]['label_link'] = l($label, $download_path);
      $datastreams[$id]['download_url'] = $download_path;
      $datastreams[$id]['mimetype'] = $ds->mimetype;
      $datastreams[$id]['size'] = islandora_datastream_get_human_readable_size($ds);
      $datastreams[$id]['created_date'] = $ds->createdDate->format("Y-m-d");
      $datastreams[$id]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $id));
    }
    catch (RepositoryException $e) {

    }
  }
  $variables['datastreams'] = $datastreams;
  try {
    $dc = $islandora_object['DC']->content;
    $dc_object = DublinCore::importFromXMLString($dc);
    $dc_array = $dc_object->asArray();
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $variables['dc_array'] = isset($dc_array) ? $dc_array : array();
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
  $variables['islandora_object_label'] = $islandora_object->label;
  global $base_url;
  if (isset($islandora_object['TN'])) {
    $variables['islandora_thumbnail_url'] = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/TN/view';
  }
}
