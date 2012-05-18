<?php

/**
 * @file islandora.admin.inc
 * Create admin form
 * @return array 
 */

  function islandora_repository_admin() {
    if (!user_access('administer site configuration')) {
      drupal_set_message(t('You must be a site administrator to edit the Fedora collections list.'), 'error');
      return;
    }
   
    $form = array();
    $form['islandora_repository_pid'] = array(
      '#type' => 'textfield',
      '#title' => t('Root Collection PID'),
      '#default_value' => variable_get('islandora_repository_pid', 'islandora:root'),
      '#description' => t('The PID of the Root Collection Object'),
      '#required' => TRUE,
      '#weight' => -18
    );

    $form['islandora_base_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora base URL'),
      '#default_value' => variable_get('islandora_base_url', 'http://localhost:8080/fedora'),
      '#description' => t('The URL to use for REST-type connections'),
      '#required' => TRUE,
      '#weight' => -16,
    );
    

    $form['islandora_repository_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora RISearch URL'),
      '#default_value' => variable_get('islandora_repository_url', 'http://localhost:8080/fedora/risearch'),
      '#description' => t('The url of the Fedora server'), '#required' => TRUE,
      '#weight' => -14
    );
    
    $form['islandora_soap_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora SOAP Url'),
      '#default_value' => variable_get('islandora_soap_url', 'http://localhost:8080/fedora/services/access?wsdl'),
      '#description' => t('The URL to use for SOAP connections'),
      '#required' => TRUE,
      '#weight' => -12,
      );
    
    $form['islandora_soap_manage_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora SOAP management URL'),
      '#default_value' => variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/wsdl?api=API-M'), '#description' => t('The URL to use for SOAP API-M connections'),
      '#required' => TRUE,
      '#weight' => -10
    );
        
// will allow admin user to remove namepsace restrictions if not explicitly disallowed  in settings.php
    if (variable_get('allow_open_namespace', TRUE)) {
      $form['islandora_namespace'] = array(
        '#type' => 'fieldset',
      );

      $form['islandora_namespace']['islandora_namespace_restriction_enforced'] = array(
        '#weight' => -1,
        '#type' => 'radios',
        '#title' => t('Enforce namespace restrictions'),
        '#options' => array(
          TRUE => t('Enabled'),
          FALSE => t('Disabled')
        ),
        '#description' => t('Allow administrator to restrict user\'s access to the PID namepaces listed below'),
        '#default_value' => variable_get('islandora_namespace_restriction_enforced', TRUE)
      );

      $form['islandora_namespace']['fedora_pids_allowed'] = array(
        '#type' => 'textfield',
        '#title' => t('PID namespaces allowed in this Drupal install'),
        '#default_value' => variable_get('islandora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '),
        '#description' => t('A space separated list of PID namespaces that users are permitted to access from this Drupal installation. <br /> This could be more than a simple namespace ie demo:mydemos.'),
        '#weight' => 0
      );
    }
    else {
      $form['islandora_pids_allowed'] = array(
        '#type' => 'textfield',
        '#title' => t('PID namespaces allowed in this Drupal install'),
        '#default_value' => variable_get('islandora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '),
        '#description' => t('A space separated list of PID namespaces that users are permitted to access from this Drupal installation. <br /> This could be more than a simple namespace ie demo:mydemos.'),
        '#weight' => 0
      );
    }

    //have tabs options (like disable)
    $form['tabs'] = array(
      '#type' => 'fieldset',
      '#title' => t('Tabs Configuration'),
      '#description' => t('Configure the tabs avaialble when viewing Fedora objects.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    //when checked show object details tab
    $form['tabs']['islandora_repository_show_object_details_tab'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show Object Details Tab'),
      '#default_value' => variable_get('islandora_repository_show_object_details_tab', TRUE),
      '#description' => t("When enabled, the 'Object Details' tab will be visible to users with the correct permissions when viewing an object in the repository"),
    );
    
    $form['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced configuration options'),
      '#description' => t('Advanced configuration.  Under normal circumstances these will not be touched'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    
   

    $form['advanced']['islandora_object_restrict_datastreams'] = array(
      '#type' => 'checkbox',
      '#title' => t('Restrict Access to Fedora Object Datastreams'),
      '#default_value' => variable_get('islandora_object_restrict_datastreams', FALSE),
      '#description' => t('When enabled, restricts access to fedora object datastreams that are not listed in the Islandora Content Model for the object (unless the user is an administrator).'),
    );

    

    //Export functionality
    $form['advanced']['module']['islandora_export_area'] = array(
      '#type' => 'textfield',
      '#title' => t('Export area'),
      '#default_value' => variable_get('islandora_export_area', file_directory_path() . '/fedora_export_area'),
      '#description' => t("Path to the export area. It must be accessible by druapl (i.e. apache user)."),
      '#required' => TRUE,
    );

    $form['#attributes'] = array('enctype' => "multipart/form-data");


    return system_settings_form($form);
  }