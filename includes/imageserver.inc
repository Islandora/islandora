<?php

/**
 * @file
 * Image server central configuration.
 */

/**
 * Image server configuration form.
 *
 * @param array $form
 *   Drupal form.
 * @param array $form_state
 *   Drupal form state.
 *
 * @return array
 *   Drupal form.
 *
 * @throws \Exception
 *   Thrown by theme() function.
 */
function islandora_imageserver_admin_form(array $form, array &$form_state) {
  form_load_include($form_state, 'inc', 'islandora', 'includes/utilities');
  $settings = islandora_imageserver_get_settings();

  $form = array(
    'type' => islandora_imageserver_get_type_selectbox(FALSE),
    'url' => array(
      '#prefix' => '<div id="islandora-imageserver-path-wrapper">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('Image Server Base URL'),
      '#title_display' => 'invisible',
      '#default_value' => $settings['url'],
      '#description' => t('The location of the image server. <br/> !confirmation_message', array(
        '!confirmation_message' => islandora_imageserver_admin_form_access_message($form_state),
      )),
      '#ajax' => array(
        'callback' => 'islandora_imageserver_admin_ajax_url',
        'wrapper' => 'islandora-imageserver-path-wrapper',
      ),
    ),
    'iiif' => array(
      '#type' => 'fieldset',
      '#title' => t('IIIF Image Server Settings'),
      '#description' => t('Settings for IIIF Image Server'),
      '#states' => array(
        'visible' => array(
          ':input[name="type"]' => array('value' => 'iiif'),
        ),
      ),
      'iiif_token_header' => array(
        '#type' => 'checkbox',
        '#title' => t('Add token as header'),
        '#default_value' => $settings['iiif_token_header'],
        '#description' => t('Instead of sending the token as a query parameter, it will be sent in the X-ISLANDORA-TOKEN header.'),
      ),
      'iiif_identifier' => array(
        '#type' => 'textfield',
        '#title' => t('IIIF Identifier'),
        '#default_value' => $settings['iiif_identifier'],
        '#element_validate' => array('token_element_validate'),
        '#token_types' => array('islandora'),
      ),
      'islandora_imageserver_iiif_token_tree' => array(
        '#type' => 'fieldset',
        '#title' => t('Replacement patterns'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#description' => theme('token_tree', array(
          'token_types' => array('islandora'),
          'global_types' => FALSE,
        )),
      ),
    ),
    'actions' => array(
      '#type' => 'actions',
      'save' => array(
        '#type' => 'submit',
        '#value' => t('Save configuration'),
        '#weight' => 0,
      ),
      'reset' => array(
        '#type' => 'submit',
        '#value' => t('Reset to defaults'),
        '#weight' => 1,
        '#submit' => array('islandora_imageserver_admin_submit_reset'),
      ),
    ),
  );
  return $form;
}

/**
 * Redirect to Reset form.
 *
 * @param array $form
 *   Drupal form.
 * @param array $form_state
 *   Drupal form state.
 */
function islandora_imageserver_admin_submit_reset(array $form, array &$form_state) {
  $form_state['redirect'] = 'admin/islandora/image_server/reset';
}

/**
 * Settings reset confirmation form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal $form_state.
 *
 * @return mixed
 *   The created form.
 */
function islandora_imageserver_admin_reset_confirm_form(array $form, array &$form_state) {
  return confirm_form(
    $form,
    t("Do you really want to reset the Image Server settings?"),
    'admin/islandora/image_server',
    t("This action cannot be undone."),
    t("Reset")
  );
}

/**
 * Implements hook_form_submit().
 */
function islandora_imageserver_admin_reset_confirm_form_submit(array $form, array &$form_state) {
  variable_del('islandora_imageserver_settings');
  drupal_set_message(t('Settings reset.'), 'status');
  $form_state['redirect'] = 'admin/islandora/image_server';
}

/**
 * Implements hook_form_submit().
 */
function islandora_imageserver_admin_form_submit(array $form, array &$form_state) {
  $type = $form_state['values']['type'];
  $settings = islandora_imageserver_get_settings();
  $settings['type'] = $type;
  $settings['url'] = rtrim($form_state['values']['url'], '/');
  if ($type == 'iiif') {
    $settings['iiif_token_header'] = (bool) $form_state['values']['iiif_token_header'];
    $settings['iiif_identifier'] = $form_state['values']['iiif_identifier'];
  }
  else {
    $settings['iiif_token_header'] = FALSE;
    $settings['iiif_identifier'] = ISLANDORA_IMAGESERVER_DEFAULT_TOKEN;
  }
  variable_set('islandora_imageserver_settings', $settings);
  drupal_set_message(t("Settings saved successfully."), 'status');
}

/**
 * Callback to check URL.
 *
 * @param array $form
 *   Drupal form.
 * @param array $form_state
 *   Drupal form state.
 *
 * @return mixed
 *   The form element.
 */
