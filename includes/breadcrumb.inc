<?php

/**
 * @file
 *
 * This file contains functions to create breadcrumbs on Islandora object pages.
 */

/**
 * Get an array of links to be passed to drupal_set_breadcrumb(). This is used
 * for generating the bread-crumbs for the view object page.
 *
 * Each link in the bread-crumbs represents a member of the given objects ancestry
 * which is identified by any of the following RELS-EXT terms
 * (isMemberOf,isMemberOfCollection,isPartOf).
 *
 * @param FedoraObject $object
 *   An object whose ancestry will be mapped to bread-crumbs.
 *
 * @see drupal_set_breadcrumb()
 *
 * @return array
 *   Array of links, starting with most distant ancestor proceeding up to but
 *   not including the given object. For use in the function
 *   drupal_set_breadcrumb().
 */
function islandora_get_breadcrumbs($object) {
  $breadcrumbs = array();
  islandora_get_breadcrumbs_recursive($object->id, $breadcrumbs, $object->repository);
  if (isset($breadcrumbs[0])) {
    // Remove the actual object.
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
 * @todo Could use some clean up, can't be called multiple times safely due to
 *   the use of static variables.
 *
 * @param string $pid
 *   THe object id whose parent will be fetched for the next link.
 * @param array $breadcrumbs
 *   The list of existing bread-crumb links in reverse order.
 * @param FedoraRepository $repository
 *   The fedora repository.
 */
function islandora_get_breadcrumbs_recursive($pid, array &$breadcrumbs, FedoraRepository $repository) {
  // Before executing the query, we hve a base case of accessing the
  // top-level collection
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
      // Add an non-link, as we don't know how to get back to the root.
      $breadcrumbs[] = '...';
      // And render the last two links and break (on the next pass).
      islandora_get_breadcrumbs_recursive($root, $breadcrumbs, $repository);
    }
  }
}
