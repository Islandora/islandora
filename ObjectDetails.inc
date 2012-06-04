<?php

function fedora_repository_islandora_object_details_display() {
  $profiles = array(
    'hidden' => array(
      "name" => "Hidden",
      "module" => "fedora_repository",
      "file" => "ObjectDetails.inc",
      "function" => "fedora_repository_object_details_hidden",
      "description" => t("No object details page"),
    ),
    'xslt' => array(
      "name" => "XSLT",
      "module" => "fedora_repository",
      "file" => "ObjectDetails.inc",
      "function" => "fedora_repository_object_details_xslt",
      "description" => t("Show a datastream with an XSLT"),
      "config" => "admin/settings/fedora_repository/object_details_xslt",
    ),
    'table' => array(
      "name" => "Table",
      "module" => "fedora_repository",
      "file" => "ObjectDetails.inc",
      "function" => "fedora_repository_object_details_table",
      "description" => t("Show a datastream with a table"),
      "config" => "admin/settings/fedora_repository/object_details_table",
    )
  );
  return $profiles;
}

function fedora_repository_object_details_hidden($item) {
  // do nothing
  return "";
}

function fedora_repository_object_details_XSLT($item) {
  global $base_url;
  $path = drupal_get_path('module', 'fedora_repository');
  module_load_include('inc', 'fedora_repository', 'ConnectionHelper');

  $dsid = variable_get('islandora_object_details_xslt_datastream', 'DC');
  // special case for DC+QDC for backward compatibility
  if ($dsid == 'DC' || $dsid == 'QDC') {
    $dsid = array_key_exists('QDC', $item->get_datastreams_list_as_array()) ? 'QDC' : 'DC';
  }
  $xmlstr = $item->get_datastream_dissemination($dsid);

  if (empty($xmlstr)) {
    return '';
  }

  try {
    $proc = new XsltProcessor();
  } catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    watchdog('fedora_repository', "Error while creating XSLT processor: @e", array('@e' => $e->getMessage()), WATCHDOG_ERROR);
    return;
  }

  $proc->setParameter('', 'baseUrl', $base_url);
  $proc->setParameter('', 'path', $base_url . '/' . $path);
  $input = NULL;
  
  $xsl_file = './'. $path .'/'. variable_get('islandora_object_details_xslt_sheet', 'xsl/convertQDC.xsl');
  if (is_readable($xsl_file)) {
    $xsl = new DOMDocument();
    $xsl->load($xsl_file);
    $input = new DOMDocument();
    $input->loadXML(trim($xmlstr));
    $xsl = $proc->importStylesheet($xsl);
    $newdom = $proc->transformToDoc($input);
    $output = $newdom->saveHTML();
    return $output;
  }
  else {
    watchdog('fedora_repository', 'The XSLT file @xslt_name is not readable.', array(
      '@xslt_name' => $xsl_file,
    ));
  }
}

function fedora_repository_object_details_table($item) {
  global $base_url;
  $path = drupal_get_path('module', 'fedora_repository');
  module_load_include('inc', 'fedora_repository', 'ConnectionHelper');

  $dsid = variable_get('islandora_object_details_table_datastream', 'DC');
  // special case for DC+QDC for backward compatibility
  if ($dsid == 'DC' || $dsid == 'QDC') {
    $dsid = array_key_exists('QDC', $item->get_datastreams_list_as_array()) ? 'QDC' : 'DC';
  }
  $xmlstr = $item->get_datastream_dissemination($dsid);

  if (empty($xmlstr)) {
    return '';
  }

  $simplexml = new SimpleXMLElement($xmlstr);

  $headers = array(
    array(
      'data' => t('Metadata'),
      'colspan' => 2,
    ),
  );
  $rows = array();
  foreach ($simplexml->getNamespaces(TRUE) as $ns) {
    foreach ($simplexml->children($ns) as $child) {
      $rows[] = array(
        array(
          'data' => $child->getName(),
          'class' => 'dc-tag-name',
        ),
        array(
          'data' => (string)$child,
          'class' => 'dc-content',
        ),
      );
    }
  }

  return theme('table', $headers, $rows, array('class' => 'dc-table'));
}

// configuration pages
function fedora_repository_object_details_XSLT_config() {
  $form = array();
  $form['config'] = array(
    '#type' => 'fieldset',
    '#title' => t("XSLT display options"),
  );

  $form['config']['xslt'] = array(
    '#type' => 'textfield',
    '#title' => t("XSL transform to use"),
    '#default_value' => variable_get('islandora_object_details_xslt_sheet', 'xsl/convertQDC.xsl'),
    '#required' => TRUE,
  );
  $form['config']['dsid'] = array(
    '#type' => 'textfield',
    '#title' => t("Datastream to transform"),
    '#default_value' => variable_get('islandora_object_details_xslt_datastream', 'DC'),
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Submit"),
    '#weight' => 1,
  );

  return $form;
}

function fedora_repository_object_details_table_config() {
  $form = array();
  $form['config'] = array(
    '#type' => 'fieldset',
    '#title' => t("Table display options"),
  );

  $form['config']['dsid'] = array(
    '#type' => 'textfield',
    '#title' => t("Datastream to transform"),
    '#default_value' => variable_get('islandora_object_details_table_datastream', 'DC'),
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Submit"),
    '#weight' => 1,
  );

  return $form;
}

function fedora_repository_object_details_XSLT_config_submit($form, &$form_state) {
  variable_set('islandora_object_details_xslt_sheet', $form_state['values']['xslt']);
  variable_set('islandora_object_details_xslt_datastream', $form_state['values']['dsid']);
}

function fedora_repository_object_details_table_config_submit($form, &$form_state) {
  variable_set('islandora_object_details_table_datastream', $form_state['values']['dsid']);
}
