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
  // Objects in fcrepo4 don't always contain a DC datastream.
  if (isset($islandora_object['DC'])) {
    $dc_object = DublinCore::importFromXMLString($islandora_object['DC']->content);
    $dc_array = $dc_object->asArray();
  }
  $variables['dc_array'] = isset($dc_array) ? $dc_array : array();
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
  $variables['islandora_object_label'] = $islandora_object->label;
  if (isset($islandora_object['TN'])) {
    $variables['islandora_thumbnail_url'] = url("islandora/object/{$islandora_object->id}/datastream/TN/view");
  }
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_preprocess_islandora_object_print(array &$variables) {
  // Apply the print CSS in non print context.
  $only_print_media = function($o) {
    return $o['media'] == 'print';
  };
  $css = array_filter(drupal_add_css(), $only_print_media);
  foreach ($css as $data => $options) {
    $options['media'] = 'all';
    drupal_add_css($data, $options);
  }
  // Allow modules to define their own theme suggestions for the given object.
  // Suggestions are defined as islandora_object_print__CMODEL__PID. For
  // example for the image object islandora:1234.
  // islandora_object_print__islandora_imagecmodel
  // islandora_object_print__islandora_imagecmodel__islandora_1234
  // would be valid theme suggestions. This step up does not support objects
  // with multiple content models in which each content model provides a theme
  // suggestion.
  $object = $variables['object'];
  $pid = strtolower(preg_replace('/[^a-zA-Z0-9_]/', '_', $object->id));
  $models = array_diff($object->models, array('fedora-system:FedoraObject-3.0'));
  foreach ($models as $model) {
    $model = strtolower(preg_replace('/[^a-zA-Z0-9_]/', '_', $model));
    $suggestions = theme_get_suggestions(array($model, $pid), 'islandora_object_print');
    $variables['theme_hook_suggestions'] = array_merge($variables['theme_hook_suggestions'], $suggestions);
  }
}

/**
 * Implements theme_hook().
 */
function theme_islandora_object_print(array &$variables) {
  return drupal_render($variables['content']);
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_preprocess_islandora_objects(array &$variables) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $display = (empty($_GET['display'])) ? 'grid' : $_GET['display'];
  $grid_display = $display == 'grid';
  $list_display = !$grid_display;
  $query_params = drupal_get_query_parameters($_GET);
  $variables['display_links'] = array(
    array(
      'title' => t('Grid view'),
      'href' => url($_GET['q'], array('absolute' => TRUE)),
      'attributes' => array(
        'class' => $grid_display ? 'active' : '',
      ),
      'query' => array('display' => 'grid') + $query_params,
    ),
    array(
      'title' => t('List view'),
      'href' => url($_GET['q'], array('absolute' => TRUE)),
      'attributes' => array(
        'class' => $list_display ? 'active' : '',
      ),
      'query' => array('display' => 'list') + $query_params,
    ),
  );
  // Pager.
  $objects = $variables['objects'];
  $limit = $variables['limit'];
  $page_size = $variables['page_size'];
  $page = pager_default_initialize(count($objects), $limit);
  $objects = array_slice($objects, $page * $limit, $limit);
  $variables['pager'] = theme('pager', array('quantity' => 10));
  // Content.
  $map_objects = function($o) {
    $o = islandora_object_load($o);
    $url = "islandora/object/{$o->id}";
    $link_options = array('html' => TRUE, 'attributes' => array('title' => $o->label));
    $img = theme_image(array('path' => url("$url/datastream/TN/view"), 'attributes' => array()));
    $description = NULL;
    $dc = DublinCore::importFromXMLString($o['DC']->content);
    if ($dc) {
      $dc = $dc->asArray();
      $description = $dc['dc:description']['value'];
    }
    return array(
      'label' => $o->label,
      'class' => drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $o->id)),
      'link' => l($o->label, $url, $link_options),
      'thumb' => l($img, $url, $link_options),
      'description' => $description,
    );
  };
  $objects = array_map($map_objects, $objects);
  $theme = $grid_display ? 'islandora_objects_grid' : 'islandora_objects_list';
  $variables['content'] = theme($theme, array('objects' => $objects));
  $module_path = drupal_get_path('module', 'islandora');
  drupal_add_css("$module_path/css/islandora.objects.css");
}
