<?php

/**
 * @file
 *
 * This file contains all admin and callback functions for solution pack
 * management.
 */

/**
 * Solution pack admin page callback.
 *
 * @return string
 *   The html repersentation of all solution pack forms for required objects.
 */
function islandora_solution_packs_admin() {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (($connection = islandora_get_tuque_connection()) === NULL) {
    islandora_display_repository_inaccessible_message();
    return;
  }
  drupal_add_css(drupal_get_path('module', 'islandora') . '/css/islandora.admin.css');
  $output = '';
  $enabled_solution_packs = module_invoke_all('islandora_required_objects', $connection);
  foreach ($enabled_solution_packs as $solution_pack_module => $solution_pack_info) {
    // @todo We should probably get the title of the solution pack from the
    // systems table for consistency in the interface.
    $solution_pack_name = $solution_pack_info['title'];
    $objects = array_filter($solution_pack_info['objects']);
    $form = drupal_get_form('islandora_solution_pack_form_' . $solution_pack_module, $solution_pack_module, $solution_pack_name, $objects);
    $output .= drupal_render($form);
  }
  return $output;
}

/**
 * A solution pack form for the given module, it lists all the given objects and
 * their status, allowing the user to re-ingest them.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param string $solution_pack_module
 *   The module which requires the given objects.
 * @param string $solution_pack_name
 *   The name of the solution pack to display to the users.
 * @param array $objects
 *   An array of NewFedoraObjects which describe the state in which objects
 *   should exist.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_solution_pack_form(array $form, array &$form_state,  $solution_pack_module, $solution_pack_name, $objects = array()) {
  // The order is important in terms of severity of the status, where higher
  // index indicates the status is more serious, this will be used to determine
  // what messages get displayed to the user.
  $ok_image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
  $warning_image = theme_image(array('path' => 'misc/watchdog-warning.png', 'attributes' => array()));
  $status_info = array(
    'up_to_date' => array(
      'object' => t('Up-to-date'),
      'solution_pack' => t('All required objects are installed and up-to-date.'),
      'image' => $ok_image,
      'button' => t("Force reinstall objects"),
    ),
    'out_of_date' => array(
      'object' => t('Out-of-date'),
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects")
    ),
    'missing_datastream' => array(
      'object' => t('Missing Datastream'),
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects")
    ),
    'missing' => array(
      'object' => t('Missing'),
      'solution_pack' => t( 'Some objects are missing and must be installed. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Install objects")
    ),
  );
  $status_severities = array_keys($status_info);
  $solution_pack_status_severity = array_search('up_to_date', $status_severities);
  $table_rows = array();
  foreach ($objects as $object) {
    $object_status = islandora_check_object_status($object);
    $object_status_info = $status_info[$object_status];
    $object_status_severity = array_search($object_status, $status_severities);
    // The solution pack status severity will be the highest severity of the objects.
    $solution_pack_status_severity = max($solution_pack_status_severity, $object_status_severity);
    $exists = $object_status != 'missing';
    $label = $exists ? l($object->label, "islandora/object/{$object->id}") : $object->label;
    $status_msg = "{$object_status_info['image']}&nbsp{$object_status_info['object']}";
    $table_rows[] = array($label, $object->id, $status_msg);
  }
  $solution_pack_status = $status_severities[$solution_pack_status_severity];
  $solution_pack_status_info = $status_info[$solution_pack_status];
  return array(
    'solution_pack' => array(
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#attributes' => array('class' => array('islandora-solution-pack-fieldset')),
      'solution_pack_module' => array(
        '#type' => 'value',
        '#value' => $solution_pack_module,
      ),
      'solution_pack_name' => array(
        '#type' => 'value',
        '#value' => $solution_pack_name,
      ),
      'objects' => array(
        '#type' => 'value',
        '#value' => $objects,
      ),
      'solution_pack_label' => array(
        '#markup' => $solution_pack_name,
        '#prefix' => '<h3>',
        '#suffix' => '</h3>',
      ),
      'install_status' => array(
        '#markup' => t('<strong>Object status:</strong> !image !status', array(
                     '!image' => $solution_pack_status_info['image'],
                     '!status' => $solution_pack_status_info['solution_pack']
                   )),
        '#prefix' => '<div class="islandora-solution-pack-install-status">',
        '#suffix' => '</div>',
      ),
      'table' => array(
        '#type' => 'item',
        '#markup' => theme('table', array('header' => array(t('Label'), t('PID'), t('Status')), 'rows' => $table_rows))
      ),
      'submit' => array(
        '#type' => 'submit',
        '#name' => $solution_pack_module,
        '#value' => $solution_pack_status_info['button'],
        '#attributes' => array('class' => array('islandora-solution-pack-submit')),
      )
    )
  );
}

/**
 * Submit handler for solution pack form.
 *
 * @param array $form
 *   The form submitted.
 * @param array $form_state
 *   The state of the form submited.
 */
