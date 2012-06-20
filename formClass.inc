<?php



/**
 * @file
 * formClass class
 */

/**
 * form Class
 */
class formClass {

  function formClass() {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  /**
   * Create the paths for urls and map them to php functions
   * @return array 
   */
  function createMenu() {
    $items = array();

    $items['admin/settings/fedora_repository'] = array(
      'title' => t('Islandora Configuration'),
      'description' => t("Configure Islandora's interaction with Fedora"),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('fedora_repository_admin'),
      'access arguments' => array('administer site configuration'),
      'type' => MENU_NORMAL_ITEM,
    );
    $items['admin/settings/fedora_repository/collection'] = array(
      'title' => t('Collection list'),
      'description' => t('Enter the Islandora collection information here.'),
      'access arguments' => array('administer site configuration'),
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => 0,
    );

    $items['admin/settings/fedora_repository/demoobjects'] = array(
      'title' => t('Solution Packs'),
      'description' => t('Install content models and collections required by installed solution packs.'),
      'page callback' => 'fedora_repository_solution_packs_page',
      'access arguments' => array('add fedora datastreams'),
      'file' => 'fedora_repository.solutionpacks.inc',
      'type' => MENU_LOCAL_TASK,
    );
    $items['islandoracm.xsd'] = array(
      'title' => t('Islandora Content Model XML Schema Definition'),
      'page callback' => 'fedora_repository_display_schema',
      'page arguments' => array('islandoracm.xsd'),
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection'),
    );

    $items['collection_policy.xsd'] = array(
      'title' => t('Islandora Content Model XML Schema Definition'),
      'page callback' => 'fedora_repository_display_schema',
      'page arguments' => array('collection_policy.xsd'),
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection'),
    );

    $items['fedora'] = array(
      'page callback' => 'repository_page',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection'),
    );
    $items['fedora/repository'] = array(
      'title' => 'Digital Repository',
      'page callback' => 'repository_page',
      'type' => MENU_NORMAL_ITEM,
      'access arguments' => array('view fedora collection'),
    );

    $items['fedora/repository/service'] = array(
      'page callback' => 'repository_service',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection'),
    );

    $items['fedora/repository/object_download'] = array(
      'title' => t('Download object'),
      'page callback' => 'fedora_object_as_attachment',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection')
    );
    
    $items['fedora/repository/editmetadata'] = array(
      'title' => t('Edit metadata'),
      'page callback' => 'fedora_repository_edit_qdc_page',
      'type' => MENU_CALLBACK,
      'access arguments' => array('edit fedora meta data')
    );
    
    $items['fedora/repository/purgeStream'] = array(
      'title' => t('Purge data stream'),
      'page callback' => 'fedora_repository_purge_stream',
      'type' => MENU_CALLBACK,
      'access arguments' => array('purge objects and datastreams')
    );
    
    $items['fedora/repository/replaceStream'] = array(
      'title' => t('Replace Stream'),
      'page callback' => 'fedora_repository_replace_stream',
      'type' => MENU_CALLBACK,
      'access arguments' => array('add fedora datastreams'),
    );
    
    $items['fedora/repository/purgeObject'] = array(
      'title' => t('Purge object'),
      'page callback' => 'fedora_repository_purge_object',
      'type' => MENU_CALLBACK,
      'access arguments' => array('purge objects and datastreams')
    );
    
    $items['fedora/repository/addStream'] = array(
      'title' => t('Add stream'),
      'page callback' => 'add_stream',
      'type' => MENU_CALLBACK,
      'access arguments' => array('add fedora datastreams')
    );
    
    //new for mnpl******************************************
    $items['fedora/repository/mnpl_advanced_search'] = array(
      'title' => t('Repository advanced search'),
      'page callback' => 'fedora_repository_mnpl_advanced_search',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection')
    );
    
    $items['fedora/ingestObject'] = array(
      'title' => t('Ingest object'),
      'page callback' => 'fedora_repository_ingest_object',
      'type' => MENU_CALLBACK,
      'access arguments' => array('add fedora datastreams')
    );

    $items['fedora/repository/list_terms'] = array(
      'title' => t('List terms'),
      'page callback' => 'fedora_repository_list_terms',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection')
    );

    /* Export functionality */
    $items['fedora/basket'] = array(
      'title' => t('Fedora Basket'),
      'description' => t('View and download objects added to your basket'),
      'page callback' => 'fedora_repository_basket',
      'access arguments' => array('view fedora collection'),
      'type' => MENU_CALLBACK,
    );

    $items['fedora/repository/addToBasket'] = array(
      'page callback' => 'fedora_repository_add_to_basket',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection'),
    );

    $items['fedora/repository/removeFromBasket'] = array(
      'page callback' => 'fedora_repository_remove_from_basket',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection'),
    );

    $items['fedora/repository/add_search_results_to_basket'] = array(
      'page callback' => 'fedora_repository_add_search_results_to_basket',
      'type' => MENU_CALLBACK,
      'access arguments' => array('view fedora collection'),
    );

    $items['admin/settings/fedora_repository/object_details_xslt'] = array(
      'title' => "",
      'type' => MENU_CALLBACK,
      'page callback' => 'drupal_get_form',
      'page arguments' => array('fedora_repository_object_details_XSLT_config'),
      'file' => 'ObjectDetails.inc',
      'access arguments' => array('administer site configuration'),
    );
    $items['admin/settings/fedora_repository/object_details_table'] = array(
      'title' => "",
      'type' => MENU_CALLBACK,
      'page callback' => 'drupal_get_form',
      'page arguments' => array('fedora_repository_object_details_table_config'),
      'file' => 'ObjectDetails.inc',
      'access arguments' => array('administer site configuration'),
    );

    return $items;
  }

  /**
   * Create admin form
   * @return array 
   */
  function createAdminForm() {
    if (!user_access('administer site configuration')) {
      drupal_set_message(t('You must be a site administrator to edit the Fedora collections list.'), 'error');
      return;
    }
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $form = array();
    $form['fedora_repository_pid'] = array(
      '#type' => 'textfield',
      '#title' => t('Root Collection PID'),
      '#default_value' => variable_get('fedora_repository_pid', 'islandora:root'),
      '#description' => t('The PID of the Root Collection Object'),
      '#required' => TRUE,
      '#weight' => -18
    );

    $form['fedora_base_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora base URL'),
      '#default_value' => variable_get('fedora_base_url', 'http://localhost:8080/fedora'),
      '#description' => t('The URL to use for REST-type connections'),
      '#required' => TRUE,
      '#weight' => -16,
    );
    

    $form['fedora_repository_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora RISearch URL'),
      '#default_value' => variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch'),
      '#description' => t('The url of the Fedora server'), '#required' => TRUE,
      '#weight' => -14
    );
    
