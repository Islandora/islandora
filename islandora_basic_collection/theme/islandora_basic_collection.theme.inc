<?php

/**
 * list view preprocess
 * currently just calls normal view preprocess as variables are the same
 * @param type $variables 
 */
function islandora_basic_collection_preprocess_islandora_basic_collection_grid(&$variables) {
  islandora_basic_collection_preprocess_islandora_basic_collection($variables);
}

function islandora_basic_collection_preprocess_islandora_basic_collection_wrapper(&$variables) {
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $page_size = (empty($_GET['pagesize'])) ? variable_get('islandora_basic_collection_page_size', '10') : $_GET['pagesize'];
  $islandora_object = $variables['islandora_object'];
  $results = islandora_basic_collection_get_objects($islandora_object, $page_number, $page_size);
  $total_count = count($results);
  pager_default_initialize($total_count, $page_size);
  $variables['collection_pager'] = theme('pager', array('quantity' => 10));
  $display = (empty($_GET['display'])) ? variable_get('islandora_basic_collection_default_view', 'grid') : $_GET['display'];
  $link_text = (empty($_GET['display'])) ? 'grid' : $_GET['display'];
  $query_params = drupal_get_query_parameters($_GET);
  global $base_url;
  if ($display == 'grid') {
    $query_params['display'] = 'list';
     $list_link = array('title' => 'List view', 'href' => $base_url . '/islandora/object/' . $islandora_object->id, 'attributes' => array('class' => 'islandora-view-list'),
    'query' => $query_params);
     unset($query_params['display']);
     $query_params['display'] = 'grid';
    $grid_link = array('title' => 'Grid view', 'href' => $base_url . '/islandora/object/' . $islandora_object->id, 'attributes' => array('class' => array('islandora-view-grid', 'active')),
    'query' => $query_params);
    $collection_content = theme('islandora_basic_collection_grid', array('islandora_object' => $islandora_object, 'collection_results' => $results));
  }
  else {
    $query_params['display'] = 'list';
    $list_link = array('title' => 'List view', 'href' => $base_url . '/islandora/object/' . $islandora_object->id, 'attributes' => array('class' => array('islandora-view-list', 'active')),
    'query' => $query_params);
    unset($query_params['display']);
    $query_params['display'] = 'grid';
    $grid_link = array('title' => 'Grid view', 'href' => $base_url . '/islandora/object/' . $islandora_object->id, 'attributes' => array('class' => 'islandora-view-grid'),
    'query' => $query_params);
    $collection_content = theme('islandora_basic_collection', array('islandora_object' => $islandora_object, 'collection_results' => $results));
  }
  $variables['view_links'] = array($grid_link,$list_link);
  //$variables['grid_link'] = $grid_link;
  $variables['collection_content'] = $collection_content;
}

/**
 * 
 * @global type $base_url
 * @param array $variables
 *   an array of variables that will be passed to the theme function
 */
function islandora_basic_collection_preprocess_islandora_basic_collection(&$variables) {
// base url
  global $base_url;
// base path
  global $base_path;
  $islandora_object = $variables['islandora_object'];
  module_load_include('inc', 'islandora', 'includes/islandora_dublin_core');
  try {
    $dc = $islandora_object['DC']->content;
    $dc_object = Dublin_Core::import_from_xml_string($dc);
  } catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $page_size = (empty($_GET['pagesize'])) ? variable_get('islandora_basic_collection_page_size', '10') : $_GET['pagesize'];
  $results = $variables['collection_results']; //islandora_basic_collection_get_objects($islandora_object, $page_number, $page_size);
  $total_count = count($results);
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : array();
  $variables['islandora_object_label'] = $islandora_object->label;
  $display = (empty($_GET['display'])) ? 'list' : $_GET['display'];
  if ($display == 'grid') {
    $variables['theme_hook_suggestions'][] = 'islandora_basic_collection_grid__' . str_replace(':', '_', $islandora_object->id);
  }
  else {
    $variables['theme_hook_suggestions'][] = 'islandora_basic_collection__' . str_replace(':', '_', $islandora_object->id);
  }
  if (isset($islandora_object['OBJ'])) {
    $full_size_url = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/OBJ/view';
    $variables['islandora_full_img'] = '<img src="' . $full_size_url . '"/>';
  }
  if (isset($islandora_object['TN'])) {
    $thumbnail_size_url = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/TN/view';
    $variables['islandora_thumbnail_img'] = '<img src="' . $thumbnail_size_url . '"/>';
  }
  if (isset($islandora_object['MEDIUM_SIZE'])) {
    $medium_size_url = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/MEDIUM_SIZE/view';
    $variables['islandora_medium_img'] = '<img src="' . $medium_size_url . '"/>';
  }

  $associated_objects_array = array();
  $start = $page_size * ($page_number);
  $end = min($start + $page_size, $total_count);

  for ($i = $start; $i < $end; $i++) {
    $pid = $results[$i]['object']['value'];
    $fc_object = islandora_basic_collection_get_object($pid);
    if (!isset($fc_object)) {
      continue; //null object so don't show in collection view;
    }
    $associated_objects_array[$pid]['object'] = $fc_object;
    try {
      $dc = $fc_object['DC']->content;
      $dc_object = Dublin_Core::import_from_xml_string($dc);
      $associated_objects_array[$pid]['dc_array'] = $dc_object->as_formatted_array();
    } catch (Exception $e) {
      drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
    $object_url = 'islandora/object/' . $pid;
    $thumbnail_img = '<img src="' . $base_path . $object_url . '/datastream/TN/view"' . '/>';
    $title = $results[$i]['title']['value'];
    $associated_objects_array[$pid]['pid'] = $pid;
    $associated_objects_array[$pid]['path'] = $object_url;
    $associated_objects_array[$pid]['title'] = $title;
    $associated_objects_array[$pid]['class'] = strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $pid));
    if (isset($fc_object['TN'])) {
      $thumbnail_img = '<img src="' . $base_path . $object_url . '/datastream/TN/view"' . '/>';
    }
    else {
      $image_path = drupal_get_path('module', 'islandora');
      $thumbnail_img = '<img src="' . $base_path . $image_path . '/images/Crystal_Clear_action_filenew.png"/>';
    }
    $associated_objects_array[$pid]['thumbnail'] = $thumbnail_img;
    $associated_objects_array[$pid]['title_link'] = l($title, $object_url, array('html' => TRUE, 'attributes' => array('title' => $title)));
    $associated_objects_array[$pid]['thumb_link'] = l($thumbnail_img, $object_url, array('html' => TRUE, 'attributes' => array('title' => $title)));
  }
  $variables['associated_objects_array'] = $associated_objects_array;
}

?>
