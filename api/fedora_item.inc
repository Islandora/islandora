<?php

/**
 * @file 
 * Fedora Item
 */
define('RELS_EXT_URI', 'info:fedora/fedora-system:def/relations-external#');
define("FEDORA_MODEL_URI", 'info:fedora/fedora-system:def/model#');

/**
 * Fedora Item Class
 */
class Fedora_Item {

  public $pid = NULL; // The $pid of the fedora object represented by an instance of this class.
  public $objectProfile = NULL;
  private $datastreams_list = NULL; // A SimpleXML object to store a list of this item's datastreams
  public $datastreams = NULL;
  private static $connection_helper = NULL;
  private static $instantiated_pids = array();

  /**
   * Create an object to represent an item in the Fedora repository.
   * Throws a SOAPException if the PID is not in the repository.
   *
   * @param string $pid
   * @return Fedora_Item
   */
  function __construct($pid) {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

    $this->pid = $pid;
    if (isset(Fedora_Item::$instantiated_pids[$pid])) {
      $this->objectProfile = & Fedora_Item::$instantiated_pids[$pid]->objectProfile;
      $this->datastreams = & Fedora_Item::$instantiated_pids[$pid]->datastreams;
      $this->datastreams_list = & Fedora_Item::$instantiated_pids[$pid]->datastreams_list;
    }
    else {
      if (empty(self::$connection_helper)) {
        self::$connection_helper = new ConnectionHelper();
      }

      $raw_objprofile = $this->soap_call('getObjectProfile', array('pid' => $this->pid, 'asOfDateTime' => ""));

      if (!empty($raw_objprofile)) {
        $this->objectProfile = $raw_objprofile->objectProfile;
        $this->datastreams = $this->get_datastreams_list_as_array();
      }
      else {
        $this->objectProfile = '';
        $this->datastreams = array();
      }
      Fedora_Item::$instantiated_pids[$pid] = &$this;
    }
  }

  /**
   * Exists 
   * @return type 
   */
  function exists() {
    return (!empty($this->objectProfile));
  }

  /**
   * Add datastream from file
   * @param type $datastream_file
   * @param type $datastream_id
   * @param type $datastream_label
   * @param type $datastream_mimetype
   * @param type $controlGroup
   * @param type $logMessage
   * @return type 
   */
  function add_datastream_from_file($datastream_file, $datastream_id, $datastream_label = NULL, $datastream_mimetype = '', $controlGroup = 'M', $logMessage = NULL) {
    module_load_include('inc', 'fedora_repository', 'MimeClass');
    if (!is_file($datastream_file)) {
      drupal_set_message("$datastream_file not found<br />", 'warning');
      return;
    }
    if (empty($datastream_mimetype)) {
      // Get mime type from the file extension.
      $mimetype_helper = new MimeClass();
      $datastream_mimetype = $mimetype_helper->getType($datastream_file);
    }
    $original_path = $datastream_file;
    // Temporarily move file to a web-accessible location.
    file_copy($datastream_file, file_directory_path());
    $datastream_url = drupal_urlencode($datastream_file);
    $url = file_create_url($datastream_url);

    $return_value = $this->add_datastream_from_url($url, $datastream_id, $datastream_label, $datastream_mimetype, $controlGroup, $logMessage);

    if ($original_path != $datastream_file) {
      file_delete($datastream_file);
    }
    return $return_value;
  }

  /**
   * Add datastream from url
   * @param type $datastream_url
   * @param type $datastream_id
   * @param type $datastream_label
   * @param type $datastream_mimetype
   * @param type $controlGroup
   * @param type $logMessage
   * @return type 
   */
  function add_datastream_from_url($datastream_url, $datastream_id, $datastream_label = NULL, $datastream_mimetype = '', $controlGroup = 'M', $logMessage = NULL) {
    if (empty($datastream_label)) {
      $datastream_label = $datastream_id;
    }

    $params = array(
      'pid' => $this->pid,
      'dsID' => $datastream_id,
      'altIDs' => NULL,
      'dsLabel' => $datastream_label,
      'versionable' => TRUE,
      'MIMEType' => $datastream_mimetype,
      'formatURI' => NULL,
      'dsLocation' => $datastream_url,
      'controlGroup' => $controlGroup,
      'dsState' => 'A',
      'checksumType' => 'DISABLED',
      'checksum' => 'none',
      'logMessage' => ($logMessage != NULL) ? $logMessage : 'Ingested object ' . $datastream_id
    );


    return $this->soap_call('addDataStream', $params);
  }

