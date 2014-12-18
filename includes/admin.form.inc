<?php

/**
 * @file
 * This administration form for this module.
 */

/**
 * The administration form for the fedora repository connection.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_repository_admin(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  drupal_add_css(drupal_get_path('module', 'islandora') . '/css/islandora.admin.css');
  $url = islandora_system_settings_form_default_value('islandora_base_url', 'http://localhost:8080/fedora', $form_state);
  $restrict_namespaces = islandora_system_settings_form_default_value('islandora_namespace_restriction_enforced', FALSE, $form_state);
  $confirmation_message = islandora_admin_settings_form_repository_access_message($url);
  $form = array(
    'islandora_tabs' => array(
      '#type' => 'vertical_tabs',
      'islandora_general' => array(
        '#type' => 'fieldset',
        '#title' => t('General Configuration'),
        'wrapper' => array(
          '#prefix' => '<div id="islandora-url">',
          '#suffix' => '</div>',
          '#type' => 'markup',
          'islandora_base_url' => array(
            '#type' => 'textfield',
            '#title' => t('Fedora base URL'),
            '#default_value' => variable_get('islandora_base_url', 'http://localhost:8080/fedora'),
            '#description' => t('The URL to use for REST connections <br/> !confirmation_message', array(
                              '!confirmation_message' => $confirmation_message)),
            '#required' => TRUE,
            '#ajax' => array(
              'callback' => 'islandora_update_url_div',
              'wrapper' => 'islandora-url',
              'effect' => 'fade',
              'event' => 'blur',
              'progress' => array('type' => 'throbber'),
            ),
          ),
        ),
        'islandora_repository_pid' => array(
          '#type' => 'textfield',
          '#title' => t('Root Collection PID'),
          '#default_value' => variable_get('islandora_repository_pid', 'islandora:root'),
          '#description' => t('The PID of the Root Collection Object'),
          '#required' => TRUE,
        ),
        'islandora_use_datastream_cache_headers' => array(
          '#type' => 'checkbox',
          '#title' => t('Generate/parse datastream HTTP cache headers'),
          '#description' => t('HTTP caching can reduce network traffic, by allowing clients to used cached copies.'),
          '#default_value' => variable_get('islandora_use_datastream_cache_headers', TRUE),
        ),
        'islandora_defer_derivatives_on_ingest' => array(
          '#type' => 'checkbox',
          '#title' => t('Defer derivative generation during ingest'),
          '#description' => t('Prevent derivatives from running during ingest,
            useful if derivatives are to be created by an external service.'),
          '#default_value' => variable_get('islandora_defer_derivatives_on_ingest', FALSE),
        ),
        'islandora_show_print_option' => array(
          '#type' => 'checkbox',
          '#title' => t('Show print option on objects'),
          '#description' => t('Displays an extra print tab, allowing an object to be printed'),
          '#default_value' => variable_get('islandora_show_print_option', FALSE),
        ),
      ),
      'islandora_namespace' => array(
        '#type' => 'fieldset',
        '#title' => t('Namespaces'),
        'islandora_namespace_restriction_enforced' => array(
          '#type' => 'checkbox',
          '#title' => t('Enforce namespace restrictions'),
          '#description' => t("Allow administrator to restrict user's access to the PID namepaces listed below"),
          '#default_value' => $restrict_namespaces,
        ),
        'islandora_pids_allowed' => array(
          '#type' => 'textarea',
          '#title' => t('PID namespaces allowed in this Drupal install'),
          '#description' => t('A list of PID namespaces, separated by spaces, that users are permitted to access from this Drupal installation. <br /> This could be more than a simple namespace, e.g. <b>demo:mydemos</b>. <br /> The namespace <b>islandora:</b> is reserved, and is always allowed.'),
          '#default_value' => variable_get('islandora_pids_allowed', 'default: demo: changeme: ilives: islandora-book: books: newspapers: '),
          '#states' => array(
            'invisible' => array(
              ':input[name="islandora_namespace_restriction_enforced"]' => array('checked' => FALSE),
            ),
          ),
        ),
      ),
      'islandora_ds_replace_exclude' => array(
        '#type' => 'fieldset',
        '#title' => t('Excluded DSID'),
        'islandora_ds_replace_exclude_enforced' => array(
          '#type' => 'textfield',
          '#title' => t('Enforce DSID restrictions'),
          '#description' => t("A comma seperated list, allowing administrator to restrict user's access to replace a versionable datastreams latest version"),
          '#default_value' => variable_get('islandora_ds_replace_exclude_enforced', 'RELS-EXT,RELS-INT'),
        ),
      ),
    ),
  );
  return system_settings_form($form);
}

/**
 * Gets a message which describes if the repository is accessible.
 *
 * Also describes if the user is considered an authenticated user by the
 * repository.
 *
 * @param string $url
 *   The url to the Fedora Repository.
 *
 * @return string
 *   A message describing the accessibility of the repository at the given url.
 */
function islandora_admin_settings_form_repository_access_message($url) {
  $info = $dc = FALSE;
  $connection = islandora_get_tuque_connection(NULL, $url);
  if ($connection) {
    try {
      $info = $connection->api->a->describeRepository();
      // If we are able to successfully call API-M::getDatastream, assume we are
      // an authenticated user, as API-M is usally locked down.
      $dc = $connection->api->m->getDatastream('fedora-system:ContentModel-3.0', 'DC');
    }
    catch (RepositoryException $e) {
      // Ignore, we only testing to see what is accessible.
    }
  }
  if ($info && $dc) {
    $confirmation_message = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
    $confirmation_message .= t('Successfully connected to Fedora Server (Version !version).', array(
                               '!version' => $info['repositoryVersion']));
  }
  elseif ($info) {
    $confirmation_message = theme_image(array('path' => 'misc/watchdog-warning.png', 'attributes' => array()));
    $confirmation_message .= t('Unable to authenticate when connecting to to Fedora Server (Version !version). Please configure the !filter.', array(
                               '!version' => $info['repositoryVersion'], '!filter' => 'Drupal Filter'));
  }
  else {
    $confirmation_message = theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array()));
    $confirmation_message .= t('Unable to connect to Fedora server at !islandora_url', array(
                               '!islandora_url' => $url));
  }
  return $confirmation_message;
}

/**
 * Get the element to render for the AJAX event that triggered this callback.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The form element to render.
 */
function islandora_update_url_div(array $form, array $form_state) {
  return $form['islandora_tabs']['islandora_general']['wrapper'];
}
