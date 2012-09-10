<?php

/**
 * @file
 * This file contains functions to create breadcrumbs on Islandora object pages.
 */

function islandora_get_breadcrumbs($object) {
  $breadcrumbs = array();

  islandora_get_breadcrumbs_recursive($object->id, $breadcrumbs, $object->repository);

  if (isset($breadcrumbs[0])) {
    unset($breadcrumbs[0]);
  }

  $breadcrumbs = array_reverse($breadcrumbs);

  return $breadcrumbs;
}

/**
 * Builds an array of drupal links for use in breadcrumbs.
 *
 * @todo Make fully recursive...
 *
 * @global type $base_url
 * @param type $pid
 * @param type $breadcrumbs
 * @param type $level
 */
function islandora_get_breadcrumbs_recursive($pid, &$breadcrumbs, $repository) {
  // Before executing the query, we hve a base case of accessing the top-level collection
  global $base_url;
  static $max_level = 10;
  static $level = -1;

  if (count($breadcrumbs) === 0) {
    $level = $max_level;
  }

  $root = variable_get('islandora_repository_pid', 'islandora:root');
  if ($pid == $root) {
    $breadcrumbs[] = l(menu_get_active_title(), 'islandora');
    $breadcrumbs[] = l(t('Home'), '<front>');
  }
  else {
    $query_string = 'select $parentObject $title $content from <#ri>
                      where (
                        <info:fedora/' . $pid . '> <fedora-model:label> $title
                        and $parentObject <fedora-model:hasModel> $content
                        and (
                          <info:fedora/' . $pid . '> <fedora-rels-ext:isMemberOfCollection> $parentObject
                          or <info:fedora/' . $pid . '> <fedora-rels-ext:isMemberOf> $parentObject
                          or <info:fedora/' . $pid . '> <fedora-rels-ext:isPartOf> $parentObject
                        )
                        and $parentObject <fedora-model:state> <info:fedora/fedora-system:def/model#Active>
                      )
                      minus $content <mulgara:is> <info:fedora/fedora-system:FedoraObject-3.0>
                      order by $title desc';
    $results = $repository->ri->itqlQuery($query_string);

    if (count($results) > 0  && $level > 0) {
      $parent = $results[0]['parentObject']['value'];
      $this_title = $results[0]['title']['value'];

      if (empty($this_title)) {
        $this_title = t('-');
      }

      $breadcrumbs[] = l($this_title, "islandora/object/$pid");

      $level--;
      islandora_get_breadcrumbs_recursive($parent, $breadcrumbs, $repository);
    }
    else {
      $breadcrumbs[] = '...'; // Add an non-link, as we don't know how to get back to the root.
      islandora_get_breadcrumbs_recursive($root, $breadcrumbs, $repository); // And render the last two links and break (on the next pass).
    }
  }
}