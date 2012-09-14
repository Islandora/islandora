<?php

/**
 * @file
 *   This file defines the hooks that fedora_repository (islandora)
 *   makes available.
 */

/**
 * Implements hook_islandora_tabs().
 * This hook lets one add tabs to the object page in Islandora.
 *
 * @param array $content_models
 *   An Array of content model objects.  A content model is only included
 *   if the object actualy exists with a ISLANDORACM datastream.
 * @param string $pid
 *   The Fedora PID of the object who's page is firing the hook.
 * @param int $page_number
 *   Page number for collection views.
 *
 * @return array
 *   $tabset a tab definition.
 */
function hook_islandora_tabs($content_models, $pid, $page_number) {

  $tabset['A TAB'] = array(
    '#type' => 'tabpage',
    '#title' => t('A TITLE'),
    '#content' => 'content')
  );

  return $tabset;
}