  /**
   * Add datastream from string
   * @param type $str
   * @param type $datastream_id
   * @param type $datastream_label
   * @param type $datastream_mimetype
   * @param type $controlGroup
   * @param type $logMessage
   * @return type 
   */
  function add_datastream_from_string($str, $datastream_id, $datastream_label = NULL, $datastream_mimetype = 'text/xml', $controlGroup = 'M', $logMessage = NULL) {
    $dir = file_directory_temp();
    $tmpfilename = tempnam($dir, 'fedoratmp');
    $tmpfile = fopen($tmpfilename, 'w');
    fwrite($tmpfile, $str, strlen($str));
    fclose($tmpfile);
    $returnvalue = $this->add_datastream_from_file($tmpfilename, $datastream_id, $datastream_label, $datastream_mimetype, $controlGroup, $logMessage);
    unlink($tmpfilename);
    return $returnvalue;
  }

  /**
   * Add a relationship string to this object's RELS-EXT.
   * does not support rels-int yet.
   * @param type $relationship
   * @param type $object
   * @param type $namespaceURI 
   */
  function add_relationship($relationship, $object, $namespaceURI = RELS_EXT_URI) {
    $ds_list = $this->get_datastreams_list_as_array();

    if (empty($ds_list['RELS-EXT'])) {
      $this->add_datastream_from_string('        <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
          <rdf:Description rdf:about="info:fedora/' . $this->pid . '">
          </rdf:Description>
        </rdf:RDF>', 'RELS-EXT', 'Fedora object-to-object relationship metadata', 'text/xml', 'X');
    }

    $relsext = $this->get_datastream_dissemination('RELS-EXT');

    if (substr($object, 0, 12) != 'info:fedora/') {
      $object = "info:fedora/$object";
    }

    $relsextxml = new DomDocument();

    $relsextxml->loadXML($relsext);
    $description = $relsextxml->getElementsByTagNameNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'Description');
    if ($description->length == 0) {
      $description = $relsextxml->getElementsByTagNameNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'description');
    }
    $description = $description->item(0);

    // Create the new relationship node.
    $newrel = $relsextxml->createElementNS($namespaceURI, $relationship);

    $newrel->setAttribute('rdf:resource', $object);