function islandora_solution_pack_form_submit(array $form, array &$form_state) {
  $solution_pack_module = $form_state['values']['solution_pack_module'];
  $solution_pack_name = $form_state['values']['solution_pack_name'];
  $objects = $form_state['values']['objects'];
  $batch = array(
    'title' => t('Installing / Updating solution pack objects'),
    'file' => drupal_get_path('module', 'islandora') . '/includes/solution_packs.inc',
    'operations' => array(),
  );
  foreach ($objects as $object) {
    $batch['operations'][] = array('islandora_solution_pack_batch_operation_reingest_object', array($object));
  }
  batch_set($batch);
  // Hook to let solution pack objects be modified.
  // Not using module_invoke so solution packs can be expanded by other modules.
  // @todo shouldn't we send the object list along as well?
  module_invoke_all('islandora_postprocess_solution_pack', $solution_pack_module);
}

/**
 * Batch operation used by the solution pack forms to ingest/reingest required
 *  object(s)
 *
 * @param NewFedoraObject $object
 *   The object to ingest/reingest.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_solution_pack_batch_operation_reingest_object(NewFedoraObject $object, array &$context) {
  $deleted = FALSE;
  $existing_object = islandora_object_load($object->id);
  if ($existing_object) {
    $deleted = islandora_delete_object($existing_object);
    $purged = $deleted && $exsiting_object == NULL;
    if (!$purged) {
      $object_link = l($existing_object->label, "islandora/object/{$existing_object->id}");
      drupal_set_message(t('Failed to purge existing object !object_link.', array(
            '!object_link' => $object_link,
          )), 'error');
      // Failed to purge don't attempt to ingest.
      return;
    }
  }
  // Object was deleted or did not exist.
  $pid = $object->id;
  $label = $object->label;
  $action = $deleted ? 'reinstalled' : 'installed';
  $object_link = l($label, "islandora/object/{$pid}");
  $object = islandora_add_object($object);
  $msg = $object ? "Successfully $action !object_link." : "Failed to $action @label identified by @pid.";
  $status = $object ? 'status' : 'error';
  drupal_set_message(t($msg, array(
        '@pid' => $pid,
        '@label' => $label,
        '!object_link' => $object_link
      )), $status);
}

/**
 * This is to be called from the solution pack's hook_install()
 * and hook_uninstall() functions. It provides a convient way to have a
 * solution pack's required objects ingested at install time.
 *
 * @param string $module_name
 *   The name of the module that is calling this function in its
 *   install/unistall hooks.
 * @param string $op
 *   The operation to perform, either install or uninstall.
 *
 * @todo Implement hook_modules_installed/hook_modules_uninstalled instead of
 *   calling this function directly.
 * @todo Remove the second parameter and have two seperate functions.
 */
