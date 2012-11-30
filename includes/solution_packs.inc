<?php

/**
 * @file
 *
 * This file contains all admin and callback functions for solution pack
 * management.
 */

/**
 * Solution pack admin page callback.
 */
function islandora_solution_packs_admin() {
  module_load_include('inc', 'islandora', 'includes/utilities');
  drupal_add_css(drupal_get_path('module', 'islandora') . '/css/islandora.admin.css');
  if (!islandora_describe_repository()) {
    $message = t('Could not connect to the repository. Please check the settings on the ' .
               '<a href="@config_url" title="Islandora configuration page">Islandora configuration</a> page.',
               array('@config_url' => url('admin/islandora/configure')));
    drupal_set_message($message, 'error');
  }
  $enabled_solution_packs = module_invoke_all('islandora_required_objects');
  $output = '';
  foreach ($enabled_solution_packs as $solution_pack_module => $solution_pack_info) {
    $objects = array();
    foreach ($solution_pack_info as $field => $value) {
      switch ($field) {
        case 'title':
          $solution_pack_name = $value;
          break;
        case 'objects':
          $objects = $value;
          break;
      }
    }
    $form_array = drupal_get_form('islandora_solution_pack_form_' . $solution_pack_module, $solution_pack_module, $solution_pack_name, $objects);
    $output .= drupal_render($form_array);
  }
  return $output;
}

/**
 * Solution pack admin page
 */
function islandora_solution_pack_form($form, &$form_state,  $solution_pack_module, $solution_pack_name, $objects = array()) {
  global $base_url;
  $needs_update = FALSE;
  $needs_install = FALSE;
  $could_not_connect = FALSE;
  $form = array();

  $form['solution_pack'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array('class' => array('islandora-solution-pack-fieldset')),
  );

  // adding values
  $form['solution_pack']['solution_pack_module'] = array(
    '#type' => 'value',
    '#value' => $solution_pack_module,
  );
  $form['solution_pack']['solution_pack_name'] = array(
    '#type' => 'value',
    '#value' => $solution_pack_name,
  );
  $form['solution_pack']['objects'] = array(
    '#type' => 'value',
    '#value' => $objects,
  );

  $table_header = array(t('Label'), t('PID'), t('Status'));
  $table_rows = array();

  foreach ($objects as $object) {
    $datastreams = NULL;
    if (isset($object['pid'])) {
      $pid = $object['pid'];
      $table_row = array();
      $object_status = islandora_check_object_status($object);
      switch ($object_status) {
        case 'up_to_date':
          $object_status = t('Up-to-date');
          break;
        case 'missing':
          $object_status = t('Missing');
          $needs_install = TRUE;
          break;
        case 'missing_datastream':
          $object_status = t('Missing datastream');
          $needs_update = TRUE;
          break;
        case 'out_of_date':
          $object_status = t('Out-of-date');
          $needs_update = TRUE;
          break;
        case 'could_not_connect':
          $object_status = t('Could not connect');
          $could_not_connect = TRUE;
          break;
      }
      if ($needs_install OR $could_not_connect) {
        $label = $object['label'] ? $object['label'] : '';
      }
      else {
        $label = $object['label'] ? l($object['label'], $base_url . '/islandora/object/' . $pid) : '';
      }
      $table_row[] = $label;
      $table_row[] = $pid;
      $table_row[] = $object_status;
      $table_rows[] = $table_row;
    }
  }

  // title
  if (!$form_state['submitted']) {
    $form['solution_pack']['solution_pack_label'] = array(
      '#markup' => filter_xss($solution_pack_name),
      '#prefix' => '<h3>',
      '#suffix' => '</h3>',
    );

    $form['solution_pack']['install_status'] = array(
      '#markup' => '<strong>' . t('Object status:') . '&nbsp;</strong>',
      '#prefix' => '<div class="islandora-solution-pack-install-status">',
      '#suffix' => '</div>',
    );
    if (!$needs_install AND !$needs_update AND !$could_not_connect) {
      $form['solution_pack']['install_status']['#markup'] .= ' ' . theme('image', array('path' => 'misc/watchdog-ok.png')) . ' ' . t('All required objects are installed and up-to-date.');
      $submit_button_text = t("Force reinstall objects");
    }
    elseif ($needs_install) {
      $form['solution_pack']['install_status']['#markup'] .= ' ' . theme('image', array('path' => 'misc/watchdog-warning.png')) . ' ' . t('Some objects are missing and must be installed. See objects list for details.');
      $submit_button_text = t("Install objects");
    }
    elseif ($needs_update) {
      $form['solution_pack']['install_status']['#markup'] .= ' ' . theme('image', array('path' => 'misc/watchdog-warning.png')) . ' ' . t('Some objects must be reinstalled. See objects list for details.');
      $submit_button_text = t("Reinstall objects");
    }
    elseif ($could_not_connect) {
      $form['solution_pack']['install_status']['#markup'] .= ' ' . theme('image', array('path' => 'misc/watchdog-error.png')) . ' ' . t('Could not connect to the repository.');
      $submit_button_text = '';
    }

    $form['solution_pack']['table'] = array(
      '#type' => 'item',
      '#markup' => theme('table', array('header' => $table_header, 'rows' => $table_rows)),
    );
  }

  if (!$could_not_connect) {
    $form['solution_pack']['submit'] = array(
      '#value' => $submit_button_text,
      '#type' => 'submit',
      '#name' => $solution_pack_module,
      '#attributes' => array('class' => array('islandora-solution-pack-submit')),
      '#weight' => 40,
    );
    $form['solution_pack']['#submit'] = array('islandora_solution_pack_form_submit');
  }
  return $form;
}

