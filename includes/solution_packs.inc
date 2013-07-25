<?php

/**
 * @file
 * Admin and callback functions for solution pack management.
 */

/**
 * Solution pack admin page callback.
 *
 * @return string
 *   The html repersentation of all solution pack forms for required objects.
 */
function islandora_solution_packs_admin() {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!islandora_describe_repository()) {
    islandora_display_repository_inaccessible_message();
    return '';
  }

  $connection = islandora_get_tuque_connection();
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
 * A solution pack form for the given module.
 *
 * It lists all the given objects and their status, allowing the user to
 * re-ingest them.
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
function islandora_solution_pack_form(array $form, array &$form_state, $solution_pack_module, $solution_pack_name, $objects = array()) {
  // The order is important in terms of severity of the status, where higher
  // index indicates the status is more serious, this will be used to determine
  // what messages get displayed to the user.
  $ok_image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
  $warning_image = theme_image(array('path' => 'misc/watchdog-warning.png', 'attributes' => array()));
  $status_info = array(
    'up_to_date' => array(
      'solution_pack' => t('All required objects are installed and up-to-date.'),
      'image' => $ok_image,
      'button' => t("Force reinstall objects"),
    ),
    'modified_datastream' => array(
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects"),
    ),
    'out_of_date' => array(
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects"),
    ),
    'missing_datastream' => array(
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects"),
    ),
    'missing' => array(
      'solution_pack' => t('Some objects are missing and must be installed. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Install objects"),
    ),
  );
  $status_severities = array_keys($status_info);
  $solution_pack_status_severity = array_search('up_to_date', $status_severities);
  $table_rows = array();
  foreach ($objects as $object) {
    $object_status = islandora_check_object_status($object);
    $object_status_info = $status_info[$object_status['status']];
    $object_status_severity = array_search($object_status['status'], $status_severities);
    // The solution pack status severity will be the highest severity of
    // the objects.
    $solution_pack_status_severity = max($solution_pack_status_severity, $object_status_severity);
    $exists = $object_status['status'] != 'missing';
    $label = $exists ? l($object->label, "islandora/object/{$object->id}") : $object->label;
    $status_msg = "{$object_status_info['image']}&nbsp{$object_status['status_friendly']}";
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
                     '!status' => $solution_pack_status_info['solution_pack'],
                   )),
        '#prefix' => '<div class="islandora-solution-pack-install-status">',
        '#suffix' => '</div>',
      ),
      'table' => array(
        '#type' => 'item',
        '#markup' => theme('table', array(
                     'header' => array(t('Label'), t('PID'), t('Status')),
                     'rows' => $table_rows)),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#name' => $solution_pack_module,
        '#value' => $solution_pack_status_info['button'],
        '#attributes' => array('class' => array('islandora-solution-pack-submit')),
      ),
    ),
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
 * Batch operation to ingest/reingest required object(s).
 *
 * @param AbstractObject $object
 *   The object to ingest/reingest.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_solution_pack_batch_operation_reingest_object(AbstractObject $object, array &$context) {
  $existing_object = islandora_object_load($object->id);
  $deleted = FALSE;
  if ($existing_object) {
    $deleted = islandora_delete_object($existing_object);
    if (!$deleted) {
      $object_link = l($existing_object->label, "islandora/object/{$existing_object->id}");
      drupal_set_message(filter_xss(t('Failed to purge existing object !object_link.', array(
            '!object_link' => $object_link,
          ))), 'error');
      // Failed to purge don't attempt to ingest.
      return;
    }
  }

  // Object was deleted or did not exist.
  $pid = $object->id;
  $label = $object->label;
  $object_link = l($label, "islandora/object/{$pid}");
  $object = islandora_add_object($object);
  $params = array(
    '@pid' => $pid,
    '@label' => $label,
    '!object_link' => $object_link,
  );

  $msg = '';
  if ($object) {
    if ($deleted) {
      $msg = t('Successfully reinstalled !object_link.', $params);
    }
    else {
      $msg = t('Successfully installed !object_link.', $params);
    }
  }
  elseif ($deleted) {
    $msg = t('Failed to reinstall @label, identified by @pid.', $params);
  }
  else {
    $msg = t('Failed to install @label, identified by @pid.', $params);
  }

  $status = $object ? 'status' : 'error';
  drupal_set_message(filter_xss($msg), $status);
}