function islandora_install_solution_pack($module, $op = 'install') {
  if ($op == 'uninstall') {
    islandora_uninstall_solution_pack($module);
    return;
  }
  module_load_include('module', 'islandora', 'islandora');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('module', $module, $module);
  $info_file = drupal_get_path('module', $module) . "/{$module}.info";
  $info_array = drupal_parse_info_file($info_file);
  $module_name = $info_array['name'];
  $admin_link = l(t('Solution Pack admin'), 'admin/islandora/solution_packs');
  $config_link = l(t('Islandora configuration'), 'admin/islandora/configure');
  if (!islandora_describe_repository()) {
    $msg = '@module: Did not install any objects. Could not connect to the ';
    $msg .= 'repository. Please check the settings on the !config_link page ';
    $msg .= 'and install the required objects manually on the !admin_link page.';
    drupal_set_message(st($msg, array(
          '@module' => $module_name,
          '!config_link' => $config_link,
          '@admin_link' => $admin_link
        )), 'error');
    return;
  }
  $connection = islandora_get_tuque_connection();
  $islandora_required_objects = $module . '_islandora_required_objects';
  $required_objects = $islandora_required_objects($connection);
  $objects = $required_objects[$module]['objects'];
  $status_messages = array(
    'up_to_date' => 'The object already exists and is up-to-date',
    'missing_datastream' => 'The object already exists but is missing a datastream. Please reinstall the object on the !admin_link page',
    'out_of_date' => 'The object already exists but is out-of-date. Please update the object on the !admin_link page'
  );
  foreach ($objects as $object) {
    $query = $connection->api->a->findObjects('query', 'pid=' . $object->id);
    $already_exists = !empty($query['results']);
    $label =  $object->label;
    $object_link = l($label, "islandora/object/{$object->id}");
    if ($already_exists) {
      $object_status = islandora_check_object_status($object);
      $status_msg = $status_messages[$object_status];
      drupal_set_message(st("@module: Did not install !object_link. $status_msg.", array(
            '@module' => $module_name,
            '!object_link' => $object_link,
            '!admin_link' => $admin_link,
          )), 'warning');
    }
    else {
      $object = islandora_add_object($object);
      if ($object) {
        drupal_set_message(t('@module: Successfully installed. !object_link.', array(
            '@module' => $module_name,
            '!object_link' => $object_link,
          )), 'status');
      }
      else {
        drupal_set_message(t('@module: Failed to install. @label.', array(
              '@module' => $module_name,
              '@label' => $label,
            )), 'warning');
      }
    }
  }
}

/**
 * Uninstalls the given solution pack.
 *
 * @param string $module
 *   The solution pack to uninstall.
 *
 * @todo Implement hook_modules_uninstalled instead of calling this function
 *    directly for each solution pack.
 */
function islandora_uninstall_solution_pack($module) {
  module_load_include('module', 'islandora', 'islandora');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('module', $module, $module);
  $admin_link = l(t('Solution Pack admin'), 'admin/islandora/solution_packs');
  $config_link = l(t('Islandora configuration'), 'admin/islandora/configure');
  $info_file = drupal_get_path('module', $module) . "/{$module}.info";
  $info_array = drupal_parse_info_file($info_file);
  $module_name = $info_array['name'];
  if (!islandora_describe_repository()) {
    $msg = '@module: Did not uninstall any objects. Could not connect to the ';
    $msg .= 'repository. Please check the settings on the !config_link page ';
    $msg .=  'and uninstall the required objects manually if necessary.';
    drupal_set_message(st($msg, array(
          '@module' => $module_name,
          '!config_link' => $config_link
        )), 'error');
    return;
  }
  $connection = islandora_get_tuque_connection();
  $islandora_required_objects = $module . '_islandora_required_objects';
  $required_objects = $islandora_required_objects($connection);
  $objects = $required_objects[$module]['objects'];
  $existing_objects = array_filter($objects, function($o) use($connection) {
                        $param = "pid={$o->id}";
                        $query = $connection->api->a->findObjects('query', $param);
                        return !empty($query['results']);
                      }
  );
  foreach ($existing_objects as $object) {
    $msg = '@module: Did not remove !object_link. It may be used by other sites.';
    $object_link = l($object->label, "islandora/object/{$object->id}");
    drupal_set_message(st($msg, array(
          '!object_link' => $object_link,
          '@module' => $module_name
        )), 'warning');
  }
}

