<?php



/**
 * @file
 * ShowDemoStreamsInFieldSets class
 */

/**
 * Show Demo Streams in Field Sets ???
 */
class ShowDemoStreamsInFieldSets {

  private $pid = NULL;
  
  /**
   * Constructor
   * @param type $pid 
   */
  function ShowDemoStreamsInFieldSets($pid) {
    //drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    $this->pid = $pid;
  }

  /**
   * Show Medium Size ??
   * @global type $base_url
   * @return type 
   */
  function showMediumSize() {
    $path = "fedora/repository/{$this->pid}/MEDIUM_SIZE";
    $collection_fieldset = array(
      '#collapsible' => FALSE,
      '#value' => l(theme('image', $path), $path, array('html' => TRUE)),
    );
    return theme('fieldset', $collection_fieldset);
  }

}