/**
 * Install the given solution pack.
 *
 * This is to be called from the solution pack's hook_install() and
 * hook_uninstall() functions.
 *
 * It provides a convient way to have a solution pack's required objects
 * ingested at install time.
 *
 * @param string $module
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

  // Login as user 1 if the installation is coming from the command line (e.g.
  // drush).
  if (drupal_is_cli()) {
    global $user;
    $user = user_load(1);
    user_login_finalize();
  }

  $t = get_t();

  // Some general replacements.
  $admin_link = l($t('Solution Pack admin'), 'admin/islandora/solution_packs');
  $config_link = l($t('Islandora configuration'), 'admin/islandora/configure');
  $t_params = array(
    '@module' => $module,
    '!config_link' => $config_link,
    '!admin_link' => $admin_link,
  );

  module_load_include('module', 'islandora', 'islandora');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('module', $module, $module);
  $info_file = drupal_get_path('module', $module) . "/{$module}.info";
  $info_array = drupal_parse_info_file($info_file);
  $module_name = $info_array['name'];
  if (!islandora_describe_repository()) {
    $msg = $t('@module: Did not install any objects. Could not connect to the repository. Please check the settings on the !config_link page and install the required objects manually on the !admin_link page.', $t_params);
    drupal_set_message(filter_xss($msg), 'error');
    return;
  }
  $connection = islandora_get_tuque_connection();
  $required_objects = module_invoke($module, 'islandora_required_objects', $connection);
  $objects = $required_objects[$module]['objects'];
  $status_messages = array(
    'up_to_date' => $t('The object already exists and is up-to-date.', $t_params),
    'missing_datastream' => $t('The object already exists but is missing a datastream. Please reinstall the object on the !admin_link page.', $t_params),
    'out_of_date' => $t('The object already exists but is out-of-date. Please update the object on the !admin_link page.', $t_params),
    'modified_datastream' => $t('The object already exists but datastreams are modified. Please reinstall the object on the !admin_link page.', $t_params),
  );
  foreach ($objects as $object) {
    $query = $connection->api->a->findObjects('query', 'pid=' . $object->id);
    $already_exists = !empty($query['results']);
    $label = $object->label;
    $object_link = l($label, "islandora/object/{$object->id}");
    if ($already_exists) {
      $object_status = islandora_check_object_status($object);
      $here_params = array(
        '!summary' => $t("@module: Did not install !object_link.", array(
            '!object_link' => $object_link,
          ) + $t_params),
        '!description' => $status_messages[$object_status['status']],
      );
      drupal_set_message(filter_xss(format_string('!summary !description', $here_params)), 'warning');
    }
    else {
      $object = islandora_add_object($object);
      if ($object) {
        drupal_set_message(filter_xss($t('@module: Successfully installed. !object_link.', array(
            '!object_link' => $object_link,
          ) + $t_params)), 'status');
      }
      else {
        drupal_set_message($t('@module: Failed to install. @label.', array(
              '@label' => $label,
            ) + $t_params), 'warning');
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

  // Login as user 1 if the uninstall is coming from the command line (e.g.
  // drush).
  if (drupal_is_cli()) {
    global $user;
    $user = user_load(1);
    user_login_finalize();
  }

  $t = get_t();
  module_load_include('module', 'islandora', 'islandora');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('module', $module, $module);
  $config_link = l($t('Islandora configuration'), 'admin/islandora/configure');
  $info_file = drupal_get_path('module', $module) . "/{$module}.info";
  $info_array = drupal_parse_info_file($info_file);
  $module_name = $info_array['name'];
  if (!islandora_describe_repository()) {
    $msg = $t('@module: Did not uninstall any objects. Could not connect to the repository. Please check the settings on the !config_link page and uninstall the required objects manually if necessary.', array(
      '@module' => $module_name,
      '!config_link' => $config_link,
    ));
    drupal_set_message(filter_xss($msg), 'error');
    return;
  }
  $connection = islandora_get_tuque_connection();
  $required_objects = module_invoke($module, 'islandora_required_objects', $connection);
  $objects = $required_objects[$module]['objects'];
  $filter = function($o) use($connection) {
    $param = "pid={$o->id}";
    $query = $connection->api->a->findObjects('query', $param);
    return !empty($query['results']);
  };
  $existing_objects = array_filter($objects, $filter);
  foreach ($existing_objects as $object) {
    $object_link = l($object->label, "islandora/object/{$object->id}");
    $msg = $t('@module: Did not remove !object_link. It may be used by other sites.', array(
      '!object_link' => $object_link,
      '@module' => $module_name,
    ));

    drupal_set_message(filter_xss($msg), 'warning');
  }
}

/**
 * Function to check the status of an object against an object model array.
 *
 * @param AbstractObject $object_definition
 *   A new fedora object that defines what the object should contain.
 *
 * @return string
 *   Returns one of the following values:
 *   up_to_date, missing, missing_datastream or out_of_date
 *   You can perform an appropriate action based on this value.
 *
 * @see islandora_solution_pack_form()
 * @see islandora_install_solution_pack()
 */
