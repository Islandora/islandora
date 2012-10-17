<?php

/**
   * Setups the globals for the Drupal pager.
   *
   * Does just what it says:  Hacks the default Drupal pager such that it might
   * be rendered, likely with: theme('pager', array(), $per_page, $pager_name)
   * (I reccomend seeing the real documentation for more detail, but the first
   * array can be a list of the tags to use for first, previous, next and last
   * (text in the pager), I don't believe per_page is actually used in the theme
   * function, and $pager_name is an integer used to identify the pager (such 
   * that there can be more than one--that is, tracking different lists of 
   * content on a single page.  You can render the exact same pager multiple 
   * times, say if you want one at the top and bottom of a list, using the same 
   * ID/pager_name.
   *
   * @global $pager_total array
   *   Numerically indexed array, where keys are the $pager_names and values
   *   are the number of pages in the given set, based on: ceil($total_items/$per_page);
   * @global $pager_page_array array
   *   Numerically indexed array, where keys are the $pager_names and values
   *   are the page selected in the relevant set.
   * @param $pager_name int
   *   An integer to identify the pager to affect.  Do note that paging in using
   *   this function will add the 'page' HTTP GET parameter to the URL, with 
   *   the value containing a comma-separated list with max($pager_name + 1)
   *   values--that is, if you create a single pager named '10', the 'next'
   *   link will look something like: 0,0,0,0,0,0,0,0,0,0,1
   * @param $per_page int
   *   An integer representing the number of items per page.
   * @param $total_items int
   *   An integer representing the total number of items in the set.
   * @return int
   *   An integer representing what the current page should be.
   */
function fedora_repository_setup_pager($pager_name, $per_page = NULL, $total_items = NULL) {
  global $pager_total, $pager_page_array;

  if ($per_page !== NULL && $total_items !== NULL) {
    $pager_total[$pager_name] = ceil($total_items / $per_page);
  }

  //XXX: Don't know that this is neccessary, to try to load all the time, or
  //  whether Drupal will load it automatically somewhere...  Docs seems a
  //  a little sparse.
  $page_info = explode(',', isset($_GET['page']) ? $_GET['page'] : '');
  $page = $page_info[$pager_name];
  if ($page < 0) {
    $page = 0;
  }

  if (!isset($pager_page_array)) {
    $pager_page_array = pager_load_array($page, $pager_name, $page_info);
  }
  else {
    $pager_page_array = pager_load_array($page, $pager_name, $pager_page_array);
  }

  $page = $pager_page_array[$pager_name];
  return $page;
}

