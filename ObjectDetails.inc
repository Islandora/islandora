<?php

/**
 * @file ObjectDetails.inc
 * The functions required to define and respond to all the default object
 * details display modes.
 */

/**
 * Islandora core object details display modes.  These are the default display
 * modes that are always available.
 * @return A list of display modes that can be used to render the object details
 *         page.
 */
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

/**
 * The renderer for the "hidden" display mode.  In this mode, no data is
 * displayed.  This is supplied so you can disable the object details metadata
 * display without disabling the tab entirely.
 * @param item The item with the metadata to display.
 * @return The fully composed object details metadata display.
 */
function fedora_repository_object_details_hidden($item) {
  // do nothing
  return "";
}

/**
 * The renderer for the "xslt" display mode.  In this mode, an xslt is applied
 * to the selected datastream to produce a user defined look and feel to the
 * output data.
 * @param item The item with the metadata to display.
 * @return The fully composed object details metadata display.
 */
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
    return t('Error - could not find datastream @dsid on object @pid<br/>Please contact the site administrator.',
    array('@dsid' => $dsid, '@pid' => $item->pid));
  }

  try {
    $proc = new XsltProcessor();
  } catch (Exception $e) {
    watchdog('fedora_repository', "Error while creating XSLT processor: @e", array('@e' => $e->getMessage()), WATCHDOG_ERROR);
    return;
  }

  $proc->setParameter('', 'baseUrl', $base_url);
  $proc->setParameter('', 'path', $path);
  $input = NULL;

  $xsl_file = variable_get('islandora_object_details_xslt_sheet', 'sites/all/modules/islandora/object_details_xslts/convertQDC.xsl');
  // set an error message in case xslt parsing fails
  $output = t("Failed to parse xslt file at @xsltFile", array('@xsltFile' => $xsl_file));
  if (is_readable($xsl_file)) {
    $xsl = new DomDocument();
    $input = new DomDocument();
    try {
      $xsl->load($xsl_file);

      $input->loadXML(trim($xmlstr));
    } catch (Exception $e) {
      watchdog('fedora_repository', "Problem loading XSL file: @e", array('@e' => $e->getMessage()), NULL, WATCHDOG_ERROR);
    }
    $xsl = $proc->importStylesheet($xsl);
    $newdom = $proc->transformToDoc($input);
    $output = $newdom->saveHTML();
  }
  return $output;
}

/**
 * The renderer for the "table" display mode.  In this mode, the requested
 * datastream is rendered using a simple table with keys(tags) on the left and
 * values on the right.
 * @param item The item with the metadata to display.
 * @return The fully composed object details metadata display.
 */
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
    return t('Error - could not find datastream @dsid on object @pid<br/>Please contact the site administrator.',
    array('@dsid' => $dsid, '@pid' => $item->pid));
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

/**
 * Configuration page for the xslt display mode.
 *
 * This mode requires two parameters: the datastream to render, and the xslt to
 * apply to it.
 *
 * @return
 *   The configuration page.
 */
function fedora_repository_object_details_XSLT_config() {
  $form = array();
  $form['config'] = array(
    '#type' => 'fieldset',
    '#title' => t("XSLT display options"),
  );

  $options = module_invoke_all("object_details_get_available_xslts");

  $form['config']['xslt'] = array(
    '#type' => 'select',
    '#title' => t("XSL transform to use"),
    '#default_value' => variable_get('islandora_object_details_xslt_sheet', 'sites/all/modules/islandora/object_details_xslts/convertQDC.xsl'),
    '#options' => $options,
    '#key_type' => 'associative',
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
/**
 * Custom submit handler for the xslt configuration page.
 *
 * @param $form
 * @param &$form_state
 *   The user supplied values for the form.
 */
function fedora_repository_object_details_XSLT_config_submit($form, &$form_state) {
  variable_set('islandora_object_details_display_table', 'xslt');
  variable_set('islandora_object_details_xslt_sheet', $form_state['values']['xslt']);
  variable_set('islandora_object_details_xslt_datastream', $form_state['values']['dsid']);
  drupal_set_message('Object Details view has been set to XSLT and your configuration has been saved');
}

/**
 * Base function to supply the available xslt files.
 *
 * Modules implementing this hook need to return an array describing where the
 * XSLT is.  The array key is the path to the XSLT (paths start with sites/) and
 * the value in the array is the display name.  In the simplest form you can use
 * file_scan_directory like we do here - this puts the filename as the display
 * name and will automatically detect new files as they are added.
 */
function fedora_repository_object_details_get_available_xslts() {
  $folder = drupal_get_path("module", "fedora_repository") ."/object_details_xslts/";
  // retrieve the filenames from the system
  $xslts = file_scan_directory($folder, ".xsl");
  $options = array();
  foreach ($xslts as $xsl) {
    $options[$xsl->filename] = $xsl->basename;
  }
  return $options;
}

/**
 * Configuration page for the table display mode.  This mode requires only one
 * parameter: the datastream to render.
 *
 * @return
 *   The configuration page.
 */
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

/**
 * Custom submit handler for the table configuration page.
 *
 * @param $form
 * @param &$form_state
 *   The user supplied values for the form.
 */
function fedora_repository_object_details_table_config_submit($form, &$form_state) {
  variable_set('islandora_object_details_display_table', 'table');
  variable_set('islandora_object_details_table_datastream', $form_state['values']['dsid']);
  drupal_set_message('Object Details view has been set to Table and your configuration has been saved');
}