function islandora_check_object_status(AbstractObject $object_definition) {
  $existing_object = islandora_object_load($object_definition->id);
  if (!$existing_object) {
    return array('status' => 'missing', 'status_friendly' => t('Missing'));
  }

  $existing_datastreams = array_keys(iterator_to_array($existing_object));
  $expected_datastreams = array_keys(iterator_to_array($object_definition));
  $datastream_diff = array_diff($expected_datastreams, $existing_datastreams);
  if (!empty($datastream_diff)) {
    $status_friendly = format_plural(count($datastream_diff), 'Missing Datastream: %dsids.', 'Missing Datastreams: %dsids.', array('%dsids' => implode(', ', $datastream_diff)));
    return array(
      'status' => 'missing_datastream',
      'status_friendly' => $status_friendly,
      'data' => $datastream_diff,
    );
  }

  $is_xml_datastream = function($ds) {
    return $ds->mimetype == 'text/xml';
  };
  $xml_datastreams = array_filter(iterator_to_array($object_definition), $is_xml_datastream);
  $out_of_date_datastreams = array();
  foreach ($xml_datastreams as $ds) {
    $installed_version = islandora_get_islandora_datastream_version($existing_object, $ds->id);
    $available_version = islandora_get_islandora_datastream_version($object_definition, $ds->id);
    if ($available_version > $installed_version) {
      $out_of_date_datastreams[] = $ds->id;
    }
  }

  if (count($out_of_date_datastreams)) {
    $status_friendly = format_plural(count($out_of_date_datastreams), 'Datastream out of date: %dsids.', 'Datastreams out of date: %dsids.', array('%dsids' => implode(', ', $out_of_date_datastreams)));
    return array(
      'status' => 'out_of_date',
      'status_friendly' => $status_friendly,
      'data' => $out_of_date_datastreams,
    );
  }

  // This is a pretty heavy function, but I'm not sure a better way. If we have
  // performance trouble, we should maybe remove this.
  $modified_datastreams = array();
  foreach ($object_definition as $ds) {
    if ($ds->mimetype == 'text/xml'
      || $ds->mimetype == 'application/rdf+xml'
      || $ds->mimetype == 'application/xml') {
      // If the datastream is XML we use the domdocument C14N cannonicalization
      // function to test if they are equal, because the strings likely won't
      // be equal as Fedora does some XML mangling. In order for C14N to work
      // we need to replace the info:fedora namespace, as C14N hates it.
      // C14N also doesn't normalize whitespace at the end of lines and Fedora
      // will sometimes replace new-lines with white-space. So first we strip
      // leading/tailing white-space and replace all new-lines within the xml
      // document to account for Fedora's weird formatting.
      $xsl = new DOMDocument();
      $xsl->load(drupal_get_path('module', 'islandora') . '/xml/strip_newlines_and_whitespace.xsl');
      $xslt = new XSLTProcessor();
      $xslt->importStyleSheet($xsl);
      $object_definition_dom = new DOMDocument();
      $object_definition_dom->preserveWhiteSpace = FALSE;
      $object_definition_dom->loadXML(str_replace('info:', 'http://', $ds->content));
      $object_definition_dom = $xslt->transformToDoc($object_definition_dom);
      $object_actual_dom = new DOMDocument();
      $object_actual_dom->preserveWhiteSpace = FALSE;
      $object_actual_dom->loadXML(str_replace('info:', 'http://', $existing_object[$ds->id]->content));
      $object_actual_dom = $xslt->transformToDoc($object_actual_dom);

      // Fedora changes the xml structure so we need to cannonize it.
      if ($object_actual_dom->C14N() != $object_definition_dom->C14N()) {
        $modified_datastreams[] = $ds->id;
      }
    }
    else {
      $object_definition_hash = md5($ds->content);
      $object_actual_hash = md5($existing_object[$ds->id]->content);
      if ($object_definition_hash != $object_actual_hash) {
        $modified_datastreams[] = $ds->id;;
      }
    }
  }
  if (count($modified_datastreams)) {
    $status_friendly = format_plural(count($modified_datastreams), 'Modified Datastream: %dsids.', 'Modified Datastreams: %dsids.', array('%dsids' => implode(', ', $modified_datastreams)));
    return array(
      'status' => 'modified_datastream',
      'data' => $modified_datastreams,
      'status_friendly' => $status_friendly,
    );
  }

  // If not anything else we can assume its up to date.
  return array('status' => 'up_to_date', 'status_friendly' => t('Up-to-date'));
}