    $description->appendChild($newrel);
    $this->modify_datastream_by_value($relsextxml->saveXML(), 'RELS-EXT', "Fedora Object-to-Object Relationship Metadata", 'text/xml');
    //print ($description->dump_node());
    /*
      $params = array( 'pid' => $this->pid,
      'relationship' => $relationship,
      'object' => $object,
      'isLiteral' => FALSE,
      'datatype' => '',
      );

      return $this->soap_call( 'addRelationship', $params );
     */
  }

  /**
   * Removes the given relationship from the item's RELS-EXT and re-saves it.
   * @param string $relationship
   * @param string $object
   */
  function purge_relationship($relationship, $object) {
    $relsext = $this->get_datastream_dissemination('RELS-EXT');
    $namespaceURI = 'info:fedora/fedora-system:def/relations-external#';
    // Pre-pend a namespace prefix to recognized relationships

    switch ($relationship) {
      case 'rel:isMemberOf':
      case 'fedora:isMemberOf':
        $relationship = "isMemberOf";
        $namespaceURI = 'info:fedora/fedora-system:def/relations-external#';
        break;
      case "rel:isMemberOfCollection":
      case "fedora:isMemberOfCollection":
        $relationship = "isMemberOfCollection";
        $namespaceURI = 'info:fedora/fedora-system:def/relations-external#';
        break;
      case "fedora:isPartOf":
        $relationship = "isPartOf";
        $namespaceURI = 'info:fedora/fedora-system:def/relations-external#';
        break;
      case "rel:hasModel":
      case "hasModel":
        $relationship = "hasModel";
        $namespaceURI = FEDORA_MODEL_URI;
        break;
    }

    if (substr($object, 0, 12) != 'info:fedora/') {
      $object = "info:fedora/$object";
    }

    $relsextxml = new DomDocument();
    $relsextxml->loadXML($relsext);
    $modified = FALSE;
    $rels = $relsextxml->getElementsByTagNameNS($namespaceURI, $relationship);
    if (!empty($rels)) {
      foreach ($rels as $rel) {
        if ($rel->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource') == $object) {
          $rel->parentNode->removeChild($rel);
          $modified = TRUE;
        }
      }
    }
    if ($modified) {
      $this->modify_datastream_by_value($relsextxml->saveXML(), 'RELS-EXT', "Fedora Object-to-Object Relationship Metadata", 'text/xml');
    }
    return $modified;
    //print ($description->dump_node());
  }

  /**
   * Export as foxml
   * @return type 
   */
  function export_as_foxml() {
    $params = array(
      'pid' => $this->pid,
      'format' => 'info:fedora/fedora-system:FOXML-1.1',
      'context' => 'migrate',
    );
    $result = self::soap_call('export', $params);
    return $result->objectXML;
  }

  /**
   * Does a search using the "query" format followed by the Fedora REST APi.
   *
   * @param string $pattern to search for, including wildcards.
   * @param string $field The field to search on, e.g. pid, title, cDate. See http://www.fedora-commons.org/confluence/display/FCR30/REST+API#RESTAPI-findObjects for details
   * @param int $max_results not used at this time
   * @return Array of pid => title pairs that match the results
   */
  static function find_objects_by_pattern($pattern = '*', $field = 'pid', $max_results = 100, $resultFields = array()) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

    $pattern = drupal_urlencode($pattern);
    $done = FALSE;
    $cursor = 0;
    $session_token = '';
    $i = 0;
    $results = array();
    while (!$done && $i < 5) {
      $i++;
      $url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');
      if ($cursor == 0) {
        $url .= "/objects?query=$field~$pattern&pid=TRUE&title=TRUE&resultFormat=xml&maxResults=$max_results";
      }
      else {
        $url .= "/objects?pid=TRUE&title=TRUEsessionToken=$session_token&resultFormat=xml&maxResults=$max_results";
      }

      if (count($resultFields) > 0) {
        $url .= '&' . join('=TRUE&', $resultFields) . '=TRUE';
      }

      $resultxml = do_curl($url);

      libxml_use_internal_errors(TRUE);
      $resultelements = simplexml_load_string($resultxml);
      if ($resultelements === FALSE) {
        libxml_clear_errors();
        break;
      }
      $cursor += count($resultelements->resultList->objectFields);
      if (count($resultelements->resultList->objectFields) < $max_results
          || count($resultelements->resultList->objectFields) == 0) {
        $done = TRUE;
      }
      foreach ($resultelements->resultList->objectFields as $obj) {

        $ret = (string) $obj->title;
        if (count($resultFields) > 0) {
          $ret = array('title' => $ret);
          foreach ($resultFields as $field) {
            $ret[$field] = (string) $obj->$field;
          }
        }
        $results[(string) $obj->pid] = $ret;
        $cursor++;
        if ($cursor >= $max_results) {
          $done = TRUE;
          break;
        }
      }
      $session_token = $resultelements->listSession->token;
      $done = !empty($session_token);
    }
    return $results;
  }

  /**
   * Get datastream dissemination
   * @param type $dsid
   * @param type $as_of_date_time
   * @return string 
   */
  function get_datastream_dissemination($dsid, $as_of_date_time = "") {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'asOfDateTime' => $as_of_date_time,
    );
    $object = self::soap_call('getDataStreamDissemination', $params);
    if (!empty($object)) {
      $content = $object->dissemination->stream;
      $content = trim($content);
    }
    else {
      $content = NULL;
    }
    return $content;
  }

  /**
   * Get datastream
   * @param type $dsid
   * @param type $as_of_date_time
   * @return type 
   */
  function get_datastream($dsid, $as_of_date_time = "") {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'asOfDateTime' => $as_of_date_time,
    );
    $object = self::soap_call('getDatastream', $params);

    return $object->datastream;
  }

  /**
   * Get datastream history
   * @param type $dsid
   * @return type 
   */
  function get_datastream_history($dsid) {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid
    );
    $object = self::soap_call('getDatastreamHistory', $params);
    $ret = FALSE;
    if (!empty($object)) {
      $ret = $object->datastream;
    }

    return $ret;
  }

  /**
   * Get dissemination
   * @param type $service_definition_pid
   * @param type $method_name
   * @param type $parameters
   * @param type $as_of_date_time
   * @return string 
   */
  function get_dissemination($service_definition_pid, $method_name, $parameters = array(), $as_of_date_time = NULL) {
    $params = array(
      'pid' => $this->pid,
      'serviceDefinitionPid' => $service_definition_pid,
      'methodName' => $method_name,
      'parameters' => $parameters,
      'asOfDateTime' => $as_of_date_time,
    );
    $object = self::soap_call('getDissemination', $params);
    if (!empty($object)) {
      $content = $object->dissemination->stream;
      $content = trim($content);
    }
    else {
      $content = "";
    }
    return $content;
  }

  /**
   * Retrieves and returns a SimpleXML list of this item's datastreams, and stores them
   * as an instance variable for caching purposes.
   *
   * @return SimpleXMLElement
   */
  function get_datastreams_list_as_SimpleXML() {
    //if ( empty( $this->datastreams_list ) ) {
    $params = array(
      'pid' => $this->pid,
      'asOfDateTime' => ""
    );

    $this->datastreams_list = $this->soap_call('listDataStreams', $params);
    //}
    return $this->datastreams_list;
  }

  /**
   *     *  DatastreamControlGroup controlGroup - String restricted to the values of "X", "M", "R", or "E" (InlineXML,Managed Content,Redirect, or External Referenced).
   * String ID - The datastream ID (64 characters max).
   * String versionID - The ID of the most recent datastream version
   * String[] altIDs - Alternative IDs for the datastream, if any.
   * String label - The Label of the datastream.
   * boolean versionable - Whether the datastream is versionable.
   * String MIMEType - The mime-type for the datastream, if set.
   * String formatURI - The format uri for the datastream, if set.
   * String createDate - The date the first version of the datastream was created.
   * long size - The size of the datastream in Fedora. Only valid for inline XML metadata and managed content datastreams.
   * String state - The state of the datastream. Will be "A" (active), "I" (inactive) or "D" (deleted).
   * String location - If the datastream is an external reference or redirect, the url to the contents. TODO: Managed?
   * String checksumType - The algorithm used to compute the checksum. One of "DEFAULT", "DISABLED", "MD5", "SHA-1", "SHA-256", "SHA-385", "SHA-512".
   * String checksum - The value of the checksum represented as a hexadecimal string.

   *
   * @param string $dsid
   * @return datastream object
   * get the mimetype size etc. in one shot.  instead of iterating throught the datastream list for what we need
   */
  function get_datastream_info($dsid, $as_of_date_time = "") {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'asOfDateTime' => $as_of_date_time
    );

    return $this->soap_call('getDatastream', $params);
  }

  /**
   * Returns an associative array of this object's datastreams. Results look like this:
   *
   *  'DC' =>
   *    array
   *      'label' => string 'Dublin Core Record for this object' (length=34)
   *      'MIMEType' => string 'text/xml' (length=8)
   *  'RELS-EXT' =>
   *    array
   *      'label' => string 'RDF Statements about this object' (length=32)
   *      'MIMEType' => string 'application/rdf+xml' (length=19)
   *
   * @return array
   */
  function get_datastreams_list_as_array() {
    $this->get_datastreams_list_as_SimpleXML();
    $ds_list = array();
    if ($this->datastreams_list != FALSE) {

      // This is a really annoying edge case: if there is only one
      // datastream, instead of returning it as an array, only
      // the single item will be returned directly. We have to
      // check for this.
      if (count($this->datastreams_list->datastreamDef) >= 2) {
        foreach ($this->datastreams_list->datastreamDef as $ds) {
          if (!is_object($ds)) {
            print_r($ds);
          }
          $ds_list[$ds->ID]['label'] = $ds->label;
          $ds_list[$ds->ID]['MIMEType'] = $ds->MIMEType;
          $ds_list[$ds->ID]['URL'] = $this->url() . '/' . $ds->ID . '/' . drupal_urlencode($ds->label);
        }
      }
      else {
        $ds = $this->datastreams_list->datastreamDef;
        $ds_list[$ds->ID]['label'] = $ds->label;
        $ds_list[$ds->ID]['MIMEType'] = $ds->MIMEType;
        $ds_list[$ds->ID]['URL'] = $this->url() . '/' . $ds->ID . '/' . drupal_urlencode($ds->label);
      }
    }

    return $ds_list;
  }

  /**
   * Returns a MIME type string for the given Datastream ID.
   *
   * @param string $dsid
   * @return string
   */
  function get_mimetype_of_datastream($dsid) {
    $this->get_datastreams_list_as_SimpleXML();

    $mimetype = '';
    foreach ($datastream_list as $datastream) {
      foreach ($datastream as $datastreamValue) {
        if ($datastreamValue->ID == $dsid) {
          return $datastreamValue->MIMEType;
        }
      }
    }

    return '';
  }

  /**
   * Currently the Fedora API call getRelationships is reporting an uncaught
   * exception so we will parse the RELS-EXT ourselves and simulate the
   * documented behaviour.
   * @param String $relationship - filter the results to match this string.
   */
  function get_relationships($relationship = NULL) {
    $relationships = array();
    try {
      $relsext = $this->get_datastream_dissemination('RELS-EXT');
    } catch (exception $e) {
      drupal_set_message(t("Error retrieving RELS-EXT of object $pid"), 'error');
      return $relationships;
    }

    // Parse the RELS-EXT into an associative array.
    $relsextxml = new DOMDocument();
    $relsextxml->loadXML($relsext);
    $relsextxml->normalizeDocument();
    $rels = $relsextxml->getElementsByTagNameNS('info:fedora/fedora-system:def/relations-external#', '*');
    foreach ($rels as $child) {
      if (empty($relationship) || preg_match("/$relationship/", $child->tagName)) {
        $relationships[] = array(
          'subject' => $this->pid,
          'predicate' => $child->tagName,
          'object' => substr($child->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource'), 12),
        );
      }
    }

    return $relationships;
  }


    function get_models() {
    $relationships = array();
    try {
      $relsext = $this->get_datastream_dissemination('RELS-EXT');
    } catch (exception $e) {
      drupal_set_message(t("Error retrieving RELS-EXT of object $pid"), 'error');
      return $relationships;
    }

    // Parse the RELS-EXT into an associative array.
    $relsextxml = new DOMDocument();
    $relsextxml->loadXML($relsext);
    $relsextxml->normalizeDocument();
    $mods = $relsextxml->getElementsByTagNameNS(FEDORA_MODEL_URI, '*');
    foreach ($mods as $child) {
      if (empty($relationship) || preg_match("/$relationship/", $child->tagName)) {
        $relationships[] = array(
          'subject' => $this->pid,
          'predicate' => $child->tagName,
          'object' => substr($child->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource'), 12),
        );
      }
    }

    return $relationships;
  }

  /**
   * Creates a RELS-EXT XML stream from the supplied array and saves it to
   * the item on the server.
   * @param <type> $relationships
   */
  function save_relationships($relationships) {
    // Verify the array format and that it isn't empty.
    if (!empty($relationships)) {
      $relsextxml = '<rdf:RDF xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#">'
          . '<rdf:description rdf:about="' . $this->pid . '">';

      foreach ($relationships as $rel) {
        if (empty($rel['subject']) || empty($rel['predicate']) || empty($rel['object']) || $rel['subject'] != 'info:fedora/' . $this->pid) {
          // drupal_set_message should use parameterized variables, not interpolated.
          drupal_set_message(t("Error with relationship format: " . $rel['subject'] . " - " . $rel['predicate'] . ' - ' . $rel['object']), "error");
          return FALSE;
        }
      }
    }

    // Do the messy work constructing the RELS-EXT XML. Because addRelationship is broken.

    return FALSE;
  }

  /**
   * Removes this object form the repository.
   * @param type $log_message
   * @param type $force
   * @return type 
   */
  function purge($log_message = 'Purged using Islandora API.', $force = FALSE) {
    $params = array(
      'pid' => $this->pid,
      'logMessage' => $log_message,
      'force' => $force
    );

    return $this->soap_call('purgeObject', $params);
  }

  /**
   * Purge datastream
   * @param type $dsID
   * @param type $start_date
   * @param type $end_date
   * @param type $log_message
   * @param type $force
   * @return type 
   */
  function purge_datastream($dsID, $start_date = NULL, $end_date = NULL, $log_message = 'Purged datastream using Islandora API', $force = FALSE) {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsID,
      'startDT' => $start_date,
      'endDT' => $end_date,
      'logMessage' => $log_message,
      'force' => $force,
    );
    return $this->soap_call('purgeDatastream', $params);
  }

  /**
   * URL
   * @global type $base_url
   * @return type 
   */
  function url() {
    global $base_url;
    return $base_url . '/fedora/repository/' . $this->pid . (!empty($this->objectProfile) ? '/-/' . drupal_urlencode($this->objectProfile->objLabel) : '');
  }

  /**
   * Get Next PID in Namespace
   * @param type $pid_namespace
   * @return type 
   */
  static function get_next_PID_in_namespace($pid_namespace = '') {

    if (empty($pid_namespace)) {
      // Just get the first one in the config settings.
      $allowed_namespaces = explode(" ", variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '));
      $pid_namespace = $allowed_namespaces[0];
      if (!empty($pid_namespace)) {
        $pid_namespace = substr($pid_namespace, 0, strpos($pid_namespace, ":"));
      }
      else {
        $pid_namespace = 'default';
      }
    }

    $params = array(
      'numPIDs' => '',
      'pidNamespace' => $pid_namespace,
    );

    $result = self::soap_call('getNextPID', $params);
    return $result->pid;
  }

  /**
   * ingest from FOXML
   * @param type $foxml
   * @return Fedora_Item 
   */
  static function ingest_from_FOXML($foxml) {
    $params = array('objectXML' => $foxml->saveXML(), 'format' => "info:fedora/fedora-system:FOXML-1.1", 'logMessage' => "Fedora Object Ingested");
    $object = self::soap_call('ingest', $params);
    return new Fedora_Item($object->objectPID);
  }

  /**
   * ingest from FOXML file
   * @param type $foxml_file
   * @return type 
   */
  static function ingest_from_FOXML_file($foxml_file) {
    $foxml = new DOMDocument();
    $foxml->load($foxml_file);
    return self::ingest_from_FOXML($foxml);
  }

  /**
   * ingest from FOXML files in directory
   * @param type $path 
   */
  static function ingest_from_FOXML_files_in_directory($path) {
    // Open the directory
    $dir_handle = @opendir($path);
    // Loop through the files
    while ($file = readdir($dir_handle)) {
      if ($file == "." || $file == ".." || strtolower(substr($file, strlen($file) - 4)) != '.xml') {
        continue;
      }

      try {
        self::ingest_from_FOXML_file($path . '/' . $file);
      } catch (exception $e) {
        
      }
    }
    // Close
    closedir($dir_handle);
  }

  /**
   * Modify Object
   * @param type $label
   * @param type $state
   * @param type $ownerId
   * @param type $logMessage
   * @param type $quiet
   * @return type 
   */
  function modify_object($label = '', $state = NULL, $ownerId = NULL, $logMessage = 'Modified by Islandora API', $quiet=TRUE) {

    $params = array(
      'pid' => $this->pid,
      'ownerId' => $ownerId,
      'state' => $state,
      'label' => $label,
      'logMessage' => $logMessage
    );

    return self::soap_call('modifyObject', $params, $quiet);
  }

  /**
   * Modify datastream by reference
   * @param type $external_url
   * @param type $dsid
   * @param type $label
   * @param type $mime_type
   * @param type $force
   * @param type $logMessage
   * @param type $quiet
   * @return type 
   */
  function modify_datastream_by_reference($external_url, $dsid, $label, $mime_type, $force = FALSE, $logMessage = 'Modified by Islandora API', $quiet=FALSE) {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'altIDs' => NULL,
      'dsLabel' => $label,
      'MIMEType' => $mime_type,
      'formatURI' => NULL,
      'dsLocation' => $external_url,
      'checksumType' => 'DISABLED',
      'checksum' => 'none',
      'logMessage' => $logMessage,
      'force' => $force
    );
    return self::soap_call('modifyDatastreamByReference', $params, $quiet);
  }

  /**
   * Modify datastream by value
   * @param type $content
   * @param type $dsid
   * @param type $label
   * @param type $mime_type
   * @param type $force
   * @param type $logMessage
   * @param type $quiet
   * @return type 
   */
  function modify_datastream_by_value($content, $dsid, $label, $mime_type, $force = FALSE, $logMessage = 'Modified by Islandora API', $quiet=FALSE) {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'altIDs' => NULL,
      'dsLabel' => $label,
      'MIMEType' => $mime_type,
      'formatURI' => NULL,
      'dsContent' => $content,
      'checksumType' => 'DISABLED',
      'checksum' => 'none',
      'logMessage' => $logMessage,
      'force' => $force
    );
    return self::soap_call('modifyDatastreamByValue', $params, $quiet);
  }

  function set_datastream_state($dsid, $state, $log_message = 'Modified by Islandora API', $quiet = FALSE) {
    $valid_states = array('A', 'D', 'I');
    if (array_search($state, $valid_states) !== FALSE) {
      $params = array(
        'pid' => $this->pid,
        'dsID' => $dsid,
        'dsState' => $state,
        'logMessage' => $log_message,
      );
      return self::soap_call('setDatastreamState', $params, $quiet);
    }
  }

  /**
   * Soap call
   * @param type $function_name
   * @param type $params_array
   * @param type $quiet
   * @return type 
   */
  static function soap_call($function_name, $params_array, $quiet = FALSE) {
    if (!self::$connection_helper) {
      module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
      self::$connection_helper = new ConnectionHelper();
    }
    switch ($function_name) {
      case 'ingest':
      case 'addDataStream':
      case 'addRelationship':
      case 'export':
      case 'getDatastream':
      case 'getDatastreamHistory':
      case 'getNextPID':
      case 'getRelationships':
      case 'modifyDatastreamByValue':
      case 'modifyDatastreamByReference':
      case 'purgeDatastream':
      case 'purgeObject':
      case 'modifyObject':
      case 'setDatastreamState':
        $soap_client = self::$connection_helper->getSoapClient(variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/services/management?wsdl'));
        try {
          if (!empty($soap_client)) {
            $result = $soap_client->__soapCall($function_name, array('parameters' => $params_array));
          }
          else {
            watchdog(t("FEDORA_REPOSITORY"), "Error trying to get SOAP client connection.");
            return NULL;
          }
        } catch (exception $e) {
          if (!$quiet) {

            if (preg_match('/org\.fcrepo\.server\.security\.xacml\.pep\.AuthzDeniedException/', $e->getMessage())) {
              drupal_set_message(t('Error:  Insufficient permissions to call SOAP function !fn.', array('!fn' => $function_name)), 'error');
            }
            else {
              drupal_set_message(t("Error trying to call SOAP function $function_name. Check watchdog logs for more information."), 'error');
            }

            watchdog(t("FEDORA_REPOSITORY"), "Error Trying to call SOAP function @fn: @e", array('@fn' => $function_name, '@e' => $e), NULL, WATCHDOG_ERROR);
          }
          return NULL;
        }
        break;

      default:
        try {
          $soap_client = self::$connection_helper->getSoapClient(variable_get('fedora_soap_url', 'http://localhost:8080/fedora/services/access?wsdl'));
          if (!empty($soap_client)) {
            $result = $soap_client->__soapCall($function_name, array('parameters' => $params_array));
          }
          else {
            watchdog(t("FEDORA_REPOSITORY"), "Error trying to get SOAP client connection.");
            return NULL;
          }
        } catch (exception $e) {

          if (!$quiet) {
            watchdog(t("FEDORA_REPOSITORY"), "Error trying to call SOAP function @fn: @e", array('@fn' => $function_name, '@e' => $e), NULL, WATCHDOG_ERROR);
          }
          return NULL;
        }
    }
    return $result;
  }

  /**
   * Creates the minimal FOXML for a new Fedora object, which is then passed to
   * ingest_from_FOXML to be added to the repository.
   *
   * @param string $pid if none given, getnextpid will be called.
   * @param string $state The initial state, A - Active, I - Inactive, D - Deleted
   * @param type $label
   * @param type $owner
   * @return DOMDocument 
   */
  static function create_object_FOXML($pid = '', $state = 'A', $label = 'Untitled', $owner = '') {
    $foxml = new DOMDocument("1.0", "UTF-8");
    $foxml->formatOutput = TRUE;
    if (empty($pid)) {
      // Call getNextPid
      $pid = self::get_next_PID_in_namespace();
    }
    if (empty($owner)) {
      if (!empty($user->uid)) { // Default to current Drupal user.
        $owner = $user->uid;
      }
      else { // We are not inside Drupal
        $owner = 'fedoraAdmin';
      }
    }

    $root_element = $foxml->createElement("foxml:digitalObject");
    $root_element->setAttribute("VERSION", "1.1");
    $root_element->setAttribute("PID", $pid);
    $root_element->setAttribute("xmlns:foxml", "info:fedora/fedora-system:def/foxml#");
    $root_element->setAttribute("xmlns:xsl", "http://www.w3.org/2001/XMLSchema-instance");
    $root_element->setAttribute("xsl:schemaLocation", "info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd");
    $foxml->appendChild($root_element);

    // FOXML object properties section
    $object_properties = $foxml->createElement("foxml:objectProperties");
    $state_property = $foxml->createElement("foxml:property");
    $state_property->setAttribute("NAME", "info:fedora/fedora-system:def/model#state");
    $state_property->setAttribute("VALUE", $state);

    $label_property = $foxml->createElement("foxml:property");
    $label_property->setAttribute("NAME", "info:fedora/fedora-system:def/model#label");
    $label_property->setAttribute("VALUE", $label);

    $owner_property = $foxml->createElement("foxml:property");
    $owner_property->setAttribute("NAME", "info:fedora/fedora-system:def/model#ownerId");
    $owner_property->setAttribute("VALUE", $owner);

    $object_properties->appendChild($state_property);
    $object_properties->appendChild($label_property);
    $object_properties->appendChild($owner_property);
    $root_element->appendChild($object_properties);
    $foxml->appendChild($root_element);

    return $foxml;
  }

  /**
   * ingest new item
   * @param type $pid
   * @param type $state
   * @param type $label
   * @param type $owner
   * @return type 
   */
  static function ingest_new_item($pid = '', $state = 'A', $label = '', $owner = '') {
    return self::ingest_from_FOXML(self::create_object_FOXML($pid, $state, $label, $owner));
  }

  /**
   * fedora item exists 
   * @param type $pid
   * @return type 
   */
  static function fedora_item_exists($pid) {
    $item = new Fedora_Item($pid);
    return $item->exists();
  }

  /*   * ******************************************************
   * Relationship Functions
   * ****************************************************** */

  /**
   * Returns an associative array of relationships that this item has
   * in its RELS-EXT.
   */
}