/**
 * Submit handler for solution pack form.
 *
 * @param array $form
 *   The form submitted.
 * @param array_reference $form_state
 *   The state of the form submited.
 */
function islandora_solution_pack_form_submit($form, &$form_state) {
  $solution_pack_module = $form_state['values']['solution_pack_module'];
  $solution_pack_name = $form_state['values']['solution_pack_name'];
  $objects = $form_state['values']['objects'];

  $batch = array(
    'title' => t('Installing / updating solution pack objects'),
    'file' => drupal_get_path('module', 'islandora') . '/includes/solution_packs.inc',
    'operations' => array(),
  );

  foreach ($objects as $object) {
    // Add this object to the batch job queue.
    $batch['operations'][] = array('islandora_batch_reingest_object', array($object));
  }

  batch_set($batch);

  // Hook to let solution pack objects be modified.
  // Not using module_invoke so solution packs can be expanded by other modules.
  module_invoke_all('islandora_postprocess_solution_pack', $solution_pack_module);

}

/**
 * Batch reingest object(s)
 *
 * @param array $object
 * @param type $context
 * @return type
 */
function islandora_batch_reingest_object($object_model, &$context) {
  module_load_include('inc', 'islandora', 'includes/utilities');

  global $user;
  global $base_url;
  // new connection
  try {
    $connection = new IslandoraTuque($user);
  }
  catch (Exception $e) {
    drupal_set_message(t('Unable to connect to the repository %e', array('%e' => $e)), 'error');
    return;
  }

  if (!empty($object_model) && is_array($object_model)) {
    $pid = $object_model['pid'];
    if (!islandora_is_valid_pid($pid)) {
      return NULL;
    }

    // purge object
    // check if object already exits
    $object_query = $connection->api->a->findObjects('query', 'pid=' . $pid);
    $reinstall = FALSE;
    if (!empty($object_query['results'])) {
      $object = islandora_object_load($pid);
      if (isset($object)) {
        islandora_delete_object($object);
      }
      $reinstall = TRUE;
    }

    // build and ingest new object
    try {
      $object = islandora_solution_pack_add_object($object_model);
      $object_name = $object->label;
      if ($reinstall) {
        drupal_set_message(t('Successfully reinstalled <em>@object_name</em>.', array('@object_name' => $object_name, '@pid' => $pid)));
      }
      else {
        drupal_set_message(t('Successfully installed <em>@object_name</em>.', array('@object_name' => $object_name, '@pid' => $pid)));
      }
    }
    catch (Exception $e) {
      drupal_set_message(t('Installation of object @pid failed', array('@pid' => $pid)), 'error');
    }
  }
}


/**
 * Callback function that can be called from the solution pack's hook_install() and hook_uninstall() functions.
 *
 * @TODO: add documentation
 */
