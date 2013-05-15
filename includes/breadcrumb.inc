<?php

/**
 * @file
 * This file contains functions to create breadcrumbs on Islandora object pages.
 */

/**
 * Get an array of links to be passed to drupal_set_breadcrumb().
 *
 * This is used for generating the bread-crumbs for the view object page.
 *
 * Each link in the bread-crumbs represents a member of the given objects
 * ancestry which is identified by any of the following RELS-EXT terms
 * (isMemberOf,isMemberOfCollection,isPartOf).
 *
 * @param AbstractObject $object
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
  $breadcrumbs = islandora_get_breadcrumbs_recursive($object->id, $object->repository);
  array_pop($breadcrumbs);
  return $breadcrumbs;
}

/**
 * Builds an array of drupal links for use in breadcrumbs.
 *
 * @todo Make fully recursive...
 *
 * @param string $pid
 *   The object id whose parent will be fetched for the next link.
 * @param FedoraRepository $repository
 *   The fedora repository.
 * @param array $context
 *   An associative array of context for internal use when recursing. Currently
 *   only used to track a single value:
 *   - level: The number of child-parent relationships to follow. Defaults to
 *     10.
 *
 * @return array
 *   An array of links representing the breadcrumb trail, "root" first.
 */
function islandora_get_breadcrumbs_recursive($pid, FedoraRepository $repository, array &$context = NULL) {
  // Before executing the query, we have a base case of accessing the top-level
  // collection.
  if ($context === NULL) {
    $context['level'] = 10;
  }

  $root = variable_get('islandora_repository_pid', 'islandora:root');
  if ($pid == $root) {
    $item = menu_get_item('islandora');
    return array(
      l(t('Home'), '<front>'),
      l($item['title'], 'islandora'),
    );
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
                      minus $parentObject <mulgara:is> <info:fedora/' . $pid . '>
                      order by $title desc';
    $results = $repository->ri->itqlQuery($query_string);

    if (count($results) > 0  && $context['level'] > 0) {
      $parent = $results[0]['parentObject']['value'];
      $this_title = $results[0]['title']['value'];

      if (empty($this_title)) {
        $this_title = t('-');
      }

      $context['level']--;
      return array_merge(
        islandora_get_breadcrumbs_recursive($parent, $repository, $context),
        array(
          l($this_title, "islandora/object/$pid"),
        )
      );
    }
    else {
      // Add an non-link, as we don't know how to get back to the root, and
      // render the last two links and break (on the next pass).
      return array_merge(
        islandora_get_breadcrumbs_recursive($root, $repository, $context),
        array(
          '...',
        )
      );
    }
  }
}
