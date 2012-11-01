<?php

/**
 * @file
 * This file contains the admin form and callback functions for datastream manipulations.
 */

define('DS_COMP_STREAM', 'DS-COMPOSITE-MODEL');

/**
 * Callback function to view or download a datastream.
 *
 * @global object $user
 * @param string $object_id
 * @param string $dsid
 * @return stream
 *  prints datastream to browser
 */
function islandora_view_datastream($object, $dsid, $method = 'view') {
  // if the object exists but the datastream doesn't
  if (!isset($object[$dsid])) {
    return drupal_not_found();
  }
  header_remove('Cache-Control');
  header_remove('Expires');
  header('Content-type: ' . $object[$dsid]->mimetype);
  if ($object[$dsid]->controlGroup == 'M' || $object[$dsid]->controlGroup == 'X') {
    header('Content-length: ' . $object[$dsid]->size);
  }

  if ($method == 'download') {
    header("Content-Disposition: attachment; filename=\"" . $object[$dsid]->label);
  }

  // Disable page caching
  drupal_page_is_cacheable(FALSE);

  // Try not to load the file into PHP memory
  $file = drupal_tempnam(file_directory_temp(), 'islandora');
  $object[$dsid]->getContent($file);
  readfile($file);
  drupal_unlink($file);
  exit();
}

/**
 * For a given object, return all parent collections.
 */
function islandora_datastream_get_parents($islandora_object) {
  $parent_collections = array();

  try {
    $repository = $islandora_object->repository;
    $collections1 = $islandora_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
    $collections2 = $islandora_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  }
  catch (RepositoryException $e) {
    $collections1 = array();
    $collections2 = array();
  }
  $collections = array_merge($collections1, $collections2);

  foreach ($collections as $collection) {
    try {
      $pid = $collection['object']['value'];
      $object = $repository->getObject($collection['object']['value']);
      $parent_collections[$pid] = array();
      $parent_collections[$pid]['object'] = $object;
      $parent_collections[$pid]['url'] = 'islandora/object/' . $object->id;
      $parent_collections[$pid]['label'] = $object->label;
      $parent_collections[$pid]['label_link'] = l($parent_collections[$pid]['label'], $parent_collections[$pid]['url']);
    }
    catch (RepositoryException $e) {

    }
  }

  return $parent_collections;
}

/**
 *
 * @param array $arr
 *   an array of dsids that are defined by this objects cmodels
 * @param string $ds_comp_stream
 *   the dscomposite stream as xml
 */
function islandora_get_defined_dsids_array(&$arr, $ds_comp_stream) {
  $sxml = new SimpleXMLElement($ds_comp_stream);
  foreach ($sxml->dsTypeModel as $ds) {
    //$arr[$ds['ID']]
    $mimes = array();
    foreach ($ds->form as $form) {
      $mimetype = (string) $form['MIME'];
      $mimes[] = $mimetype;
    }
    $dsid = (string) $ds['ID'];
    if ($dsid != 'AUDIT') {
      $arr[(string) $ds['ID']] = $mimes;
    }
  }
}

/**
 *
 * @global type $user
 * @param string $object_id
 * @return string|array
 */
function islandora_get_unused_dsids($object) {

  $defined_dsids = array();

  if (!$object) {
    return $defined_dsids;
  }

  $models = $object->models;
  if (isset($models)) {
    foreach ($models as $model) {
      try {
        $model_object = $object->repository->getObject($model);
        if (isset($model_object[DS_COMP_STREAM])) {
          $dscomposite_stream = $model_object[DS_COMP_STREAM]->content;
          islandora_get_defined_dsids_array($defined_dsids, $dscomposite_stream);
        }
      } catch (Exception $e) {
        //do nothing as other objects may have a dscompsite stream
      }
    }
  }
  foreach ($defined_dsids as $key => $value) {
    if (isset($object[$key])) {
      unset($defined_dsids[$key]); //ds exists in the object so don't show in the dropdown
    }
  }
  return $defined_dsids;
}

/**
 * buids the default add datastream form
 * @param string $object_id
 * @param array $form_state
 * @return array
 *  a form ready to be rendered with a call to Drupal render
 */
