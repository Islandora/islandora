<?php

/**
 * @file
 * Drush command/hook implementation.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_drush_command() {
  $commands = array();

  $commands['islandora-solution-pack-install-required-objects'] = array(
    'description' => dt('Install Solution Pack objects.'),
    'options' => array(
      'module' => array(
        'description' => dt('The module for which to install the required objects.'),
        'required' => TRUE,
      ),
      'force' => array(
        'description' => dt('Force reinstallation of the objects.'),
      ),
    ),
    'aliases' => array('ispiro'),
    'drupal dependencies' => array(
      'islandora',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $commands['islandora-solution-pack-uninstall-required-objects'] = array(
    'description' => dt('Uninstall Solution Pack objects.'),
    'options' => array(
      'module' => array(
        'description' => dt('The module for which to uninstall the required objects.'),
        'required' => TRUE,
      ),
      'force' => array(
        'description' => dt('Force reinstallation of the objects.'),
      ),
    ),
    'aliases' => array('ispuro'),
    'drupal dependencies' => array(
      'islandora',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $commands['islandora-solution-pack-required-objects-status'] = array(
    'description' => dt('Get Solution Pack object status.'),
    'options' => array(
      'module' => array(
        'description' => dt('The module for which to get the status of the required objects.'),
      ),
    ),
    'aliases' => array('ispros'),
    'drupal dependencies' => array(
      'islandora',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  return $commands;
}

/**
 * Command callback to install required objects.
 */
function drush_islandora_solution_pack_install_required_objects() {
  module_load_include('inc', 'islandora', 'includes/solution_packs');

  $module = drush_get_option('module');
  if (module_exists($module)) {
    islandora_install_solution_pack(
      $module,
      'install',
      drush_get_option('force', FALSE)
    );
  }
  else {
    drush_log(dt('"@module" is not installed/enabled?...', array(
      '@module' => $module,
    )));
  }
}

/**
 * Command callback to uninstall required objects.
 */
function drush_islandora_solution_pack_uninstall_required_objects() {
  module_load_include('inc', 'islandora', 'includes/solution_packs');

  $module = drush_get_option('module');
  if (module_exists($module)) {
    islandora_uninstall_solution_pack(
      $module,
      drush_get_option('force', FALSE)
    );
  }
  else {
    drush_log(dt('"@module" is not installed/enabled?...', array(
      '@module' => $module,
    )));
  }
}

/**
 * Command callback for object status.
 */
function drush_islandora_solution_pack_required_objects_status() {
  module_load_include('inc', 'islandora', 'includes/solution_packs');

  $module = drush_get_option('module', FALSE);
  $required_objects = array();
  if ($module && module_exists($module)) {
    $required_objects[$module] = islandora_solution_packs_get_required_objects($module);
  }
  elseif ($module === FALSE) {
    $required_objects = islandora_solution_packs_get_required_objects();
  }
  else {
    drush_log(dt('"@module" is not installed/enabled?...', array(
      '@module' => $module,
    )));
    return;
  }

  $header = array('PID', 'Machine Status', 'Readable Status');
  $widths = array(30, 20, 20);
  foreach ($required_objects as $module => $info) {
    $rows = array();
    drush_print($info['title']);
    foreach ($info['objects'] as $object) {
      $status = islandora_check_object_status($object);
      $rows[] = array(
        $object->id,
        $status['status'],
        $status['status_friendly'],
      );
    }
    drush_print_table($rows, $header, $widths);
  }
}