/**
 * Function to check the status of an object against an object model array.
 *
 * @param NewFedoraObject $object_definition
 *   A new fedora object that defines what the object should contain.
 *
 * @return string
 *   Returns one of the following values:
 *   up_to_date, missing, missing_datastream or out_of_date
 *   You can perform an appropriate action based on this value.
 *
 * @see islandora_solution_pack_form()
 * @see islandora_install_solution_pack()
 * @todo Should this function live in islandora.module so it can be called
 *       easier without having to include the solution_packs.inc file?
 */
function islandora_check_object_status(NewFedoraObject $object_definition) {
  $existing_object = islandora_object_load($object_definition->id);
  if (!$existing_object) {
    return 'missing';
  }
  $existing_datastreams = array_keys(iterator_to_array($existing_object));
  $expected_datastreams = array_keys(iterator_to_array($object_definition));
  $datastream_diff = array_diff($expected_datastreams, $existing_datastreams);
  if (!empty($datastream_diff)) {
    return 'missing_datastream';
  }
  $is_xml_datastream = function($ds) { return $ds->mimetype == 'text/xml'; };
  $xml_datastreams = array_filter(iterator_to_array($object_definition), $is_xml_datastream);
  foreach ($xml_datastreams as $ds) {
    $installed_version = islandora_get_islandora_datastream_version($existing_object, $ds->id);
    $available_version = islandora_get_islandora_datastream_version($object_definition, $ds->id);
    if ($available_version > $installed_version) {
      return 'out_of_date';
    }
  }
  // If not anything else we can assume its up to date.
  return 'up_to_date';
}

/**
 * @defgroup viewer-functions
 * @{
 * Helper functions to include viewers for solution packs.
 */

/**
 * A form construct to create a viewer selection table.
 *
 * @param string $variable_id
 *   The ID of the Drupal variable to save the viewer settings in
 * @param string $mimetype
 *   The table will be populated with viewers supporting this mimetype
 * @return
 *   A form api array which defines a themed table to select a viewer.
 */
