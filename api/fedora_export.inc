<?php

// $Id$

/**
 * @file
 * Fedora Export
 */
define('FOXML_10', 'info:fedora/fedora-system:FOXML-1.0');
define('FOXML_11', 'info:fedora/fedora-system:FOXML-1.1');
define('METS_10', 'info:fedora/fedora-system:METSFedoraExt-1.0');
define('METS_11', 'info:fedora/fedora-system:METSFedoraExt-1.1');
define('ATOM_11', 'info:fedora/fedora-system:ATOM-1.1');
define('ATOMZip_11', 'info:fedora/fedora-system:ATOMZip-1.1');

/**
 * Function to to export all objects assocoiated with a given pid to the export area
 * @param type $pid
 * @param type $foxml_dir
 * @param type $ob_dir
 * @param type $log
 * @return type 
 */
function export_to_export_area($pid, $foxml_dir, $ob_dir, &$log = array()) {
  if (!$paths = export_objects_for_pid($pid, $ob_dir, $log)) {
    return FALSE;
  }

  if (!export_foxml_for_pid($pid, $foxml_dir, $paths, $log)) {
    return FALSE;
  }

  return TRUE;
}

/**
 * Export objects for pids ??
 * @param type $pid
 * @param type $dir
 * @param type $log
 * @return string 
 */
function export_objects_for_pid($pid, $dir, &$log) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  if (!$object = $item->get_datastreams_list_as_SimpleXML($pid)) {
    $log[] = log_line(t("Failed to get datastream %dsid for pid %pid", array('%dsid' => $ds->ID, '%pid' => $pid)), 'error');
    return FALSE;
  }

  // Datastreams added as a result of the ingest process
  $ignore_dsids = array('QUERY');

  $paths = array();
  foreach ($object->datastreamDef as $ds) {
    if (!in_array($ds->ID, $ignore_dsids)) {
      $file = $dir . '/' . $ds->label . '.' . get_file_extension($ds->MIMEType);
      $paths[$ds->ID] = $file;

      //$content = $ob_helper->getDatastreamDissemination($pid, $ds->ID);
      if ($content = $ob_helper->getStream($pid, $ds->ID, FALSE)) {
        if (!$fp = @fopen($file, 'w')) {
          $log[] = log_line(t("Failed to open file %file to write datastream %dsid for pid %pid", array('%file' => $file, '%dsid' => $ds->ID, '%pid' => $pid)), 'error');
          return FALSE;
        }
        fwrite($fp, $content);
        fclose($fp);
      }
      else {
        $log[] = log_line(t("Failed to get datastream %dsid for pid %pid", array('%dsid' => $ds->ID, '%pid' => $pid)), 'error');
      }
    }
  }
  return $paths;
}

/**
 * Export foxml for pid
 * @param type $pid
 * @param type $dir
 * @param type $paths
 * @param type $log
 * @param type $format
 * @param type $remove_islandora
 * @return type 
 */
function export_foxml_for_pid($pid, $dir, $paths, &$log, $format = FOXML_11, $remove_islandora = FALSE) {
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  $ob_helper = new ObjectHelper();
  if (!$object_xml = $ob_helper->getObject($pid, 'migrate', $format)) {
    $log[] = log_line(t("Failed to get foxml for %pid", array('%pid' => $pid)), 'error');
    return FALSE;
  }

  $foxml = new DOMDocument();
  $foxml->loadXML($object_xml);

  $xpath = new DOMXpath($foxml);

  // Remove rdf elements added during ingest (if present)
  if ($remove_islandora) {
    $xpath->registerNamespace('rdf', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
    $descNode = $xpath->query("//rdf:RDF/rdf:Description")->item(0);

    if ($model = $descNode->getElementsByTagName('hasModel')->item(0)) {
      $descNode->removeChild($model);
    }

    if ($member = $descNode->getElementsByTagName('rel:isMemberOfCollection')->item(0)) {
      $descNode->removeChild($member);
    }
  }

  if ($remove_islandora) {
    // Update object paths in the foxml for this pid
    switch ($format) {
      case FOXML_10:
      case FOXML_11:

        $disallowed_groups = array('E', 'R');

        // Update datastream uris
        $xpath->registerNamespace('foxml', 'info:fedora/fedora-system:def/foxml#');
        foreach ($xpath->query("//foxml:datastream[@ID]") as $dsNode) {

          // Don't update datastreams having external uris
          if (in_array($dsNode->getAttribute('CONTROL_GROUP'), $disallowed_groups)) {
            continue;
          }

          $dsId = $dsNode->getAttribute('ID');

          // Remove QUERY datastream
          if ($dsId == "QUERY") {
            $parentNode = $xpath->query('/foxml:digitalObject')->item(0);
            $parentNode->removeChild($dsNode);
          }

          foreach ($dsNode->getElementsByTagName('*') as $contentNode) {
            if ($str = $contentNode->getAttribute('REF')) {
              $contentNode->setAttribute('REF', url($paths[$dsId], array('absolute' => TRUE)));
            }
          }
        }
        break;

      case METS_10:
      case METS_11:
        // Update datastream uris
        $xpath->registerNamespace('METS', 'http://www.loc.gov/METS/');
        foreach ($xpath->query('//METS:fileGrp[@ID="DATASTREAMS"]/METS:fileGrp') as $dsNode) {

          $dsId = $dsNode->getAttribute('ID');

          // Remove QUERY datastream
          if ($dsId == "QUERY") {
            $parentNode = $xpath->query('//METS:fileGrp[@ID="DATASTREAMS"]')->item(0);
            $parentNode->removeChild($dsNode);
          }

          $xpath->registerNamespace('xlink', 'http://www.loc.gov/METS/');
          foreach ($xpath->query('METS:file[@OWNERID!="E"][@OWNERID!="R"]/METS:FLocat[@xlink:href]', $dsNode) as $Floc) {
            $Floc->setAttribute('xlink:href', url($paths[$dsId], array('absolute' => TRUE)));
          }
          /*
            foreach ($dsNode->getElementsByTagName('METS:file') as $contentNode) {
            // Don't update datastreams having external uris
            if (in_array($dsNode->getAttribute('OWNERID'), $disallowed_groups)) {
            continue;
            }

            foreach ($xpath->('METS:FLocat[@xlink:href]', $contentNode) as $Floc) {
            $Floc->setAttribute('xlink:href', url($paths[$dsId], array('absolute' => true)));
            }
            `}
           */
        }

        break;

      default:
        $log[] = log_line(t("Unknown or invalid format: " . $format), 'error');
        return FALSE;
    }
  } //if $remove_islandora

  $file = $dir . '/' . $pid . '.xml';
  if (!$foxml->save($file)) {
    $log[] = log_line(t("Failed to write datastream %dsid for pid %pid to %file", array('%dsid' => $ds->ID, '%pid' => $pid, '%file' => $file)), 'error');
    return FALSE;
  }
  else {
    $log[] = log_line(t("Exported %pid to %file", array('%pid' => $pid, '%file' => $file)), 'info');
  }

  return TRUE;
}

/**
 * Get file extension
 * @param type $mimeType
 * @return type 
 */
function get_file_extension($mimeType) {
  return substr(strstr($mimeType, '/'), 1);
}

/**
 * Log line
 * @param type $msg
 * @param type $severity
 * @param type $sep
 * @return type 
 */
function log_line($msg, $severity = 'info', $sep = "\t") {
  return date("Y-m-d H:i:s") . $sep . ucfirst($severity) . $sep . $msg;
}
