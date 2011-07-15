<?php
// $Id$

/**
 * @package Islandora
 * @subpackage plugins
 */


/*
 * Created on 17-Apr-08
 *
 *
 */
class ShowDemoStreamsInFieldSets {
  private $pid = NULL;
  function ShowDemoStreamsInFieldSets($pid) {
  //drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    $this->pid = $pid;
  }

  function showMediumSize() {
    global $base_url;
    $collection_fieldset = array(
        '#collapsible' => FALSE,
        '#value' => '<a href="'. $base_url . '/fedora/repository/'. $this->pid . '/MEDIUM_SIZE/"><img src="'.
          $base_url . '/fedora/repository/'. $this->pid . '/MEDIUM_SIZE/MEDIUM_SIZE'. '" /></a>',
    );
    return theme('fieldset', $collection_fieldset);
  }
}