function islandora_viewers_form($variable_id = NULL, $mimetype = NULL) {
  $form = array();
  // get viewers
  $viewers = islandora_get_viewers($mimetype);
  if (!empty($viewers)) {
    // add option for no viewer
    $no_viewer = array();
    $no_viewer['none'] = array(
      'label' => t('None'),
      'description' => t('Don\'t use a viewer for this solution pack.'),
    );
    // merge to viewers array
    $viewers = array_merge_recursive($no_viewer, $viewers);
  }
  // get viewer settings
  $viewers_config = variable_get($variable_id, array());
  // viewer
  $form['viewers'] = array(
    '#type' => 'fieldset',
    '#title' => t('Viewers'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  if (!empty($viewers)) {
    // viewers table
    $form['viewers'][$variable_id] = array(
      '#type' => 'item',
      '#title' => t('Select a viewer'),
      '#description' => t('Preferred viewer for your solution pack. These may be provided by third-party modules.'),
      // This attribute is important to return the submitted values in a deeper
      // nested arrays in
      '#tree' => TRUE,
      '#theme' => 'islandora_viewers_table',
    );

    // table loop
    foreach ($viewers as $name => $profile) {

      $options[$name] = '';
        // machine name
      $form['viewers'][$variable_id]['name'][$name] = array(
        '#type' => 'hidden',
        '#value' => $name,
      );
      // label
      $form['viewers'][$variable_id]['label'][$name] = array(
        '#type' => 'item',
        '#markup' => $profile['label'],
      );
      // description
      $form['viewers'][$variable_id]['description'][$name] = array(
        '#type' => 'item',
        '#markup' => $profile['description'],
      );
      // configuration url
      $form['viewers'][$variable_id]['configuration'][$name] = array(
        '#type' => 'item',
        '#markup' => (isset($profile['configuration']) AND $profile['configuration'] != '') ? l(t('configure'), $profile['configuration']) : '',
      );
    }
    // radios
    $form['viewers'][$variable_id]['default'] = array(
      '#type' => 'radios',
      '#options' => isset($options) ? $options : array(),
      '#default_value' => !empty($viewers_config) ? $viewers_config['default'] : 'none',
    );
  }
  else {
    // No viewers found
    $form['viewers'][$variable_id . '_no_viewers'] = array(
      '#markup' => t('No viewers detected.'),
    );
    // remove viewers variable
    variable_del($variable_id);
  }
  return $form;
}

/**
 * Returns all defined viewers.
 *
 * @param string $mimetype
 *  specify a mimetype to return only viewers that support this certain
 *  mimetype.
 * @return
 *  array of viewer definitions, or FALSE if none are found.
 */
function islandora_get_viewers($mimetype = NULL) {
  $viewers = array();
  // get all viewers
  $defined_viewers = module_invoke_all('islandora_viewer_info');
  // filter viewers by mimetype
  foreach ($defined_viewers as $key => $value) {
    if (in_array($mimetype, $value['mimetype']) OR $mimetype == NULL) {
      $viewers[$key] = $value;
    }
  }
  if (!empty($viewers)) {
    return $viewers;
  }
  return FALSE;
}

/**
 * Theme function for the admin primary display table
 *
 * @param type $variables
 *   render element: $form
 *   Contains the form array
 * @return
 *   rendered form element
 *
 *  @see islandora_large_image_admin()
 */
function theme_islandora_viewers_table($variables) {
  // set form
  $form = $variables['form'];
  $rows = array();

  foreach ($form['name'] as $key => $element) {
    // Do not take form control structures.
    if (is_array($element) && element_child($key)) {
      // set rows
      $row = array();
      $row[] = array('data' => drupal_render($form['default'][$key]));
      $row[] = array('data' => drupal_render($form['label'][$key]));
      $row[] = array('data' => drupal_render($form['description'][$key]));
      $row[] = array('data' => drupal_render($form['configuration'][$key]));

      // add to rows
      $rows[] = array('data' => $row);
    }
  }

  // Individual table headers.
  // default | label | description | configuration
  $header = array();
  $header[] = array('data' => t('Default'));
  $header[] = array('data' => t('Label'));
  $header[] = array('data' => t('Description'));
  $header[] = array('data' => t('Configuration'));

  // render form
  $output = '';
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'islandora-viewers-table')));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Gather information and return a rendered viewer
 *
 * @param array/string $params
 *   Array or string with data the module needs in order to render a full viewer
 * @param string $variable_id
 *   The id of the Drupal variable the viewer settings are saved in
 * @return
 *   The callback to the viewer module. Returns a rendered viewer. Returns FALSE
 *   if no viewer is set.
 */
function islandora_get_viewer($params = NULL, $variable_id = NULL) {
  // get viewer from settings
  $settings = variable_get($variable_id, array());
  // make sure a viewer is set
  if (!empty($settings) AND $settings['default'] !== 'none') {
    // get callback function
    $viewer_id = islandora_get_viewer_id($variable_id);
    if ($viewer_id AND $params !== NULL) {
      $callback = islandora_get_viewer_callback($viewer_id);
      // call callback function
      return $callback($params);
    }
  }
  return NULL;
}

/**
 * Get id of the enabled viewer.
 *
 * @param string $variable_id
 *   The id of the Drupal variable the viewer settings are saved in
 * @return
 *   The enabled viewer id. Returns FALSE if no viewer config is set.
 */
function islandora_get_viewer_id($variable_id) {
  $viewers_config = variable_get($variable_id, array());
  if (!empty($viewers_config)) {
    return $viewers_config['default'];
  }
  return FALSE;
}

/**
 * Get callback function for a viewer.
 *
 * @param string $viewer_id
 *   The ID of a viewer
 * @return
 *   The callback function as a string as defined by the viewer.
 */
function islandora_get_viewer_callback($viewer_id = NULL) {
  if ($viewer_id !== NULL) {
    $viewers = module_invoke_all('islandora_viewer_info');
    if (isset($viewers[$viewer_id]['callback'])) {
      return $viewers[$viewer_id]['callback'];
    }
  }
  return FALSE;
}

/**
 * @} End of "defgroup viewer-functions".
 */