    $form['fedora_soap_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora SOAP Url'),
      '#default_value' => variable_get('fedora_soap_url', 'http://localhost:8080/fedora/services/access?wsdl'),
      '#description' => t('The URL to use for SOAP connections'),
      '#required' => TRUE,
      '#weight' => -12,
      '#suffix' => '<p>' . (
        fedora_available() ? 
          theme('image', 'misc/watchdog-ok.png') . t('Successfully connected to Fedora server at @fedora_soap_url', array('@fedora_soap_url' => variable_get('fedora_soap_url', ''))) :
          theme('image', 'misc/watchdog-error.png') . t('Unable to connect to Fedora server at @fedora_soap_url', array('@fedora_soap_url' => variable_get('fedora_soap_url', '')))) . '</p>',
    );

    $form['fedora_soap_manage_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Fedora SOAP management URL'),
      '#default_value' => variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/wsdl?api=API-M'), '#description' => t('The URL to use for SOAP API-M connections'),
      '#required' => TRUE,
      '#weight' => -10
    );

// will allow admin user to remove namepsace restrictions if not explicitly disallowed  in settings.php
    if (variable_get('allow_open_namespace', TRUE)) {
      $form['fedora_namespace'] = array(
        '#type' => 'fieldset',
      );

      $form['fedora_namespace']['fedora_namespace_restriction_enforced'] = array(
        '#weight' => -1,
        '#type' => 'radios',
        '#title' => t('Enforce namespace restrictions'),
        '#options' => array(
          TRUE => t('Enabled'),
          FALSE => t('Disabled')
        ),
        '#description' => t('Allow administrator to restrict user\'s access to the PID namepaces listed below'),
        '#default_value' => variable_get('fedora_namespace_restriction_enforced', TRUE)
      );

      $form['fedora_namespace']['fedora_pids_allowed'] = array(
        '#type' => 'textfield',
        '#title' => t('PID namespaces allowed in this Drupal install'),
        '#default_value' => variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '),
        '#description' => t('A space separated list of PID namespaces that users are permitted to access from this Drupal installation. <br /> This could be more than a simple namespace ie demo:mydemos.'),
        '#weight' => 0
      );
    }
    else {
      $form['fedora_pids_allowed'] = array(
        '#type' => 'textfield',
        '#title' => t('PID namespaces allowed in this Drupal install'),
        '#default_value' => variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '),
        '#description' => t('A space separated list of PID namespaces that users are permitted to access from this Drupal installation. <br /> This could be more than a simple namespace ie demo:mydemos.'),
        '#weight' => 0
      );
    }

    //have tabs options (like disable)
    $form['tabs'] = array(
      '#type' => 'fieldset',
      '#title' => t('Tabs Configuration'),
      '#description' => t('Configure the tabs available when viewing Fedora objects.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    //when checked show object details tab
    $form['tabs']['fedora_repository_show_object_details_tab'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show Object Details Tab'),
      '#default_value' => variable_get('fedora_repository_show_object_details_tab', TRUE),
      '#description' => t("When enabled, the 'Object Details' tab will be visible to users with the correct permissions when viewing an object in the repository"),
    );

    module_load_include('inc', 'fedora_repository', 'ObjectDetails');
    $primary_display_mode = variable_get('islandora_object_details_display_table', 'xslt');
    $profiles = module_invoke_all("islandora_object_details_display");

    $display_options = array();
    // suppress php warnings from empty lists
    if ($profiles) {
      foreach ($profiles as $machine_name => $profile) {
        // make sure the name exists (the bare minimum)
        if (array_key_exists('name', $profile)) {
          $display_options[$machine_name] = $profile['name'];
          if (array_key_exists('config', $profile)) {
            $config_path = $profile['config'];
            if (isset($config_path) && $config_path != "") {
              $display_options[$machine_name] .= " (". l("config", $config_path, array()) .")";
            }
          }
        }
      }
    }
    $form['tabs']['islandora_object_details_display_table'] = array(
      '#type' => 'radios',
      '#title' => t('Show object details as'),
      '#options' => $display_options,
      '#default_value' => $primary_display_mode,
      '#description' => t("Tells Islandora how to display the object details page for each object"),
    );

    $form['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced configuration options'),
      '#description' => t('Advanced configuration.  Under normal circumstances these will not be touched'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['advanced']['fedora_object_restrict_datastreams'] = array(
      '#type' => 'checkbox',
      '#title' => t('Restrict Access to Fedora Object Datastreams'),
      '#default_value' => variable_get('fedora_object_restrict_datastreams', FALSE),
      '#description' => t('When enabled, restricts access to fedora object datastreams that are not listed in the Islandora Content Model for the object (unless the user is an administrator).'),
    );
    
    $form['advanced']['fedora_repository_use_imagecache_external_in_collection_view'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow imagecache_external use for thumbnails in collection view'),
      '#default_value' => variable_get('fedora_repository_use_imagecache_external_in_collection_view', FALSE),
      '#description' => t('If enabled, the default collection list view (or ' .
        'anywhere the function "@function" is used) will try to use ' .
        'imagecache_external, defaulting to the "@preset" preset.  XSLTs may ' .
        'be updated to use this function.', 
        array(
          '@function' => '_fedora_repository_render_image()',
          '@preset' => 'fedora_repository_collection_thumbnail',
          '@xsl' => 'sparql_to_html.xsl',
        )),
    );
    
    $form['advanced']['fedora_control_group_control_during_ingest'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow control groups select in datastream ingest'),
      '#description' => t('Whether or not we should allow the user to select which control group to ingest a stream as, or to follow the old paradigm--to add stream IDed as XML as inline, and everything else as managed.'),
      '#default_value' => variable_get('fedora_control_group_control_during_ingest', FALSE),
    );

    //Export functionality
    $form['advanced']['module']['export_area'] = array(
      '#type' => 'textfield',
      '#title' => t('Export area'),
      '#default_value' => variable_get('export_area', file_directory_path() . '/fedora_export_area'),
      '#description' => t("Path to the export area. It must be accessible by druapl (i.e. apache user)."),
      '#required' => TRUE,
    );

    $form['#attributes'] = array('enctype' => "multipart/form-data");


    return system_settings_form($form);
  }

  /**
   * Update Metadata
   * @param type $form_id
   * @param type $form_values
   * @param type $client
   * @return type 
   */
  function updateMetaData($form_id, $form_values, $client) {
    // ======================================
    // = begin creation of foxml dom object =
    // ======================================
    $dom = new DomDocument("1.0", "UTF-8");
    $dom->formatOutput = TRUE;

    ///begin writing qdc

    $oai = $dom->createElement("oai_dc:dc");
    $oai->setAttribute('xmlns:oai_dc', "http://www.openarchives.org/OAI/2.0/oai_dc/");
    $oai->setAttribute('xmlns:dc', "http://purl.org/dc/elements/1.1/");
    $oai->setAttribute('xmlns:dcterms', "http://purl.org/dc/terms/");
    $oai->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");

    // DC elements
    $previousElement = NULL; // Used in case we have to nest elements for qualified dublin core.
    
    $label = null;
    foreach ($form_values as $key => $value) {
      //echo ($key);
      if ($key == 'dc:title-0') {
        $label = $value;
      } 
      $index = strrpos($key, '-');
      $key = substr($key, 0, $index);
      $test = substr($key, 0, 2);
      if ($test == 'dc' || $test == 'ap') { // Don't try to process other form values.
        try {
          if (!strcmp(substr($key, 0, 4), 'app_')) {
            $key = substr($key, 4);
            $previousElement->appendChild($dom->createElement($key, $value));
          }
          else {
            $previousElement = $dom->createElement($key, $value);
            $oai->appendChild($previousElement);
          }
        } catch (exception $e) {
          drupal_set_message(t($e->getMessage()), 'error');
          continue;
        }
      }
    }

    // Save the title
    if ( $label != null )
    {
      $object = new Fedora_Item($form_values['pid']);
      $object->modify_object($label);
    } 
    
    $dom->appendChild($oai);

    if (!$client) {
      return;
    }

    $pid = $form_values['pid'];
    $dsId = $form_values['dsid'];
    $params = array(
      "pid" => $pid,
      "dsID" => $dsId,
      "altIDs" => "",
      "dsLabel" => "Qualified Dublin Core",
      "MIMEType" => "text/xml",
      "formatURI" => "URL",
      "dsContent" => $dom->saveXML(), "checksumType" => "DISABLED", "checksum" => "none",
      "logMessage" => "datastream_modified", "force" => "TRUE");
    try {
      $object = $client->__soapCall('ModifyDatastreamByValue', array($params));
    } catch (exception $e) {
      drupal_set_message(t("Error updating metadata") . " " . $e->getMessage(), 'error');
    }
  }

  /**
   * queries the collection object for a childsecurity datastream and if found parses it
   * to determine if this user is allowed to ingest in this collection
   * we assume if they are able to modify objects in the collection they can ingest as well.
   * @param type $collection_pid
   * @return type 
   */
  function can_ingest_here($collection_pid) {
    module_load_include('inc', 'fedora_repository', 'SecurityClass');
    $securityClass = new SecurityClass();
    return $securityClass->canIngestHere($collection_pid);
  }

  /**
   * Drupal's permissions at this point no xacml yet. xacml decisions are made by fedora
   * @param type $collection_pid
   * @return type 
   */
  function canShowIngestForm($collection_pid) {
    if (!user_access('ingest new fedora objects')) {
      drupal_set_message(t('You do not have permission to ingest.'), 'error');
      return FALSE;
    }
    module_load_include('inc', 'fedora_repository', 'SecurityClass');
    $security_class = new SecurityClass();
    if (!$security_class->canIngestHere($collection_pid)) {
      // Queries the collection object for a child security datastream and if found parses it
      // to determine if this user is allowed to ingest in this collection
      // we assume if they are able to modify objects in the collection they can ingest as well.
      drupal_set_message(t('You do not have premission to ingest here.'));
      return FALSE;
    }
    if ($collection_pid == NULL) {
      drupal_set_message(t('You must specify an collection object pid to ingest an object.'), 'error');
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Creates the first page of the ingest form for editing QDC.
   * 
   * @param string $collection_pid
   * @param string $collection_label
   * @param array $form_state
   * 
   * @return array
   */
  function createQDCIngestFormPageOne($collection_pid, $collection_label, array &$form_state) {
    if (($collection_policy = CollectionPolicy::loadFromCollection($collection_pid)) === FALSE) {
      drupal_set_message(t('Unable to load collection policy \'' . $collection_pid . '\'.'));
      return FALSE;
    }
    if (!($content_models = $collection_policy->getContentModels())) {
      drupal_set_message(t('No content models associated with this collection: !collection_label. Please contact your administrator.', array('!collection_label' => $collection_label)), 'error');
      return FALSE;
    }
    $potential_models = array();
    foreach ($content_models as $content_model) {
      $identifier = $content_model->getIdentifier();
      $name = $content_model->name;
      $potential_models["$identifier"] = "$name";
    }
    $identifier = key($potential_models);
    $selected_model = isset($form_state['values']['models']) ? $form_state['values']['models'] : $identifier;
    return array(
      'indicator' => array(
        '#type' => 'fieldset',
        '#title' => t('Ingest digital object into collection_pid !collection_label Step #1', array('collection_pid' => $collection_pid, '!collection_label' => $collection_label)),
        'models' => array(
          '#type' => 'select',
          '#title' => t('Content models available'),
          '#options' => $potential_models,
          '#default_value' => $selected_model,
          '#description' => t('Content models define datastream composition, relationships between this and other content models, and the mandatory behaviors associated with each digital object.<br /> Additional information may be found <a href="https://wiki.duraspace.org/display/FEDORACREATE/Content+Models+Overview">here.</a>'),
        ),
      ),
      'collection_pid' => array(
        '#type' => 'hidden',
        '#value' => $collection_pid
      ),
      'submit' => array(
        '#type' => 'submit',
        '#submit' => array('fedora_repository_ingest_form_submit'),
        '#value' => t('Next')
      ),
    );
  }

  /**
   * Create the second page of the ingest form for editing QDC
   * 
   * @param string $collection_pid
   * @param string $collection_label
   * @param array $form_state
   * 
   * @return array
   */
  function createQDCIngestFormPageTwo($collection_pid, $collection_label, array &$form_state) {
    module_load_include('inc', 'fedora_repository', 'ContentModel');
    $form = array();
    $content_model_pid = ContentModel::getPidFromIdentifier($form_state['values']['models']);
    $content_model_dsid = ContentModel::getDSIDFromIdentifier($form_state['values']['models']);
    if (($content_model = ContentModel::loadFromModel($content_model_pid, $content_model_dsid)) !== FALSE) {
      $form = $content_model->buildIngestForm($form, $form_state);
      if ($form === FALSE) {
        drupal_set_message(t("Error Building Ingest Form."), 'error');
        foreach (ContentModel::$errors as $error) {
          drupal_set_message($error, 'error');
        }
      }
      $form['collection_pid'] = array(
        '#type' => 'hidden',
        '#value' => $collection_pid
      );
      $form['submit'] = array(
        '#type' => 'submit',
        '#submit' => array('fedora_repository_ingest_form_submit'),
        '#value' => t('Ingest')
      );
    }
    return $form;
  }

  /**
   * Create the QDC Ingest Form.
   * 
   * @param type $collection_pid
   * @param type $collection_label
   * @param array $form_state 
   * 
   * @return array
   */
  function createQDCIngestForm($collection_pid, $collection_label, array &$form_state) {
    module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
    $form_state['storage']['step'] = empty($form_state['storage']['step']) ? 1 : $form_state['storage']['step'];
    if ($form_state['storage']['step'] == 1) {
      return $this->createQDCIngestFormPageOne($collection_pid, $collection_label, $form_state);
    }
    else {
      return $this->createQDCIngestFormPageTwo($collection_pid, $collection_label, $form_state);
    }
  }

  /**
   * Create a multi step form (wizard) for ingesting objects into Fedora
   * 
   * @param string $collection_pid
   * @param string $collection_label
   * @param array $form_state
   * 
   * @return array 
   */
  function createIngestForm($collection_pid, $collection_label, array &$form_state) {
    if (!$this->canShowIngestForm($collection_pid)) {
      return FALSE;
    }
    
    drupal_set_title($collection_label);
    if (module_exists('islandora_content_model_forms')) {
      module_load_include('inc', 'islandora_content_model_forms', 'IngestObjectMetadataForm');
      try {
        $form = new IngestObjectMetadataForm();
        return $form->create($collection_pid, $collection_label, $form_state);
      } catch (Exception $e) {
        //dd('An error occured; reverting to QDC form...');
        $form_state['storage']['xml'] = FALSE; // An error occured revert back to the QDC Form.
      }
    }
    return $this->createQDCIngestForm($collection_pid, $collection_label, $form_state);
  }

  /**
   * this function may not be being used
   * @param type $pid
   * @param type $form_state
   * @return string 
   */
  function createAddDataStreamForm($pid, &$form_state) {
    //dump_vars($form_state);
    // Populate the list of datastream IDs.

    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'ContentModel');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');

    $obj_helper = new ObjectHelper();
    $content_models = $obj_helper->get_content_models_list($pid);
    $available_dsids = array();
    if (!empty($content_models)) {
      foreach ($content_models as $content_model) {


        $newElements = $content_model->listDatastreams();
        if (!empty($newElements)) {
          $available_dsids = array_merge($available_dsids, $newElements);
        }
      }
    }

    $item = new Fedora_Item($pid);
    $used_datastreams = $item->get_datastreams_list_as_SimpleXML();
    $used_datastream_ids = array();
    foreach ($used_datastreams->datastreamDef as $used_datastream) {
      array_push($used_datastream_ids, $used_datastream->ID);
    }
    $unused_dsids = array();

    if ($form_state['submitted'] && $form_state['clicked_button']['#value'] != 'OK') {
      $form['add_datastream_label'] = array(
        '#value' => t('<br /><h3>The datastream has been uploaded.</h3>'),
        '#weight' => -10,
      );
      $form['#redirect'] = "fedora/repository/$pid/";
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('OK')
      );
      return $form;
    }
    if (!empty($available_dsids)) {
      $unused_dsids = array_diff($available_dsids, $used_datastream_ids);
      if (empty($unused_dsids)) {
        return;
      }
    }

    $form['fieldset'] = array(
      '#type' => 'fieldset',
      '#title' => t('Add datastream'),
    );
    //$form['add_datastream_label'] = array(
    //  '#value' => t('<br /><h3>Add Datastream:</h3>'),
    //  '#weight' => -10,
    //);

    $form['fieldset']['pid'] = array(
      '#type' => 'hidden',
      '#value' => "$pid"
    );

    $form['fieldset']['stream_label'] = array(
      '#title' => 'Datastream Label',
      '#required' => 'TRUE',
      '#description' => t('A Human readable label'),
      '#type' => 'textfield'
    );

    $form['#attributes']['enctype'] = 'multipart/form-data';
    $form['fieldset']['add-stream-file-location'] = array(
      '#type' => 'file',
      '#title' => t('Upload Document'),
      '#size' => 48,
      // '#required'=>'TRUE',
      '#description' => t('The file to upload. (Only for Managed and Inline)')
    );
    
    if (variable_get('fedora_control_group_control_during_ingest', FALSE)) {
      $form['fieldset']['ds_reference'] = array(
        '#type' => 'textfield',
        '#title' => t('Datastream reference'),
        '#size' => 48,
        '#description' => t('A URL reference to resolve for the contents of the datastream.  (Required for External and Redirect, but will still work for Managed and Inline.)'),
      );
    }
    
    $form['#redirect'] = "fedora/repository/$pid/";
    $form['fieldset']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add Datastream')
    );

    if (!empty($unused_dsids)) {
      $form['fieldset']['stream_id'] = array(
        '#type' => 'select',
        '#title' => t('Datastream ID'),
        '#default_value' => variable_get('feed_item_length', 'teaser'),
        '#weight' => -1,
        '#description' => t('Datastream IDs defined by the content model.'),
        '#options' => array_combine($unused_dsids, $unused_dsids),
      );
    }
    else {
      $form['fieldset']['stream_id'] = array(
        '#title' => 'Datastream ID',
        '#required' => 'TRUE',
        '#description' => t('An ID for this stream that is unique to this object. Must start with a letter and contain only alphanumeric characters and dashes and underscores.'),
        '#type' => 'textfield',
        '#weight' => -1,
      );
    }
    
    if (variable_get('fedora_control_group_control_during_ingest', FALSE)) {
      $form['fieldset']['control_group'] = array(
        '#type' => 'select',
        '#title' => t('Control group'),
        '#options' => array(
          'X' => t('Inline XML'),
          'M' => t('Managed datastream'),
          'E' => t('Externally Referenced/managed datastream'),
          'R' => t('Redirect datastream'),
        ),
        '#description' => t('The manner in which the datastream will be stored.  "Inline XML" must be XML and will be placed directly into the FOXML for the object.  "Managed" datastreams are made to live on the filesystem as discrete files in the Fedora data directory.  Both "Redirect" and "External" streams are URL references; the difference being the redirect stream instruct clients to perform an HTTP redirect, such that the data does not pass though Fedora (useful for streaming). External streams are mediated (by which I mean loaded and streamed from) the Fedora server.'),
        '#weight' => 0,
      );
    }
    return $form;
  }

  /**
   * Create QDC Edit form.
   * 
   * @param string $pid
   * @param string $dsid
   * @param object $client
   * @param array $form_state
   * 
   * @return string 
   */
  function createQDCEditForm($pid, $dsid, $client, &$form_state) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $dsid = empty($dsid) ? 'QDC' : $dsid;

    $item = new fedora_item($pid);
    $content = trim($item->get_datastream_dissemination($dsid));

    $doc = new DOMDocument();
    if (!$doc->loadXML($content)) {
      echo "error loading xml";
    }
    $oai_dc = $doc->getElementsByTagName('dc');
    $dcItems = $oai_dc->item(0)->getElementsByTagName('*');
    $form = array();
    for ($i = 0; $i < $dcItems->length; $i++) {
      $name = $dcItems->item($i)->nodeName;
      if ($name == 'dc:description') {
        $form["$name" . '-' . "$i"] = array(
          '#title' => $name,
          '#type' => 'textarea',
          '#default_value' => $dcItems->item($i)->nodeValue,
          '#description' => 'Dublin Core ' . substr($dcItems->item($i)->nodeName, 3)
        );
      }
      elseif ($name == 'dc:title') {
        $form["$name" . '-' . "$i"] = array(
          '#title' => $name,
          '#type' => 'textfield',
          '#required' => 'TRUE',
          '#default_value' => $dcItems->item($i)->nodeValue,
          '#description' => 'Dublin Core ' . substr($dcItems->item($i)->nodeName, 3)
        );
      }
      else {
        if ($oai_dc->item(0)->nodeName != $dcItems->item($i)->parentNode->nodeName) {
          $description = strstr($name, ':');
          $form['app_' . "$name" . '-' . "$i"] = array(
            '#title' => $name,
            '#type' => 'textfield',
            '#default_value' => $dcItems->item($i)->nodeValue,
            '#description' => 'Dublin Core ' . substr($description, 1)
          );
        }
        else {
          $field_type = 'textfield';
          $value = $dcItems->item($i)->nodeValue;
          if ($name == 'dc:coverage') {
            $value = '';
          }
          if ($name == 'dc:rights') {
            $field_type = 'textarea';
          }
          $description = strstr($name, ':');
          $form["$name" . '-' . "$i"] = array(
            '#title' => $name,
            '#type' => $field_type,
            '#default_value' => $value,
            '#description' => 'Dublin Core ' . substr($description, 1)
          );
        }
      }
    }
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => "$pid"
    );
    $form['dsid'] = array(
      '#type' => 'hidden',
      '#value' => "$dsid"
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Update Metadata'),
    );
    return $form;
  }

  /**
   * Creates a drupal form to edit either the QDC or DC datastream
   * @param type $pid
   * @param type $dsid
   * @param type $client
   * @param boolean $form_state
   * @return type 
   */
  function createMetaDataForm($pid, $dsid, $client, &$form_state) {
    if (module_exists('islandora_content_model_forms')) {
      module_load_include('inc', 'islandora_content_model_forms', 'EditObjectMetadataForm');
      try {
        $form = new EditObjectMetadataForm($form_state);
        return $form->create($pid, $client, $form_state);
      } catch (Exception $e) {
        $form_state['storage']['xml'] = FALSE; // An error occured revert back to the QDC Form.
      }
    }
    return $this->createQDCEditForm($pid, $dsid, $client, $form_state);
  }

  /**
   * Creates a form for replacing datastream
   * @param type $pid
   * @param type $dsId
   * @param type $dsLabel
   * @param type $form_state
   * @return type 
   */
  function createReplaceDataStreamForm($pid, $dsId, $dsLabel, &$form_state) {
    $form = array();

    $form['#attributes']['enctype'] = 'multipart/form-data';
    
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);
    $info = $item->get_datastream_info($dsId);
    $control_group = $info->datastream->controlGroup;
    if (in_array($control_group, array('M', 'X'))) {
      $form['file'] = array(
        '#type' => 'file',
        '#title' => t('Upload Document'),
        '#description' => t('A file with which to replace the contents of this datastream.'),
      );
    }
    if ($control_group != 'X') {
      $form['reference'] = array(
        '#type' => 'textfield',
        '#title' => t('Reference to object'),
        '#description' => t('A URL the datastream will be updated to reference.'),
      );
    }
    if ($control_group == 'M') {
      $form['note'] = array(
        '#type' => 'item',
        '#title' => t('NOTE'),
        '#value' => t('If both a file and a reference are given, the file will be given preference.'),
      );
    }
    
    $form['pid'] = array(
      '#type' => 'value',
      '#value' => $pid,
    );

    $form['dsId'] = array(
      '#type' => 'value',
      '#value' => $dsId,
    );

    $form['dsLabel'] = array(
      '#type' => 'value',
      '#value' => $dsLabel,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Replace Datastream')
    );

    return $form;
  }

}
