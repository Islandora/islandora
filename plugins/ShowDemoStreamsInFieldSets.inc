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
    global $base_url;
    $collection_fieldset = array(
      '#collapsible' => FALSE,
      '#value' => '<a href="' . $base_url . '/fedora/repository/' . $this->pid . '/MEDIUM_SIZE/"><img src="' .
      $base_url . '/fedora/repository/' . $this->pid . '/MEDIUM_SIZE/MEDIUM_SIZE' . '" /></a>',
    );
    return theme('fieldset', $collection_fieldset);
  }

}