function islandora_get_add_datastream_form($object, &$form_state) {
  $unused_dsids = islandora_get_unused_dsids($object);
  $form = array();
  $form['add_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => 'Add a datastream',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['add_fieldset']['add_datastream_label'] = array(
    '#value' => t('<br /><h3>Add Datastream:</h3>'),
    '#weight' => -10,
  );

  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => "$object->id"
  );

  $form['add_fieldset']['stream_label'] = array(
    '#title' => 'Datastream Label',
    '#required' => 'TRUE',
    '#description' => t('A Human readable label'),
    '#type' => 'textfield'
  );

  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['add_fieldset']['add-stream-file-location'] = array(
    '#type' => 'file',
    '#title' => t('Upload Document'),
    '#size' => 48,
    // '#required'=>'TRUE',
    '#description' => t('The file to upload.')
  );
  $form['#redirect'] = "islandora/object/$object->id/";
  $form['add_fieldset']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Datastream')
  );

  if (!empty($unused_dsids)) {
    $dsids_for_form = array();
    foreach ($unused_dsids as $key => $value) {
      $dsids_for_form[$key] = $key;
    }
    $form['add_fieldset']['stream_id'] = array(
      '#type' => 'select',
      '#title' => t('Datastream ID'),
      '#default_value' => variable_get('feed_item_length', 'teaser'),
      '#weight' => '-1',
      '#description' => t('Datastream IDs defined by the content model.'),
    );
    $form['add_fieldset']['stream_id']['#options'] = $dsids_for_form;
  }
  else {
    $form['add_fieldset']['stream_id'] = array(
      '#title' => 'Datastream ID',
      '#required' => 'TRUE',
      '#description' => t('An ID for this stream that is unique to this object. Must start with a letter and contain only alphanumeric characters and dashes and underscores.'),
      '#type' => 'textfield',
      '#weight' => -1,
    );
  }
  return $form;
}

/**
 * Default implmentation currently only does M (managed datastreams)
 * other modules can hook form alter to add other functionality
 * @global string $base_url
 * @global object $user
 *   Drupal user
 * @param array $form
 * @param array $form_state
 * @return type
 */
function islandora_add_datastream_form_submit($form, &$form_state) {
  global $base_url;
  if (!empty($form_state['submit']) && $form_state['submit'] == 'OK') {
    $form_state['rebuild'] = TRUE;
    return;
  }

  module_load_include('inc', 'islandora', 'includes/mime.detect');
  $mimetype = new MimeDetect();

  $file = $form_state['values']['add-stream-file-location'];
  $file = drupal_realpath($file);

  $object_id = $form_state['values']['pid'];
  $dsid = $form_state['values']['stream_id'];
  $ds_label = $form_state['values']['stream_label'];
  $dformat = $mimetype->getMimeType($file);
  $control_group = "M";

  try {
    $fedora_object = islandora_object_load($object_id);
    $ds = $fedora_object->constructDatastream($dsid, $control_group);
    $ds->label = $ds_label;
    $ds->mimetype = $dformat;
    $ds->setContentFromFile($file);
    $fedora_object->ingestDatastream($ds);
    $d_file = file_load($form_state['values']['fid']);
    file_delete($d_file);
  } catch (exception $e) {
    drupal_set_message(t('@message', array('@message' => check_plain($e->getMessage()))), 'error');
    return;
  }
  drupal_set_message(t("Successfully Added Datastream!"));
  drupal_goto("islandora/object/$object_id");
}

/**
 * validates this datastream id against its allowed mimetypes in the dscomposite
 * of its content models.
 * @param array $form
 * @param array $form_state
 * @return boolean
 */
function islandora_add_datastream_form_validate($form, &$form_state) {
  module_load_include('inc', 'islandora', 'includes/mime.detect');
  $mimetype = new MimeDetect();
  if ($form_state['clicked_button']['#value'] == 'OK') {
    $form_state['rebuild'] = TRUE;
    return;
  }
  $dsid = $form_state['values']['stream_id'];
  $ds_label = $form_state['values']['stream_label'];
  if (drupal_strlen($dsid) > 64) {
    form_set_error('', t('Data stream ID cannot be more than 64 characters.'));
    return FALSE;
  }
  if (!(preg_match("/^[a-zA-Z]/", $dsid))) {
    form_set_error('', t("Data stream ID (@dsid) has to start with a letter.", array('@dsid' => check_plain($dsid))));
    return FALSE;
  }
  if (drupal_strlen($ds_label) > 64) {
    form_set_error('', t('Data stream Label cannot be more than 64 characters.'));
    return FALSE;
  }
  if (strpos($ds_label, '/')) {
    form_set_error('', t('Data stream Label cannot contain a "/".'));
    return FALSE;
  }

  $object_id = $form_state['values']['pid'];
  $fedora_object = islandora_object_load($object_id);

  if (isset($fedora_object[$dsid])) {
    form_set_error('', t('Data stream ID already exists in object.'));
    return FALSE;
  }

  $mimetype = new MimeDetect();
  $object = islandora_object_load($form_state['values']['pid']);
  $unused_dsids = islandora_get_unused_dsids($object);
  if (isset($unused_dsids[$dsid])) {
    $types_allowed = $unused_dsids[$dsid];
    $arr = array();
    foreach ($types_allowed as $type) {
      $arr[] = $mimetype->getExtension($type);
    }
  }
  else {
    // @TODO: this is unsafe, should probably be fixed see:
    // http://api.drupal.org/api/drupal/includes!file.inc/function/file_save_upload/7
    $arr = array();
  }


  $file = file_save_upload('add-stream-file-location', array('file_validate_extensions' => $arr));
  if ($file) {
    $form_state['values']['add-stream-file-location'] = $file->uri;
    $form_state['values']['fid'] = $file->fid; //so we can load it to delete later
  }
  else {
    form_set_error('add-stream-file-location', t('There was no file uploaded'));
  }
}


