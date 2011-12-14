<?php

/**
 * @file
 * ShowSlideStreamsInFieldSets class
 */

/**
 * ShowSlideStreamInFieldSets ??????
 */
class ShowSlideStreamsInFieldSets {

  private $pid = NULL;

  /**
   * Contructor 
   * @param type $pid 
   */
  function ShowSlideStreamsInFieldSets($pid) {
    //drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    $this->pid = $pid;
  }

  /**
   * Show JPEG
   * @global type $base_url
   * @global type $user
   * @return type 
   */
  function showJPG() {
    module_load_include('inc', 'fedora_repository', 'plugins/tagging_form');
    module_load_include('inc', 'fedora_repository', 'plugins/ShowStreamsInFieldSets');
    global $base_url;
    global $user;

    $tabset = array();

    $qs = '';
    if ($user->uid != 0) {
      $qs = '?uid=' . base64_encode($user->name . ':' . $user->pass);
    }

    $viewer_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora') . '/get/' . $this->pid . '/ilives:viewerSdef/getViewer' . $qs;
    $html = '<iframe src="' . $viewer_url . '" scrolling="no" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';

    drupal_add_css(path_to_theme() . '/header-viewer.css', 'theme');

    $tabset['second_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Full-size'),
      '#content' => $html);

    $tabset['first_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('View'),
      // This will be the content of the tab.
      '#content' => '<img src="' . $base_url .
      '/fedora/imageapi/' . $this->pid . '/JPG/JPG.jpg' . '" />' . '<p>' . drupal_get_form('fedora_repository_image_tagging_form', $this->pid) . '</p>',
    );

    return $tabset;
  }

}
