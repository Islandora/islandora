<?php

/**
 * @file
 * islandora.admin.inc: This file contains the general islandora admin form and callback functions.
 */

/**
 * Create admin form
 * @return array
 */
function islandora_repository_admin($form, &$form_state) {
  module_load_include('inc', 'islandora', 'includes/tuque');
  module_load_include('inc', 'islandora', 'includes/utilities');

  if (!IslandoraTuque::exists()) {
    IslandoraTuque::getError();
    return;
  }

  $form = array();

  if (isset($form_state['values']['islandora_base_url'])) {
    $url = $form_state['values']['islandora_base_url'];
  }
  else {
    $url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
  }

  module_load_include('inc', 'islandora', 'includes/tuque');
  $connection = new IslandoraTuque(NULL, $url);
  try {
    $info = $connection->api->a->describeRepository();
  }
  catch (RepositoryException $e) {
    $info = FALSE;
  }

  if ($info) {
    try {
      $dc = $connection->api->m->getDatastream('fedora-system:ContentModel-3.0', 'DC');
    }
    catch (RepositoryException $e) {
      $dc = FALSE;
    }
  }

  if ($info) {
    if ($dc) {
      $confirmation_message = '<img src="' . url('misc/watchdog-ok.png') . '"/>'
        . t('Successfully connected to Fedora Server (Version !version).', array('!version' => $info['repositoryVersion']));
    }
    else {
      $confirmation_message = '<img src="' . url('misc/watchdog-warning.png') . '"/>'
        . t('Unable to authenticate when connecting to to Fedora Server (Version !version). Please configure the !filter.', array('!version' => $info['repositoryVersion'], '!filter' => 'Drupal Filter'));
    }
  }
  else {
    $confirmation_message = '<img src="' . url('misc/watchdog-error.png') . '"/> '
      . t('Unable to connect to Fedora server at !islandora_url', array('!islandora_url' => $url));
  }

  $form['islandora_tabs'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['islandora_tabs']['islandora_general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General Configuarion'),
  );

  // ajax wrapper for url checking
  $form['islandora_tabs']['islandora_general']['wrapper'] = array(
    '#prefix' => '<div id="islandora-url">',
    '#suffix' => '</div>',
    '#type' => 'markup',
  );

  $form['islandora_tabs']['islandora_general']['wrapper']['islandora_base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Fedora base URL'),
    '#default_value' => variable_get('islandora_base_url', 'http://localhost:8080/fedora'),
    '#description' => t('The URL to use for REST connections <br />' . $confirmation_message),
    '#required' => TRUE,
    '#ajax' => array(
      'callback' => 'islandora_update_url_div',
      'wrapper' => 'islandora-url',
      'effect' => 'fade',
      'event' => 'blur',
      'progress' => array('type' => 'throbber'),
    ),
  );

  $form['islandora_tabs']['islandora_general']['islandora_repository_pid'] = array(
    '#type' => 'textfield',
    '#title' => t('Root Collection PID'),
    '#default_value' => variable_get('islandora_repository_pid', 'islandora:root'),
    '#description' => t('The PID of the Root Collection Object'),
    '#required' => TRUE,
  );

  $form['islandora_tabs']['islandora_namespace'] = array(
    '#type' => 'fieldset',
    '#title' => t('Namespaces'),
  );

  $form['islandora_tabs']['islandora_namespace']['wrapper'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="islandora-namespace">',
    '#suffix' => '</div>',
  );

  $form['islandora_tabs']['islandora_namespace']['wrapper']['islandora_namespace_restriction_enforced'] = array(
    '#weight' => -1,
    '#type' => 'checkbox',
    '#title' => t('Enforce namespace restrictions'),
    '#description' => t('Allow administrator to restrict user\'s access to the PID namepaces listed below'),
    '#default_value' => variable_get('islandora_namespace_restriction_enforced', FALSE),
    '#ajax' => array(
      'callback' => 'islandora_update_namespace_div',
      'wrapper' => 'islandora-namespace',
      'effect' => 'fade',
      'event' => 'change',
      'progress' => array('type' => 'throbber'),
    ),
  );

  if (isset($form_state['values']['islandora_namespace_restriction_enforced'])) {
    $namespaces = $form_state['values']['islandora_namespace_restriction_enforced'];
  }
  else {
    $namespaces = variable_get('islandora_namespace_restriction_enforced', FALSE);
  }

  if ($namespaces) {
    $form['islandora_tabs']['islandora_namespace']['wrapper']['islandora_pids_allowed'] = array(
      '#type' => 'textfield',
      '#title' => t('PID namespaces allowed in this Drupal install'),
      '#default_value' => variable_get('islandora_pids_allowed', 'default: demo: changeme: ilives: islandora-book: books: newspapers: '),
      '#description' => t('A space separated list of PID namespaces that users are permitted to access from this Drupal installation. <br /> This could be more than a simple namespace ie demo:mydemos. <br /> islandora: is reserved and is always allowed.'),
      '#weight' => 0,
    );
  }

  return system_settings_form($form);
}

/**
 * Checks url validity and refreshes requestHandler dropdown list
 */
function islandora_update_url_div($form, $form_state) {
  unset($form_state['submit_handlers']);
  $form_state['rebuild'] = TRUE;

  return $form['islandora_tabs']['islandora_general']['wrapper'];
}

/**
 * Checks url validity and refreshes requestHandler dropdown list
 */
function islandora_update_namespace_div($form, $form_state) {
  unset($form_state['submit_handlers']);
  $form_state['rebuild'] = TRUE;

  return $form['islandora_tabs']['islandora_namespace']['wrapper'];
}