function islandora_imageserver_admin_ajax_url(array $form, array $form_state) {
  return $form['url'];
}

/**
 * Gets a message which describes if Adore-Djatoka is accessible.
 *
 * @param array $form_state
 *   The current form state.
 *
 * @see islandora_imageserver_get_settings()
 *
 * @return string
 *   A message describing the accessibility of the Adore-Djatoka image resolver.
 */
function islandora_imageserver_admin_form_access_message(array &$form_state) {
  $confirmation_message = '';

  $type = islandora_imageserver_get_default_value($form_state, 'type');
  $url = islandora_imageserver_get_default_value($form_state, 'url');

  if ($type == 'djatoka') {
    $url = url($url, array(
      'absolute' => TRUE,
      'query' => array(
        'url_ver' => 'Z39.88-2004',
        'rft_id' => 'http://memory.loc.gov/gmd/gmd433/g4330/g4330/np000066.jp2',
        'svc_id' => 'info:lanl-repo/svc/getRegion',
        'svc_val_fmt' => 'info:ofi/fmt:kev:mtx:jpeg2000',
        'svc.format' => 'image/jpeg',
        'svc.level' => '1',
      ),
    ));
  }
  elseif ($type == 'iiif') {
    $url = url(rtrim($url, '/'),
      array(
        'absolute' => TRUE,
      )
    );
  }
  else {
    // None, so don't confirm.
    return "";
  }

  if (isset($url) && !empty($url)) {
    $result = drupal_http_request($url);
    if ($result->code == 200) {
      $confirmation_message = theme_image(array(
        'path' => 'misc/watchdog-ok.png',
        'attributes' => array(),
      ));
      $confirmation_message .= t('Successfully connected to image server.');
    }
    else {
      $confirmation_message = theme_image(array(
        'path' => 'misc/watchdog-error.png',
        'attributes' => array(),
      ));
      $confirmation_message .= t('Unable to connect to image server at !path', array(
        '!path' => $url,
      ));
    }
  }
  return $confirmation_message;
}

/**
 * Get image server settings.
 *
 * @return array
 *   Configuration to access the image server.
 */
function islandora_imageserver_get_settings() {
  $defaults = islandora_imageserver_get_default_settings();

  $settings = variable_get('islandora_imageserver_settings', array()) + $defaults;

  return $settings;
}

/**
 * Return the default image server settings.
 *
 * @return array
 *   The settings.
 */
function islandora_imageserver_get_default_settings() {
  return array(
    'type' => 'none',
    'url' => '',
    'iiif_token_header' => FALSE,
    'iiif_identifier' => ISLANDORA_IMAGESERVER_DEFAULT_TOKEN,
  );
}

/**
 * Utility to check form_state or return the default.
 *
 * @param array $form_state
 *   Drupal form state.
 * @param string $name
 *   Name of the form element/settings key.
 *
 * @return mixed
 *   The Form value or currently saved value.
 */
function islandora_imageserver_get_default_value(array &$form_state, $name) {
  $settings = islandora_imageserver_get_settings();
  return isset($form_state['values'][$name]) ? $form_state['values'][$name] : $settings[$name];
}

/**
 * Generate a replacement string using tokens.
 *
 * @param string $string_token
 *   The replacement token to generate.
 * @param string $pid
 *   The pid of the object.
 * @param string $dsid
 *   The dsid to return.
 * @param string $authtoken
 *   The authentication token.
 *
 * @return mixed
 *   The token replaced string.
 */
function islandora_imageserver_get_identifier($string_token, $pid, $dsid, $authtoken) {
  $settings = islandora_imageserver_get_settings();
  if ($settings['type'] == 'djatoka') {
    // We use the token_replace to generate the rft_id.
    $string_token = '[islandora:url_token]';
  }
  $parts = array(
    'islandora' => array(
      'pid' => $pid,
      'dsid' => $dsid,
      'token' => $authtoken,
    ),
  );
  return token_replace($string_token, $parts);
}

/**
 * Create the select box form element with the currently configured type.
 *
 * @param bool $disabled
 *   Whether the element should be disabled.
 *
 * @return array
 *   The form element.
 */
function islandora_imageserver_get_type_selectbox($disabled = TRUE) {
  $settings = islandora_imageserver_get_settings();
  $description = $disabled ? t("Configured at Admin ≫ Islandora ≫ Image server configuration.") : t("Select the image server to configure.");
  return array(
    '#type' => 'select',
    '#title' => t('Image Server'),
    '#description' => $description,
    '#default_value' => $settings['type'],
    '#disabled' => $disabled,
    '#options' => array(
      'none' => t('No image server configured'),
      'iiif' => t('IIIF image server'),
      'djatoka' => t('Adore-Djatoka image server'),
    ),
  );
}