/**
 * @defgroup viewer-functions
 * @{
 * Helper functions to include viewers for solution packs.
 */

/**
 * A form construct to create a viewer selection table.
 *
 * The list of selectable viewers is limited by the $mimetype and the $model
 * parameters. When neither are given all defined viewers are listed. If only
 * $mimetype is given only viewers that support that mimetype will be listed,
 * the same goes for the $model parameter. If both are given, than any viewer
 * that supports either the give $mimetype or $model will be listed.
 *
 * @param string $variable_id
 *   The ID of the Drupal variable to save the viewer settings in
 * @param string $mimetype
 *   The table will be populated with viewers supporting this mimetype
 * @param string $model
 *   The table will be populated with viewers supporting this content model
 *
 * @return array
 *   A form api array which defines a themed table to select a viewer.
 */
function islandora_viewers_form($variable_id = NULL, $mimetype = NULL, $model = NULL) {
  $form = array();
  $viewers = islandora_get_viewers($mimetype, $model);
  if (!empty($viewers)) {
    $no_viewer = array();
    $no_viewer['none'] = array(
      'label' => t('None'),
      'description' => t("Don't use a viewer for this solution pack."),
    );
    $viewers = array_merge_recursive($no_viewer, $viewers);
  }
  $viewers_config = variable_get($variable_id, array());
  $form['viewers'] = array(
    '#type' => 'fieldset',
    '#title' => t('Viewers'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  if (!empty($viewers)) {
    $form['viewers'][$variable_id] = array(
      '#type' => 'item',
      '#title' => t('Select a viewer'),
      '#description' => t('Preferred viewer for your solution pack. These may be provided by third-party modules.'),
      '#tree' => TRUE,
      '#theme' => 'islandora_viewers_table',
    );

    foreach ($viewers as $name => $profile) {
      $options[$name] = '';
      $form['viewers'][$variable_id]['name'][$name] = array(
        '#type' => 'hidden',
        '#value' => $name,
      );
      $form['viewers'][$variable_id]['label'][$name] = array(
        '#type' => 'item',
        '#markup' => $profile['label'],
      );
      $form['viewers'][$variable_id]['description'][$name] = array(
        '#type' => 'item',
        '#markup' => $profile['description'],
      );
      $form['viewers'][$variable_id]['configuration'][$name] = array(
        '#type' => 'item',
        '#markup' => (isset($profile['configuration']) AND $profile['configuration'] != '') ? l(t('configure'), $profile['configuration']) : '',
      );
    }
    $form['viewers'][$variable_id]['default'] = array(
      '#type' => 'radios',
      '#options' => isset($options) ? $options : array(),
      '#default_value' => !empty($viewers_config) ? $viewers_config['default'] : 'none',
    );
  }
  else {
    $form['viewers'][$variable_id . '_no_viewers'] = array(
      '#markup' => t('No viewers detected.'),
    );
    variable_del($variable_id);
  }
  return $form;
}

/**
 * Returns all defined viewers.
 *
 * The list of selectable viewers is limited by the $mimetype and the
 * $content_model parameters. When neither are given all defined viewers are
 * listed. If only $mimetype is given only viewers that support that mimetype
 * will be listed, the same goes for the $content_model parameter. If both are
 * given, than any viewer that supports either the give $mimetype or $model will
 * be listed.
 *
 * @param string $mimetype
 *   Specify a mimetype to return only viewers that support this certain
 *   mimetype.
 * @param string $content_model
 *   Specify a content model to return only viewers that support the content
 *   model.
 *
 * @return array
 *   Viewer definitions, or FALSE if none are found.
 */
function islandora_get_viewers($mimetype = NULL, $content_model = NULL) {
  $viewers = array();
  $defined_viewers = module_invoke_all('islandora_viewer_info');
  // Filter viewers by MIME type.
  foreach ($defined_viewers as $key => $value) {
    $value['mimetype'] = isset($value['mimetype']) ? $value['mimetype'] : array();
    $value['model'] = isset($value['model']) ? $value['model'] : array();
    if (in_array($mimetype, $value['mimetype']) OR in_array($content_model, $value['model'])) {
      $viewers[$key] = $value;
    }
  }
  if (!empty($viewers)) {
    return $viewers;
  }
  return FALSE;
}

/**
 * Implements theme_hook().
 */
function theme_islandora_viewers_table($variables) {
  $form = $variables['form'];
  $rows = array();
  foreach ($form['name'] as $key => $element) {
    if (is_array($element) && element_child($key)) {
      $row = array();
      $row[] = array('data' => drupal_render($form['default'][$key]));
      $row[] = array('data' => drupal_render($form['label'][$key]));
      $row[] = array('data' => drupal_render($form['description'][$key]));
      $row[] = array('data' => drupal_render($form['configuration'][$key]));
      $rows[] = array('data' => $row);
    }
  }
  $header = array();
  $header[] = array('data' => t('Default'));
  $header[] = array('data' => t('Label'));
  $header[] = array('data' => t('Description'));
  $header[] = array('data' => t('Configuration'));

  $output = '';
  $output .= theme('table', array(
               'header' => $header,
               'rows' => $rows,
               'attributes' => array('id' => 'islandora-viewers-table'),
             ));
  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Gather information and return a rendered viewer.
 *
 * @param array/string $params
 *   Array or string with data the module needs in order to render a full viewer
 * @param string $variable_id
 *   The id of the Drupal variable the viewer settings are saved in
 * @param AbstractObject $fedora_object
 *   The tuque object representing the fedora object being displayed
 *
 * @return string
 *   The callback to the viewer module. Returns a rendered viewer. Returns FALSE
 *   if no viewer is set.
 */
function islandora_get_viewer($params = NULL, $variable_id = NULL, $fedora_object = NULL) {
  $settings = variable_get($variable_id, array());
  if (!empty($settings) AND $settings['default'] !== 'none') {
    $viewer_id = islandora_get_viewer_id($variable_id);
    if ($viewer_id AND $params !== NULL) {
      $callback = islandora_get_viewer_callback($viewer_id);
      return $callback($params, $fedora_object);
    }
  }
  return FALSE;
}

/**
 * Get id of the enabled viewer.
 *
 * @param string $variable_id
 *   The ID of the Drupal variable the viewer settings are saved in
 *
 * @return string
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
 *   The ID of a viewer.
 *
 * @return string
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
