<?php

/**
 * @file
 * FedoraObjectDetailedContent class
 */

/**
 * Fedora Object. This class is a plugin called from content models to display a detailed list of 
 * content of the Fedora Item. This is hard coded into Islandora core, and it can also be called
 * from the IslandoraCM stream.
 */
class FedoraObjectDetailedContent {

  /**
   * Constructor
   * @param type $pid 
   */
  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {

      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  /**
   * Show Field Sets
   * @global type $user
   * @return type 
   */
  public function showFieldSets() {
    global $user;
    $objectHelper = new ObjectHelper();
    $tabset = array();
    $show_purge_tab = (!empty($_POST['form_id']) && $_POST['form_id'] == 'fedora_repository_purge_object_form');
    $show_edit_tab = (!empty($_POST['form_id']) && $_POST['form_id'] == 'fedora_repository_edit_qdc_form');
    $purge_form = drupal_get_form('fedora_repository_purge_object_form', $this->pid, check_plain(substr(request_uri(), strlen(base_path()))));
    $details_selected = ($show_purge_tab || $show_edit_tab);
    $tabset['fedora_object_details'] = array(
      '#type' => 'tabpage',
      '#title' => t('Object Details'),
      '#selected' => $details_selected,
      '#weight' => 100, //XXX: Make the weight configurable?
    );
    $tabset['fedora_object_details']['tabset'] = array(
      '#type' => 'tabset',
    );

    module_load_include('inc', 'fedora_repository', 'ObjectDetails');
    $object_details_profile = variable_get('islandora_object_details_display_table', 'xslt');
    $profiles = module_invoke_all("islandora_object_details_display");
    $profile = $profiles[$object_details_profile];
    if (!isset($profile)) {
      // default behaviour
      watchdog('fedora_repository', "Error while reading the default object details display profile: @e", array("@e" => $e->getMessage()), WATCHDOG_WARNING);
      $dc_returned = $objectHelper->getFormattedDC($this->item);
    }
    else {
      // invoke the requested display profile
      require_once(drupal_get_path('module', $profile['module']) ."/". $profile['file']);
      $details_function = $profile['function'];
      if (function_exists($details_function)) {
        $dc_returned = $details_function($this->item);
      }
      else {
        // problem - display profile not found
        watchdog('fedora_repository', "Error - could not find object details display function @function", array('@function' => $details_function), WATCHDOG_WARNING);
      }
    }

    $dc_array = array();
    $i = 0;
    if (is_array($dc_returned)) {
      $dc_array = $dc_returned;
      $dc_array['#weight'] = $i++;
    }
    elseif (!empty($dc_returned)) {
      $dc_array = array(
        '#type' => 'markup',
        '#value' => $dc_returned, //XXX:  This could easily be done in Drupal, instead of using an XSL
        '#weight' => $i++
      );
    }

    $tabset['fedora_object_details']['tabset']['view'] = array(
      '#type' => 'tabpage',
      '#title' => t('View'),
    );

    if (!empty($dc_array)) {
      $tabset['fedora_object_details']['tabset']['view']['dc'] = $dc_array;
    }

    if (fedora_repository_check_perm(ObjectHelper :: $VIEW_DETAILED_CONTENT_LIST, $this->pid, $user)) {
      $tabset['fedora_object_details']['tabset']['view'] += array(
        'list' => array(
          '#type' => 'fieldset',
          '#title' => t('Detailed List of Content'),
          '#attributes' => array(
            'class' => 'fedora_detailed_list',
          ),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#weight' => $i++,
          'parents' => array(
            '#type' => 'markup',
            '#value' => $objectHelper->get_parent_objects_asHTML($this->pid),
            '#weight' => $i++,
          ),
          'datastreams' => array(
            '#type' => 'markup',
            '#value' => $objectHelper->get_formatted_datastream_list($this->pid, NULL, $this->item), //XXX:  The function called here could be cleaned up a fair bit as well...
            '#weight' => $i++,
          ),
        ),
      );
    }

    if (fedora_repository_check_perm(ObjectHelper :: $PURGE_FEDORA_OBJECTSANDSTREAMS, $this->pid, $user)) {
      $tabset['fedora_object_details']['tabset']['view'] += array(
        'purge' => array(
          '#type' => 'markup',
          '#value' => $purge_form,
          '#weight' => $i++
        )
      );
    }

    if (fedora_repository_check_perm(OBJECTHELPER :: $EDIT_FEDORA_METADATA, $this->pid, $user)) {
      $editform = drupal_get_form('fedora_repository_edit_qdc_form', $this->pid, 'DC');
      $tabset['fedora_object_details']['tabset']['edit'] = array(
        '#type' => 'tabpage',
        '#title' => t('Edit'),
        '#selected' => $show_edit_tab,
        '#content' => $editform,
      );
    }

    $ts = $tabset['fedora_object_details']['tabset'];
    if ((array_key_exists('view', $ts) && (count(element_children($ts['view'])) > 0)) || 
      (array_key_exists('edit', $ts) && (count(element_children($ts['edit'])) > 0 || array_key_exists('#content', $ts['edit'])))) {
      return $tabset;
    }
    else {
      return array();
    }
  }

}
