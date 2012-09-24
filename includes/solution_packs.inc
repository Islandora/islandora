<?php

/**
 * @file
 * This file contains all admin and callback functions for solution pack management.
 */

/**
 * Solution pack admin page callback
 */
function islandora_solution_packs_admin() {
  // check connection
  module_load_include('inc', 'islandora', 'includes/utilities');
  $url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
  $info = islandora_describe_repository($url);
  if (!$info) {
    $config_url = url('admin/islandora/configure');
    drupal_set_message(t('Could not connect to the repository. Please check the settings on the <a href="@config_url" title="Islandora configuration page">Islandora configuration</a> page.', array('@config_url' => $config_url)), 'error');
  }

  // set variables
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
    // get form
    $form_array = drupal_get_form('islandora_solution_pack_form_' . $solution_pack_module, $solution_pack_module, $solution_pack_name, $objects);
    // render form
    $output .= drupal_render($form_array);
  }

  return $output;
}

/**
 * Solution pack admin page
 */
function islandora_solution_pack_form($form, &$form_state,  $solution_pack_module, $solution_pack_name, $objects = array()) {

  // set variables
  global $base_url;
  global $base_path;
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

  // table
  // header
  $table_header = array(t('Label'), t('PID'), t('Status'));
  $table_rows = array();

  // loop over defined objects
  foreach ($objects as $object) {
    $datastreams = NULL;
    if (isset($object['pid'])) {
      // set variables
      $pid = $object['pid'];

      // table row
      $table_row = array();

      // check object status
      $object_status = islandora_check_object_status($object);

      // set status labels
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

      // label
      if ($needs_install OR $could_not_connect) {
        $label = $object['label'] ? $object['label'] : '';
      }
      else {
        $label = $object['label'] ? l($object['label'], $base_url . '/islandora/object/' . $pid) : '';
      }
      $table_row[] = $label;
      // pid
      $table_row[] = $pid;
      // object status
      $table_row[] = $object_status;
      // add row
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

    // install status
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

    // table
    $form['solution_pack']['table'] = array(
      '#type' => 'item',
      '#markup' => theme('table', array('header' => $table_header, 'rows' => $table_rows)),
    );
  }

  // submit
  if (!$could_not_connect) {
    $form['solution_pack']['submit'] = array(
      '#value' => $submit_button_text,
      '#type' => 'submit',
      '#name' => $solution_pack_module,
      '#attributes' => array('class' => array('islandora-solution-pack-submit')),
      '#weight' => 40,
    );
    // submit callback
    $form['solution_pack']['#submit'] = array(
      'islandora_solution_pack_form_submit',
    );
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

  // get variables
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
 * @param type $object
 * @param type $context
 * @return type
 */
function islandora_batch_reingest_object($object_model, &$context) {

  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/islandora.ingest');
  // include Tuque library
  module_load_include('inc', 'islandora', 'includes/tuque');
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
    // set and validate PID
    $pid = $object_model['pid'];
    if (!islandora_validate_pid($pid)) {
      return NULL;
    }

    // purge object
    // check if object already exits
    $object_query = $connection->api->a->findObjects('query', 'pid=' . $pid);
    $reinstall = FALSE;
    if (!empty($object_query['results'])) {
      islandora_object_purge($pid);
      $reinstall = TRUE;
    }

    // build and ingest new object
    try {
      $object = islandora_ingest_new_object($object_model);
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

    // include files
    module_load_include('inc', 'islandora', 'includes/tuque');
    module_load_include('module', 'islandora', 'islandora');
    module_load_include('inc', 'islandora', 'includes/islandora.ingest');
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
            // build and ingest new object
            islandora_ingest_new_object($object);
            // set message
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
 * @TODO: should this function live in islandora.module so it can be called easier without having to include the solution_packs.inc file?
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
        // object defined with single datastream file // @TODO: should dsversion be mandatory for the check to valid?
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