/**
 * buids the default add datastream form
 * @param string $object_id
 * @param array $form_state
 * @return array
 *  a form ready to be rendered with a call to Drupal render
 */
function islandora_add_datastream_form($form, &$form_state, $object) {
  // Ensure we're loaded, in case AJAX stuff is ever added.
  form_load_include($form_state, 'inc', 'islandora', 'includes/datastream');

  $unused_dsids = islandora_get_unused_dsids($object); //$defined_dsids;
  $form = array();
  $form['add_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => 'Add a datastream',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['add_fieldset']['add_datastream_label'] = array(
    '#value' => t('<br /><h3>Add Datastream:</h3>'),
    '#weight' => -10,
  );

  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => "$object->id"
  );

  $form['add_fieldset']['stream_label'] = array(
    '#title' => 'Datastream Label',
    '#required' => 'TRUE',
    '#description' => t('A Human readable label'),
    '#type' => 'textfield'
  );

  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['add_fieldset']['add-stream-file-location'] = array(
    '#type' => 'file',
    '#title' => t('Upload Document'),
    '#size' => 48,
    // '#required'=>'TRUE',
    '#description' => t('The file to upload.')
  );
  $form['#redirect'] = "islandora/object/$object->id/";
  $form['add_fieldset']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Datastream')
  );

  $unused_dsids = islandora_get_unused_dsids($object);
  $dsids_for_form = '';
  $i = 0;
  foreach ($unused_dsids as $key => $value) {
    if ($i++) {
      $dsids_for_form .= ", ";
    }
    $dsids_for_form .= "'$key'";
  }

  $form['add_fieldset']['stream_id'] = array(
    '#title' => 'Datastream ID',
    '#required' => 'TRUE',
    '#description' => t('An ID for this stream that is unique to this object. Must start with a letter and contain only alphanumeric characters and dashes and underscores. Datastreams that are defined by the content model don\'t currently exist: <b>@unused_dsids</b>.', array(
      '@unused_dsids' => $dsids_for_form,
    )),
    '#type' => 'textfield',
    '#weight' => -1,
    '#autocomplete_path' => "islandora/object/$object->id/manage/datastreams/add/autocomplete",
  );
  return $form;
}

function islandora_datastream_autocomplete_callback($object, $string = '') {
  $dsids = islandora_get_unused_dsids($object);
  $output = array();
  foreach ($dsids as $id => $ds) {
    if (trim($string) == '') {
      $output[$id] = $id;
    }
    else {
      $ret = stripos($id, $string);
      if ($ret !== FALSE) {
        $output[$id] = $id;
      }
    }
  }
  drupal_json_output($output);
}

function islandora_datastream_get_human_readable_size($ds) {
  module_load_include('inc', 'islandora', 'includes/utilities');

  // we return - if we don't have a size
  if ($ds->controlGroup == 'M' || $ds->controlGroup == 'X') {
    return islandora_convert_bytes_to_human_readable($ds->size);
  }
  else {
    return '-';
  }
}

function islandora_datastream_get_url($ds, $type = 'download') {
  if ($ds->controlGroup == 'R') {
    return $ds->url;
  }
  else {
    return "islandora/object/{$ds->parent->id}/datastream/{$ds->id}/$type";
  }
}

function islandora_datastream_get_delete_link($ds) {
  $datastreams = module_invoke_all('islandora_undeletable_datastreams', $ds->parent->models);

  if (in_array($ds->id, $datastreams)) {
    return '';
  }
  else {
    return l(t('delete'), 'islandora/object/' . $ds->parent->id . '/datastream/' . $ds->id . '/delete');
  }
}

function islandora_islandora_undeletable_datastreams($models) {
  return array('DC');
}

function islandora_datastream_edit_get_link($object, $ds_id) {
  $edit_registry = module_invoke_all('islandora_edit_datastream_registry', $object, $ds_id);
  if (count($edit_registry) > 0 && user_access(FEDORA_METADATA_EDIT)) {
    return l(t('edit'), 'islandora/object/' . $object->id . '/datastream/' . $ds_id . '/edit');
  }
  else {
    return '';
  }
}

function islandora_edit_datastream($object, $ds_id) {
  $edit_registry = module_invoke_all('islandora_edit_datastream_registry', $object, $ds_id);
  $edit_count = count($edit_registry);

  if ($edit_count == 0) {
    // No edit implementations.
    drupal_set_message(t('There are no edit methods specified for this datastream.'));
    drupal_goto('islandora/object/' . $object->id . '/manage/datastreams');
  }
  elseif ($edit_count == 1) {
    // One registry implementation, go there
    drupal_goto($edit_registry[0]['url']);
  }
  else {
    // Multiple edit routes registered
    return islandora_edit_datastream_registry_render($edit_registry);
  }
}

// @TODO: theme / preprocess
function islandora_edit_datastream_registry_render($edit_registry) {
  $output = array(
    '#type' => 'markup',
    '#markup' => '',
  );
  foreach ($edit_registry AS $edit_route) {
    $output['#markup'] .= l($edit_route['name'], $edit_route['url']) . '<br/>';
  }
  return $output;
}