function islandora_install_solution_pack($module_name = NULL, $op = 'install') {
  // check if a module name is given. // @TODO: check module name for existance
  if (!empty($module_name)) {
    module_load_include('module', 'islandora', 'islandora');
    module_load_include('inc', 'islandora', 'includes/utilities');
    module_load_include('module', $module_name, $module_name);

    // set globals
    global $base_url;
    global $user;

    // set variables
    $sp_admin = url($base_url . '/admin/islandora/solution_packs');
    $config_url = url('admin/islandora/configure');

    // get module info
    $info_file = drupal_get_path('module', $module_name) . '/' . $module_name . '.info';
    $info_array = drupal_parse_info_file($info_file);
    $module_label = $info_array['name'];

    // check connection
    $url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
    $info = islandora_describe_repository($url);
    if (!$info) {
      // operation
      switch ($op) {
        case 'install':
          drupal_set_message(st('@module_label: Did not install any objects. Could not connect to the repository. Please check the settings on the <a href="@config_url" title="Islandora configuration page">Islandora configuration</a> page and install the required objects manually on the <a href="@sp_url" title="Solution pack admin">solution pack admin</a> page.', array('@module_label' => $module_label, '@config_url' => $config_url, '@sp_url' => $sp_admin)), 'error');
          break;

        case 'uninstall':
          drupal_set_message(st('@module_label: Did not uninstall any objects. Could not connect to the repository. Please check the settings on the <a href="@config_url" title="Islandora configuration page">Islandora configuration</a> page and uninstall the required objects manually if necessary.', array('@module_label' => $module_label, '@config_url' => $config_url)), 'error');
          break;
      }
      return;
    }

    // create new connection
    try {
      $connection = new IslandoraTuque($user);
    }
    catch (Exception $e) {
      drupal_set_message(st('Unable to connect to the repository %e', array('%e' => $e)), 'error');
      return;
    }

    // get object models
    $enabled_solution_packs = module_invoke_all('islandora_required_objects');
    $islandora_required_objects = $module_name . '_islandora_required_objects';
    $required_objects = $islandora_required_objects();
    $objects = $required_objects[$module_name]['objects'];

    // loop over object models
    foreach ($objects as $object) {
      // set variables
      $pid = $object['pid'];
      $label = isset($object['label']) ? $object['label'] : st('Object');
      // check if object already exists
      $query = $connection->api->a->findObjects('query', 'pid=' . $pid);
      // object url
      $object_url = url($base_url . '/islandora/object/' . $pid);

      // operation: install or uninstall
      switch ($op) {
        case 'install':
          // if object exists, don't re-ingest
          if (!empty($query['results'])) {
            // check object status
            $object_status = islandora_check_object_status($object);
            // set messages
            switch ($object_status) {
              case 'up_to_date':
                drupal_set_message(st('@module_label: did not install <a href="@object_url" title="@pid">@label</a>. The object already exists and is up-to-date.', array('@module_label' => $module_label, '@label' => $label, '@pid' => $pid, '@object_url' => $object_url)));
                break;
              case 'missing_datastream':
                drupal_set_message(st('@module_label: did not install <a href="@object_url" title="@pid">@label</a>. The object already exists but is missing a datastream. Please reinstall the object on the <a href="@sp_admin" title="Solution pack admin page">solution pack admin page</a>.', array('@module_label' => $module_label, '@label' => $label, '@pid' => $pid, '@objecturl' => $object_url, '@sp_admin' => $sp_admin)), 'warning');
                break;
              case 'out_of_date':
                drupal_set_message(st('@module_label: did not install <a href="@object_url" title="@pid">@label</a>. The object already exists but is out-of-date. Please update the object on the <a href="@sp_admin" title="Solution pack admin page">solution pack admin page</a>.', array('@module_label' => $module_label, '@label' => $label, '@pid' => $pid, '@object_url' => $object_url, '@sp_admin' => $sp_admin)), 'warning');
                break;
            }
          }
          else {
            islandora_solution_pack_add_object($object);
            drupal_set_message(st('@module_label: installed <a href="@object_url" title="@pid">@label</a> object.', array('@module_label' => $module_label, '@label' => $label, '@pid' => $pid, '@object_url' => $object_url)));
          }
          break;

        case 'uninstall':
          // if object exists, set message
          if (!empty($query['results'])) {
            $object_url = url($base_url . '/islandora/object/' . $pid);
            drupal_set_message(st('@module_label: did not remove <a href="@object_url" title="@pid">@label</a>. It may be used by other sites.', array('@pid' => $pid, '@object_url' => $object_url, '@label' => $label, '@module_label' => $module_label)), 'warning');
          }
          break;
      }
    }
  }
}


/**
 * Function to check the status of an object against an object model array.
 *
 * @param array $object_model
 *   an array describing an object
 * @return string
 *   Returns one of the following values:
 *   up_to_date, missing, missing_datastream or out_of_date
 *   You can perform an appropriate action based on this value.
 *   Returns FALSE if the array is empty
 *
 * @see islandora_solution_pack_form()
 * @see islandora_install_solution_pack()
 * @todo: Should this function live in islandora.module so it can be called
 *   easier without having to include the solution_packs.inc file?
 */
