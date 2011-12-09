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
    drupal_set_title($this->item->objectProfile->objLabel);
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
    );
    $tabset['fedora_object_details']['tabset'] = array(
      '#type' => 'tabset',
    );
    $dc_html = $objectHelper->getFormattedDC($this->item);

    $ds_list = $objectHelper->get_formatted_datastream_list($this->pid, NULL, $this->item);


    $tabset['fedora_object_details']['tabset']['view'] = array(
      '#type' => 'tabpage',
      '#title' => t('View'),
      '#content' => $dc_html . $ds_list . $purge_form,
    );

    if (fedora_repository_access(OBJECTHELPER :: $EDIT_FEDORA_METADATA, $this->pid, $user)) {
      $editform = drupal_get_form('fedora_repository_edit_qdc_form', $this->pid, 'DC');
      $tabset['fedora_object_details']['tabset']['edit'] = array(
        '#type' => 'tabpage',
        '#title' => t('Edit'),
        '#selected' => $show_edit_tab,
        '#content' => $editform,
      );
    }

    return $tabset;
  }

}
