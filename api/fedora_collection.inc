<?php

/**
 * @file
 * Operations that affect a Fedora repository at a collection level.
 */
module_load_include('inc', 'fedora_repository', 'CollectionClass');
module_load_include('inc', 'fedora_repository', 'api/fedora_item');
module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
module_load_include('module', 'fedora_repository');

/**
 * Exports a fedora collection object and all of its children in a format
 * that will let you import them into another repository.
 * @param type $collection_pid
 * @param type $relationship
 * @param type $format
 * @return type 
 */
function export_collection($collection_pid, $relationship = 'isMemberOfCollection', $format = 'info:fedora/fedora-system:FOXML-1.1') {
  $collection_item = new Fedora_Item($collection_pid);
  $foxml = $collection_item->export_as_foxml();

  $file_dir = file_directory_path();

  // Create a temporary directory to contain the exported FOXML files.
  $container = tempnam($file_dir, 'export_');
  file_delete($container);
  print $container;
  if (mkdir($container) && mkdir($container . '/' . $collection_pid)) {
    $foxml_dir = $container . '/' . $collection_pid;
    $file = fopen($foxml_dir . '/' . $collection_pid . '.xml', 'w');
    fwrite($file, $foxml);
    fclose($file);

    $member_pids = get_related_items_as_array($collection_pid, $relationship);
    foreach ($member_pids as $member) {
      $file = fopen($foxml_dir . '/' . $member . '.xml', 'w');
      $item = new Fedora_Item($member);
      $item_foxml = $item->export_as_foxml();
      fwrite($file, $item_foxml);
      fclose($file);
    }
    if (system("cd $container;zip -r $collection_pid.zip $collection_pid/* >/dev/null") == 0) {
      header("Content-type: application/zip");
      header('Content-Disposition: attachment; filename="' . $collection_pid . '.zip' . '"');
      $fh = fopen($container . '/' . $collection_pid . '.zip', 'r');
      $the_data = fread($fh, filesize($container . '/' . $collection_pid . '.zip'));
      fclose($fh);
      echo $the_data;
    }
    if (file_exists($container . '/' . $collection_pid)) {
      system("rm -rf $container"); // I'm sorry.
    }
  }
  else {
    drupal_set_message(t("Error creating temp directory for batch export."), 'error');
    return FALSE;
  }
  return TRUE;
}

/**
 * Returns an array of pids that match the query contained in teh collection
 * object's QUERY datastream or in the suppled $query parameter.
 * @param <type> $collection_pid
 * @param <type> $query
 * @param <type> $query_format R
 */
function get_related_items_as_xml($collection_pid, $relationship = array('isMemberOfCollection'), $limit = 10000, $offset = 0, $active_objects_only = TRUE, $cmodel = NULL, $orderby = '$title') {
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');

  global $user;
  if (!fedora_repository_access(OBJECTHELPER :: $OBJECT_HELPER_VIEW_FEDORA, $pid, $user)) {
    drupal_set_message(t("You do not have access to Fedora objects within the attempted namespace or access to Fedora denied."), 'error');
    return array();
  }

  $query_string = 'select $object $title $content from <#ri>
                             where ($object <fedora-model:label> $title
                             and $object <fedora-model:hasModel> $content
                             and (';

  if (is_array($relationship)) {
    foreach ($relationship as $rel) {
      $query_string .= '$object <fedora-rels-ext:' . $rel . '> <info:fedora/' . $collection_pid . '>';
      if (next($relationship)) {
        $query_string .= ' OR ';
      }
    }
  }
  elseif (is_string($relationship)) {
    $query_string .= '$object <fedora-rels-ext:' . $relationship . '> <info:fedora/' . $collection_pid . '>';
  }
  else {
    return '';
  }

  $query_string .= ') ';
  $query_string .= $active_objects_only ? 'and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>' : '';

  if ($cmodel) {
    $query_string .= ' and $content <mulgara:is> <info:fedora/' . $cmodel . '>';
  }

  $query_string .= ')
                    minus $content <mulgara:is> <info:fedora/fedora-system:FedoraObject-3.0>
                    order by ' . $orderby;

  $query_string = htmlentities(urlencode($query_string));


  $content = '';
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=$limit&offset=$offset&lang=itql&stream=on&query=" . $query_string;
  $content .= do_curl($url);

  return $content;
}

/**
 * Get Related Items as Arrays
 * @param type $collection_pid
 * @param type $relationship
 * @param type $limit
 * @param type $offset
 * @param type $active_objects_only
 * @param type $cmodel
 * @param type $orderby
 * @return type 
 */
function get_related_items_as_array($collection_pid, $relationship = 'isMemberOfCollection', $limit = 10000, $offset = 0, $active_objects_only = TRUE, $cmodel = NULL, $orderby = '$title') {
  $content = get_related_items_as_xml($collection_pid, $relationship, $limit, $offset, $active_objects_only, $cmodel, $orderby);
  if (empty($content)) {
    return array();
  }

  $content = new SimpleXMLElement($content);

  $resultsarray = array();
  foreach ($content->results->result as $result) {
    $resultsarray[] = substr($result->object->attributes()->uri, 12); // Remove 'info:fedora/'.
  }
  return $resultsarray;
}