function islandora_check_object_status($object_model = array()) {
  if (!empty($object_model)) {
    // set variables
    $pid = $object_model['pid'];
    $object_status = 'up_to_date';

    // table row
    $table_row = array();

    // check connection
    module_load_include('inc', 'islandora', 'includes/utilities');
    $url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
    $info = islandora_describe_repository($url);
    if (!$info) {
      $object_status = 'could_not_connect';
    }
    else {

      // load object
      $object = islandora_object_load($pid);
      // check if object exists
      if (!$object) {
        $object_status = 'missing';
      }
      else {
        // object defined with single datastream file
        // @TODO: should dsversion be mandatory for the check to valid?
        if (isset($object_model['dsid']) && isset($object_model['datastream_file']) && isset($object_model['dsversion'])) {
          $datastreams = array(
            array(
              'dsid' => $object_model['dsid'],
              'datastream_file' => $object_model['datastream_file'],
              'dsversion' => $object_model['dsversion'],
            ),
          );
        }
        // object defined with multiple datastreams (using an array)
        elseif (!empty($object_model['datastreams'])) {
          $datastreams = $object_model['datastreams'];
        }
        if (!empty($datastreams) && is_array($datastreams)) {
          // loop over defined datastreams
          foreach ($datastreams as $ds) {
            $ds_id = $ds['dsid'];
            // check if defined datastream exists in the object
            if (!$object[$ds_id]) {
              $object_status = 'missing_datastream';
              break;
            }
            elseif (isset($ds['dsversion'])) {
              // Check if the datastream is versioned and needs updating.
              $installed_version = islandora_get_islandora_datastream_version($object, $ds['dsid']);
              $available_version = islandora_get_islandora_datastream_version(NULL, NULL, $ds['datastream_file']);

              if ($available_version > $installed_version) {
                $object_status = 'out_of_date';
                break;
              }
            }
          }
        }
      }
    }
    return $object_status;
  }
  else {
    return FALSE;
  }
}

/**
 * Converts the given definition into an object and add's it to the repository.
 *
 * @param array $object_definition
 *   An associative array containing the necessary parameters to create the
 *   desired object:
 *   - pid: The PID with which the object will be created.
 *   - label: An optional label to apply to the object.
 *   - datastreams: Same as the "datastreams" array accepted by
 *     islandora_prepare_new_object().
 *   - cmodel: Either an array of content models as accepted by
 *     islandora_preprare_new_object(), or a single content model PID to add
 *     to the object.
 *   - parent: Either an array of parents, or a single parent PID to which to
 *     relate to; uses isMemberOfCollection by default.
 *   - relationships: An array of relationships as accepted by
 *     islandora_prepare_new_object().
 *
 * @return FedoraObject
 *   The newly created object.
 */
function islandora_solution_pack_add_object(array $object_definition) {
  $object = islandora_solution_pack_prepare_new_object($object_definition);
  return islandora_add_object($object);
}

/**
 * Prepares a new object based on the solution pack style of declaring them as arrays.
 *
 * @param array $object_definition
 *   An associative array containing the necessary parameters to create the
 *   desired object:
 *   - pid: The PID with which the object will be created.
 *   - label: An optional label to apply to the object.
 *   - datastreams: Same as the "datastreams" array accepted by
 *     islandora_prepare_new_object().
 *   - cmodel: Either an array of content models as accepted by
 *     islandora_prepare_new_object(), or a single content model PID to add
 *     to the object.
 *   - parent: Either an array of parents, or a single parent PID to which to
 *     relate to; uses isMemberOfCollection by default.
 *   - relationships: An array of relationships as accepted by
 *     islandora_prepare_new_object().
 *
 * @return NewFedoraObject
 *   An NewFedoraObject which has been initalized with the given properties.
 */
function islandora_solution_pack_prepare_new_object(array $object_definition) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $namespace = $object_definition['pid'];
  $label = !empty($object_definition['label']) ? $object_definition['label'] : NULL;
  $datastreams = array();
  if (!empty($object_definition['datastreams']) AND is_array($object_definition['datastreams'])) {
    $datastreams = $object_definition['datastreams'];
  }
  $content_models = array();
  if (!empty($object_definition['cmodel'])) {
    if (is_array($object_definition['cmodel'])) {
      $content_models = $object_definition['cmodel'];
    }
    else {
      $content_models[] = $object_definition['cmodel'];
    }
  }
  $relationships = array();
  if (!empty($object_definition['parent']) AND !is_array($object_definition['parent'])) {
    $relationships[] = array('relationship' => 'isMemberOfCollection', 'pid' => $object_definition['parent']);
  }
  if (!empty($object_definition['parents']) AND is_array($object_definition['parents'])) {
    foreach ($object_definition['parents'] as $parent) {
      $relationships[] = array('relationship' => 'isMemberOfCollection', 'pid' => $parent);
    }
  }
  if (!empty($object_definition['relationships']) AND is_array($object_definition['relationships'])) {
    foreach ($object_definition['relationships'] as $relationship) {
      $relationships[] = array('relationship' => $relationship['relationship'], 'pid' => $relationship['pid']);
    }
  }
  return islandora_prepare_new_object($namespace, $label, $datastreams, $content_models, $relationships);
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
