<?php
/**
 * @file
 * Fedora Item
 */

define('RELS_EXT_URI', 'info:fedora/fedora-system:def/relations-external#');
define("FEDORA_MODEL_URI", 'info:fedora/fedora-system:def/model#');
define("ISLANDORA_PAGE_URI", 'info:islandora/islandora-system:def/pageinfo#');
define("ISLANDORA_RELS_EXT_URI", 'http://islandora.ca/ontology/relsext#');
define("ISLANDORA_RELS_INT_URI", "http://islandora.ca/ontology/relsint#");

define("RELS_TYPE_URI", 0);
define("RELS_TYPE_PLAIN_LITERAL", 1);
define("RELS_TYPE_STRING", 2);
define("RELS_TYPE_INT", 3);
define("RELS_TYPE_DATETIME", 4);

/**
 * Fedora Item Class
 */
class Fedora_Item {

  // The $pid of the fedora object represented by an instance of this class.
  public $pid = NULL;
  public $objectProfile = NULL;
  public $ownerId = NULL;

  // A SimpleXML object to store a list of this item's datastreams.
  private $datastreams_list = NULL;
  public $datastreams = NULL;
  private static $connection_helper = NULL;
  private static $instantiated_pids = array();
  private static $SoapManagedFunctions = array(
    'ingest',
    'addDataStream',
    'addRelationship',
    'export',
    'getDatastream',
    'getDatastreamHistory',
    'getNextPID',
    'getRelationships',
    'modifyDatastreamByValue',
    'modifyDatastreamByReference',
    'purgeDatastream',
    'purgeObject',
    'modifyObject',
    'setDatastreamState',
  );

  /**
   * Create an object to represent an item in the Fedora repository.
   * Throws a SOAPException if the PID is not in the repository.
   *
   * @param string $pid
   *   The Fedora PID to create an object for.
   *
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
      $this->ownerId = & Fedora_Item::$instantiated_pids[$pid]->ownerId;
    }
    else {
      if (empty(self::$connection_helper)) {
        self::$connection_helper = new ConnectionHelper();
      }

      $raw_objprofile = $this->soap_call('getObjectProfile', array('pid' => $this->pid, 'asOfDateTime' => ""), TRUE);

      if (!empty($raw_objprofile)) {
        $this->objectProfile = $raw_objprofile->objectProfile;
        $this->datastreams = $this->get_datastreams_list_as_array();
        $this->ownerId = Fedora_Item::getOwnerId($pid);
      }
      else {
        $this->objectProfile = '';
        $this->datastreams = array();
        $this->ownerId = NULL;
      }
      Fedora_Item::$instantiated_pids[$pid] = &$this;
    }
  }

  /**
   * Forget this Object, do manually when memory constraints apply.
   *
   * Removes this object from the static list of $instantiated_pids
   */
  function forget() {
    unset(Fedora_Item::$instantiated_pids[$this->pid]);
  }

  /**
   * Exists
   *
   * @return type
   */
  function exists() {
    return (!empty($this->objectProfile));
  }

  /**
   * Add datastream from file
   *
   * @param type $datastream_file
   * @param type $datastream_id
   * @param type $datastream_label
   * @param type $datastream_mimetype
   * @param type $controlGroup
   * @param type $logMessage
   *
   * @return type
   */
  function add_datastream_from_file($datastream_file, $datastream_id, $datastream_label = NULL, $datastream_mimetype = '', $controlGroup = 'M', $logMessage = NULL) {
    module_load_include('inc', 'fedora_repository', 'MimeClass');
    if (!is_file($datastream_file)) {
      drupal_set_message(t('The datastream file %datastream_file could not be found! (or is not a regular file...)', array('%datastream_file' => $datastream_file)), 'warning');
      return;
    }
    elseif (!is_readable($datastream_file)) {
      drupal_set_message(t('The datastream file %datastream_file could not be read! (likely due to permissions...)', array('%datastream_file' => $datastream_file)), 'warning');
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

    // Add_datastream_from_url forces a re-sync of the datastream list.
    $return_value = $this->add_datastream_from_url($url, $datastream_id, $datastream_label, $datastream_mimetype, $controlGroup, $logMessage);

    if ($original_path != $datastream_file) {
      file_delete($datastream_file);
    }
    return $return_value;
  }

  /**
   * Add datastream from url
   *
   * @param type $datastream_url
   * @param type $datastream_id
   * @param type $datastream_label
   * @param type $datastream_mimetype
   * @param type $controlGroup
   * @param type $logMessage
   *
   * @return type
   */
  function add_datastream_from_url($datastream_url, $datastream_id, $datastream_label = NULL, $datastream_mimetype = '', $controlGroup = 'M', $logMessage = NULL) {
    global $base_url;

    if (empty($datastream_label)) {
      $datastream_label = $datastream_id;
    }

    // Fedora has some problems getting files from HTTPS connections sometimes, so if we are getting a file
    // from the local drupal, we try to pass a HTTP url instead of a HTTPS one.
    if (stripos($datastream_url, 'https://') !== FALSE && stripos($datastream_url, $base_url) !== FALSE) {
      $datastream_url = str_ireplace('https://', 'http://', $datastream_url);
    }
    $datastream_label = truncate_utf8($datastream_label, 255, TRUE, TRUE);

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
      'logMessage' => ($logMessage != NULL) ? $logMessage : 'Ingested object ' . $datastream_id,
    );

    $soap_result = $this->soap_call('addDataStream', $params);

    // Add new DS to the DS list so the item is in sync with the repository.
    $this->datastreams[$datastream_id] = array(
      'label' => $datastream_label,
      'MIMEType' => $datastream_mimetype,
      'URL' => ($this->url() . '/' . $datastream_id . '/' .
        drupal_urlencode($datastream_label)),);

    return $soap_result;
  }

  /**
   * Add datastream from string
   *
   * @param type $str
   * @param type $datastream_id
   * @param type $datastream_label
   * @param type $datastream_mimetype
   * @param type $controlGroup
   * @param type $logMessage
   *
   * @return type
   */
  function add_datastream_from_string($str, $datastream_id, $datastream_label = NULL, $datastream_mimetype = 'text/xml', $controlGroup = 'M', $logMessage = NULL) {
    $dir = file_directory_temp();
    $tmpfilename = tempnam($dir, 'fedoratmp');
    $tmpfile = fopen($tmpfilename, 'w');
    fwrite($tmpfile, $str, strlen($str));
    fclose($tmpfile);
    // Add_datastream_from_file forces a re-sync of the datastream list.
    $returnvalue = $this->add_datastream_from_file($tmpfilename, $datastream_id, $datastream_label, $datastream_mimetype, $controlGroup, $logMessage);
    unlink($tmpfilename);
    return $returnvalue;
  }

  /**
   * Wrapper to add new or modify existing datastream
   *
   * @global url $base_url
   *
   * @param url $external_url
   * @param string $dsid
   * @param string $label
   * @param string $mime_type
   * @param string $controlGroup
   * @param boolean $force
   * @param string $logMessage
   * @param boolean $quiet
   */
  function add_or_modify_by_reference($external_url, $dsid, $label, $mime_type, $controlGroup = 'M', $force = FALSE, $logMessage = 'Modified by Islandora API', $quiet=FALSE) {
    global $base_url;
    if (array_key_exists($dsid, $this->datastreams)) {
      $this->modify_datastream_by_reference($external_url, $dsid, $label, $mime_type, $force, $logMessage, $quiet);
    }
    else {
      $file_location = str_replace("$base_url/", '', $external_url);
      $this->add_datastream_from_file($file_location, $dsid, $label, $mime_type, $controlGroup = 'M', $logMessage = NULL);
    }
  }

  /**
   *
   * @param unknown_type $el
   * @param unknown_type $object
   * @param unknown_type $type
   */
  protected function buildRelsStatement(&$el, $object, $type) {
    if ($type > 0) {
      $el->appendChild($el->ownerDocument->createTextNode($object));
      if ($type == RELS_TYPE_STRING) {
        $el->setAttribute('rdf:datatype', 'http://www.w3.org/2001/XMLSchema#string');
      }
      elseif ($type == RELS_TYPE_INT) {
        $el->setAttribute('rdf:datatype', 'http://www.w3.org/2001/XMLSchema#int');
      }
      elseif ($type == RELS_TYPE_DATETIME) {
        $el->setAttribute('rdf:datatype', 'http://www.w3.org/2001/XMLSchema#dateTime');
      }
      else {
        // Plain literal.
      }
    }
    else {
      $el->setAttribute('rdf:resource', $object);
    }
  }

  /**
   * Add a relationship string to this object's RELS-EXT.
   *
   * does not support rels-int yet.
   *
   * @param string $relationship
   *   The predicate/relationship tag to add
   * @param string|array $object
   *   The object(s) to be related to.
   * @param string $namespaceURI
   *   The predicate namespace.
   * @param int $literal_value
   *   Used to type the value.
   *  - 0: URI
   *  - 1: plain literal
   *  - 2: string (explicitly typed)
   *  - 3: integer
   *  - 4: dateTime
   *
   * @return ???
   *   Value returned from SOAP call for modify_datastream.
   */
  function add_relationship($relationship, $object, $namespaceURI = RELS_EXT_URI, $literal_value = RELS_TYPE_URI) {
    static $relsextxml = NULL;
    if ($relsextxml === NULL) {
      // Avoid new instantiations in long-running processes.
      $relsextxml = new DOMDocument();
    }

    $ds_list = $this->datastreams;

    $f_prefix = 'info:fedora/';
    if (!array_key_exists('RELS-EXT', $ds_list)) {
      $rdf_string = <<<RDF
        <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
          <rdf:Description rdf:about="$f_prefix{$this->pid}">
          </rdf:Description>
        </rdf:RDF>
RDF;
      $this->add_datastream_from_string($rdf_string, 'RELS-EXT', 'Fedora object-to-object relationship metadata', 'application/rdf+xml', 'X');
    }

    $relsext = $this->get_datastream_dissemination('RELS-EXT');

    $relsextxml->loadXML($relsext);
    $description = $relsextxml->getElementsByTagNameNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'Description');
    if ($description->length == 0) {
      // XXX: This really shouldn't be done; lower case d doesn't fit the schema.  Warn users to fix the data and generators, pending deprecation.
      $description = $relsextxml->getElementsByTagNameNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'description');
      if ($description->length > 0) {
        drupal_set_message(t('RDF with lower case "d" in "description" encountered.  Should be uppercase! PID: %pid', array('%pid' => $this->pid)), 'warning');
      }
    }
    $description = $description->item(0);

    // Casting a string to an array gives an array containing the string, and
    //   casting an array to an array does nothing.
    foreach ((array)$object as $obj) {
      if ($literal_value == RELS_TYPE_URI && strpos($obj, $f_prefix) !== 0) {
        $obj = $f_prefix . $obj;
      }

      // Create the new relationship node.
      $newrel = $relsextxml->createElementNS($namespaceURI, $relationship);

      $this->buildRelsStatement($newrel, $obj, $literal_value);

      $description->appendChild($newrel);
    }

    return $this->modify_datastream($relsextxml->saveXML(), 'RELS-EXT', "Fedora Object-to-Object Relationship Metadata", 'application/rdf+xml');
  }

  /**
   * Extension of add_relationship(), which acts on RELS-INT.
   *
   * @param $dsid
   *   A string containing either the base dsid (EXAMPLE)
   *   or URI to the datastream (info:fedora/pid/EXAMPLE)
   */
  function add_dsid_relationship($dsid, $relationship, $object, $namespaceURI = RELS_EXT_URI, $literal_value = RELS_TYPE_URI) {
    static $relsxml = NULL;
    if ($relsxml === NULL) {
      // Avoid new instantiations in long-running processes.
      $relsxml = new DOMDocument();
    }

    $f_prefix = 'info:fedora/';
    if (!array_key_exists('RELS-INT', $this->datastreams)) {
      $rdf_string = <<<RDF
        <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"/>
RDF;
      $this->add_datastream_from_string($rdf_string, 'RELS-INT', 'Fedora datastream relationship metadata', 'application/rdf+xml', 'X');
    }

    $rels_text = $this->get_datastream_dissemination('RELS-INT');

    if (strpos($dsid, $f_prefix) !== 0) {
      $dsid = $f_prefix . $this->pid . '/' . $dsid;
    }

    $relsxml->loadXML($rels_text);
    $descs = $relsxml->getElementsByTagNameNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'Description');
    $description = NULL;
    foreach ($descs as $desc) {
      if ($desc->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'about') == $dsid) {
        $description = $desc;
        break;
      }
    }

    // Create the description element if we didn't find it...
    if ($description === NULL) {
      $description = $relsxml->createElementNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'rdf:Description');
      $description->setAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'rdf:about', $dsid);
      $relsxml->documentElement->appendChild($description);
    }

    foreach ((array)$object as $obj) {
      if ($literal_value == RELS_TYPE_URI && strpos($obj, $f_prefix) !== 0) {
        $obj = $f_prefix . $object;
      }

      // Create the new relationship node.
      $newrel = $relsxml->createElementNS($namespaceURI, $relationship);

      $this->buildRelsStatement($newrel, $obj, $literal_value);

      $description->appendChild($newrel);
    }

    return $this->modify_datastream($relsxml->saveXML(), 'RELS-INT', "Fedora Datastream Relationship Metadata", 'application/rdf+xml');
  }

  /**
   * Purge/delete relationships string from this object's RELS-EXT.
   *
   * does not support rels-int yet.
   *
   * @param string $relationship
   *   The predicate/relationship tag to delete
   * @param string $object
   *   The object to be related to. (NULL/value for which empty()
   *   evaluates to true will remove all relations of the given type,
   *   ignoring $literal_value)
   * @param string $namespaceURI
   *   The predicate namespace.
   * @param int $literal_value
   *   Same as add_relationship.  NOTE: dateTime implemented.
   * @return boolean
   *   Whether or not this operation has produced any changes in the RELS-EXT
   */
  function purge_relationships($relationship, $object, $namespaceURI = RELS_EXT_URI, $literal_value = FALSE) {
    $relsext = trim($this->get_datastream_dissemination('RELS-EXT'));

    $relsextxml = new DOMDocument();
    $modified = FALSE;

    if ($relsext && $relsextxml->loadXML($relsext)) {
      $rels = $relsextxml->getElementsByTagNameNS($namespaceURI, $relationship);
      if (!empty($rels)) {
        // Iterate backwards so if we delete something our pointer doesn't get out of sync.
        for ($i = $rels->length; $i > 0; $i--) {
          $rel = $rels->item($i - 1);
          // Foreach ($rels as $rel) { // moving forward like this caused iteration errors when something was deleted.
          if (
            // If either no object is specified, or the object matches (in either the literal or URI case), remove this node from it's parent, and mark as changed.
              empty($object) ||
              (($literal_value == RELS_TYPE_URI) && $rel->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource') == $object) ||
              (($literal_value == RELS_TYPE_PLAIN_LITERAL) && $rel->textContent == $object) ||
              (($literal_value == RELS_TYPE_STRING) && $rel->getAttribute('rdf:datatype') == 'http://www.w3.org/2001/XMLSchema#string' && $rel->textContent == $object) ||
              (($literal_value == RELS_TYPE_INT) && $rel->getAttribute('rdf:datatype') == 'http://www.w3.org/2001/XMLSchema#int' && intval($rel->textContent) == $object)) {
            $rel->parentNode->removeChild($rel);
            $modified = TRUE;
          }
        }
      }

      // Save changes.
      if ($modified) {
        $this->modify_datastream($relsextxml->saveXML(), 'RELS-EXT', "Fedora Object-to-Object Relationship Metadata", 'text/xml');
      }
    }
    // Return whether or not we've introduced any changes.
    return $modified;
  }

  /**
   * Extension of purge_relationships, which acts on RELS-INT.
   *
   * @param string $dsid
   *   A string containing either the base dsid (EXAMPLE)
   *   or URI to the datastream (info:fedora/pid/EXAMPLE)
   */
  function purge_dsid_relationships($dsid, $relationship, $object, $namespaceURI = RELS_EXT_URI, $literal_value = FALSE) {
    $f_prefix = 'info:fedora/';
    if (strpos($dsid, $f_prefix) !== 0) {
      $dsid = $f_prefix . $this->pid . '/' . $dsid;
    }

    $relsxml = new DOMDocument();
    $relsint = trim($this->get_datastream_dissemination('RELS-INT'));
    $modified = FALSE;
    if ($relsint && $relsxml->loadXML($relsint)) {


      $descs = $relsxml->getElementsByTagNameNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'Description');
      $description = NULL;
      foreach ($descs as $desc) {
        if ($dsid === NULL || $desc->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'about') == $dsid) {
          $rels = $desc->getElementsByTagNameNS($namespaceURI, $relationship);
          if (!empty($rels)) {
            // Iterate backwards so if we delete something our pointer doesn't get out of sync.
            for ($i = $rels->length; $i>0; $i--) {
              $rel = $rels->item($i-1);
              //      foreach ($rels as $rel) { // moving forward like this caused iteration errors when something was deleted
              // If either no object is specified, or the object matches (in either the literal or URI case), remove this node from it's parent, and mark as changed.
              if (
                empty($object) ||
                (($literal_value == RELS_TYPE_URI) && $rel->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource') == $object) ||
                (($literal_value == RELS_TYPE_PLAIN_LITERAL) && $rel->textContent == $object) ||
                (($literal_value == RELS_TYPE_STRING) && $rel->getAttribute('rdf:datatype') == 'http://www.w3.org/2001/XMLSchema#string' && $rel->textContent == $object) ||
                (($literal_value == RELS_TYPE_INT) && $rel->getAttribute('rdf:datatype') == 'http://www.w3.org/2001/XMLSchema#int' && intval($rel->textContent) == $object)) {
                $rel->parentNode->removeChild($rel);
                $modified = TRUE;
              }
            }
          }

          if ($desc->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'about') == $dsid) {
            break;
          }
        }
      }

      // Save changes.
      if ($modified) {
        $this->modify_datastream($relsxml->saveXML(), 'RELS-INT', "Fedora Datastream Relationship Metadata", 'text/xml');
      }
    }

    // Return whether or not we've introduced any changes.
    return $modified;
  }

  /**
   * Removes the given relationship from the item's RELS-EXT and re-saves it.
   *
   * @deprecated
   *   Dropped in favour of purge_relationships, which follows the same paradigm as add_relationship.  This function tries to figure out the predicate URI based on the prefix/predicate given, which requires specific naming...
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
      case "isPageNumber":
        $relationship = "isPageNumber";
        $namespaceURI = ISLANDORA_PAGE_URI;
        break;
    }

    if (!empty($object) && substr($object, 0, 12) != 'info:fedora/') {
      $object = "info:fedora/$object";
    }

    $relsextxml = new DomDocument();
    $relsextxml->loadXML($relsext);
    $modified = FALSE;
    $rels = $relsextxml->getElementsByTagNameNS($namespaceURI, $relationship);
    if (!empty($rels)) {
      foreach ($rels as $rel) {
        if (empty($object) || $rel->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource') == $object) {
          $rel->parentNode->removeChild($rel);
          $modified = TRUE;
        }
      }
    }
    if ($modified) {
      $this->modify_datastream($relsextxml->saveXML(), 'RELS-EXT', "Fedora Object-to-Object Relationship Metadata", 'text/xml');
    }
    return $modified;
  }

  /**
   * Export as foxml
   *
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
   *
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
        $url .= "/objects?query=$field~$pattern&pid=true&title=TRUE&resultFormat=xml&maxResults=$max_results";
      }
      else {
        $url .= "/objects?pid=true&title=true&sessionToken=$session_token&resultFormat=xml&maxResults=$max_results";
      }

      if (count($resultFields) > 0) {
        $url .= '&' . join('=true&', $resultFields) . '=true';
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
   *
   * @param string $dsid
   *   The DSID of the datastream to get the dissemination of.
   * @param string $as_of_date_time
   *   We can get old versions of versioned datastreams.
   *   Defaults to now.
   * @param type $quiet
   *   Squash errors? defaults to FALSE.
   *
   * @return mixed
   *   NULL if the DS is not present in Fedora Item's datastream list.
   *   The content of the DS (NULL if empty)
   */
  function get_datastream_dissemination($dsid, $as_of_date_time = "", $quiet = FALSE) {
    if (!array_key_exists($dsid, $this->datastreams)) {
      return NULL;
    }

    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'asOfDateTime' => $as_of_date_time,
    );

    // Make soap call.
    $object = self::soap_call('getDataStreamDissemination', $params, $quiet);
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
   *
   * @param type $dsid
   * @param type $as_of_date_time
   *
   * @return type
   */
  function get_datastream($dsid, $as_of_date_time = '', $quiet = TRUE) {
    if (!array_key_exists($dsid, $this->datastreams)) {
      return NULL;
    }
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'asOfDateTime' => $as_of_date_time,
    );
    $object = self::soap_call('getDatastream', $params, $quiet);

    return $object->datastream;
  }

  /**
   * Get datastream history
   *
   * @param type $dsid
   *
   * @return type
   */
  function get_datastream_history($dsid) {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
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
   *
   * @param type $service_definition_pid
   * @param type $method_name
   * @param type $parameters
   * @param type $as_of_date_time
   *
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
   * Retrieves and returns a SimpleXML list of this item's datastreams,
   * and stores them as an instance variable for caching purposes.
   *
   * @return SimpleXMLElement
   */
  function get_datastreams_list_as_SimpleXML() {
    //if ( empty( $this->datastreams_list ) ) {
    $params = array(
      'pid' => $this->pid,
      'asOfDateTime' => "",
    );

    $this->datastreams_list = $this->soap_call('listDataStreams', $params);
    //}
    return $this->datastreams_list;
  }

  /**
   * DatastreamControlGroup controlGroup - String restricted to the values of "X", "M", "R", or "E" (InlineXML,Managed Content,Redirect, or External Referenced).
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
   *
   * @return datastream object
   *   get the mimetype size etc. in one shot.  instead of iterating throught the datastream list for what we need
   */
  function get_datastream_info($dsid, $as_of_date_time = '') {
    $params = array(
      'pid' => $this->pid,
      'dsID' => $dsid,
      'asOfDateTime' => $as_of_date_time,
    );

    return $this->soap_call('getDatastream', $params);
  }

  /**
   * Returns an associative array of this object's datastreams.
   * Results look like this:
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
      $xml_list = $this->datastreams_list->datastreamDef;
      if (!is_array($this->datastreams_list->datastreamDef)) {
        $xml_list = array($xml_list);
      }

      foreach ($xml_list as $ds) {
        if (!is_object($ds)) {
          print_r($ds);
        }
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
    foreach ($this->datastreams_list as $datastream) {
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
   *
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

  /**
   * Retrieves RELS-EXT values from item
   *
   * @param array $namespaces
   *
   * @return array
   */
  function get_rdf_relationships($namespaces = null) {
    if ($namespaces == NULL) {
      $namespaces = array(
        RELS_EXT_URI,
        FEDORA_MODEL_URI,
        ISLANDORA_PAGE_URI,
        ISLANDORA_RELS_EXT_URI,
        ISLANDORA_RELS_INT_URI,
      );
    }
    if (!is_array($namespaces)) {
      $namespaces = array($namespaces);
    }
    $relationships = array();
    try {
      $relsext = $this->get_datastream_dissemination('RELS-EXT');
    } catch (exception $e) {
      drupal_set_message(t('Error retrieving RELS-EXT of object %pid.', array('%pid' => $pid)), 'error');
      return $relationships;
    }

    // Parse the RELS-EXT into an associative array.
    $relsextxml = new DOMDocument();
    $relsextxml->loadXML($relsext);
    $relsextxml->normalizeDocument();
    $allTags = array();
    foreach ($namespaces as $namespace) {
      $allTags[] = $relsextxml->getElementsByTagNameNS($namespace, '*');
    }

    foreach ($allTags as $tags) {
      foreach ($tags as $child) {
        $value = preg_replace('/info:fedora\//', '', $child->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource'));
        if (!$value) {
          $value = $child->textContent;
        }
        $relationships[$child->tagName][] = $value;
      }
    }

    return $relationships;
  }

  function get_models() {
    //This is/was formerly just a copy/paste jobbie, without the parameter being passable...
    return $this->get_relationships();
  }

  /**
   * Creates a RELS-EXT XML stream from the supplied array and saves it to
   * the item on the server.
   *
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
   * Set the object to a deleted state
   */
  function move_to_trash($log_message = 'Flagged deleted using Islandora API.', $quiet = TRUE) {
    // Loop through the datastreams and mark them deleted.
    foreach ($this->get_datastreams_list_as_array() as $dsid => $info) {
      $this->set_datastream_state($dsid, 'D');
    }

    // Create a message to mark the object deleted.
    $params = array(
      'pid' => $this->pid,
      'state' => 'D',
      'logMessage' => $log_message,
      'label' => $this->objectProfile->objLabel,
      'ownerId' => null,
    );

    // Send message to mark the object deleted.
    return self::soap_call('modifyObject', $params, $quiet);
  }

  /**
   * Removes this object form the repository.
   * @param type $log_message
   * @param type $force
   * @return type
   */
  function purge($log_message = 'Purged using Islandora API.', $force = FALSE) {

    // Flag the object to be deleted first.
    $this->move_to_trash($log_message);

    // Create the delete message.
    $params = array(
      'pid' => $this->pid,
      'logMessage' => $log_message,
      'force' => $force,
    );

    $this->forget();

    // Delete the object.
    return $this->soap_call('purgeObject', $params);
  }

  /**
   * Purge datastream
   *
   * @param type $dsID
   * @param type $start_date
   * @param type $end_date
   * @param type $log_message
   * @param type $force
   *
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
    $soap_result = $this->soap_call('purgeDatastream', $params);
    // Make sure to refresh the datastream list after adding so this item stays in sync with the repository.
    $this->datastreams = $this->get_datastreams_list_as_array();
    return $soap_result;
  }

  /**
   * URL
   * @global type $base_url
   *
   * @return type
   */
  function url() {
    return url('fedora/repository/' . $this->pid . (!empty($this->objectProfile) ? '/-/' . drupal_urlencode($this->objectProfile->objLabel) : ''), array(
      'absolute' => TRUE,
    ));
  }

  /**
   * Get Next PID in Namespace
   *
   * @param $pid_namespace string
   *
   * @return string
   */
  static function get_next_PID_in_namespace($pid_namespace = '', $number_of_pids = 1) {
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
      'numPIDs' => $number_of_pids,
      'pidNamespace' => $pid_namespace,
    );

    $result = self::soap_call('getNextPID', $params);

    return $result->pid;
  }

  /**
   * ingest from FOXML
   *
   * @param type $foxml
   *
   * @return Fedora_Item
   */
  static function ingest_from_FOXML(DOMDocument $foxml) {
    $params = array(
      'objectXML' => $foxml->saveXML(),
      'format' => 'info:fedora/fedora-system:FOXML-1.1',
      'logMessage' => 'Fedora Object Ingested',
    );
    $object = self::soap_call('ingest', $params);
    return new Fedora_Item($object->objectPID);
  }

  /**
   * ingest from FOXML file
   *
   * @param type $foxml_file
   *
   * @return type
   */
  static function ingest_from_FOXML_file($foxml_file) {
    $foxml = new DOMDocument();
    $foxml->load($foxml_file);
    return self::ingest_from_FOXML($foxml);
  }

  /**
   * ingest from FOXML files in directory
   *
   * @param type $path
   */
  static function ingest_from_FOXML_files_in_directory($path) {
    // Open the directory.
    $dir_handle = @opendir($path);
    // Loop through the files.
    while ($file = readdir($dir_handle)) {
      if ($file == "." || $file == ".." || strtolower(substr($file, strlen($file) - 4)) != '.xml') {
        continue;
      }

      try {
        self::ingest_from_FOXML_file($path . '/' . $file);
      } catch (exception $e) {

      }
    }
    // Close.
    closedir($dir_handle);
  }

  /**
   * Modify Object
   *
   * @param $label string
   * @param $state string
   * @param $ownerId string
   * @param $logMessage string
   * @param $quiet boolean
   *
   * @return type
   */
  function modify_object($label = '', $state = NULL, $ownerId = NULL, $logMessage = 'Modified by Islandora API', $quiet=TRUE) {
    $params = array(
      'pid' => $this->pid,
       // Default to the current owner if none is provided..
      'ownerId' => (($ownerId !== NULL) ?
          $ownerId :
          $this->ownerId),
      'state' => $state,
      'label' => $label,
      'logMessage' => $logMessage,
    );

    return self::soap_call('modifyObject', $params, $quiet);
  }

  /**
   * Work around function, due to file_create_url not URL encoding stuff.
   *
   * Parses and reassembles the URL, exploding, rawurlencoding and imploding
   * the path components along the way.
   *
   * @param $url
   *   A string containing an HTTP(S) URL to attempt.
   *
   * @return string
   *   The results of the HTTP request if successful; boolean FALSE otherwise.
   */
  protected static function try_http_get_content($url) {
    // Can throw a warning prior to 5.3.3.
    $parsed_url = @parse_url($url);
    $supported_schemes = array(
      'http',
      'https',
    );

    $content = FALSE;

    if ($parsed_url && array_key_exists('scheme', $parsed_url) && in_array($parsed_url['scheme'], $supported_schemes)) {
      $components = explode('/', $parsed_url['path']);
      $components = array_map('rawurlencode', $components);
      $fixed_url = url(
              t(
                  '!scheme://!user:!pass@!host:!port/!path',
                  array(
                    '!scheme' => $parsed_url['scheme'],
                    '!user' => rawurlencode($parsed_url['user']),
                    '!pass' => rawurlencode($parsed_url['pass']),
                    '!host' => $parsed_url['host'],
                    '!port' => $parsed_url['port'],
                    '!path' => implode('/', $components),
                  )
              ),
              array(
                'query' => $parsed_url['query'],
                'fragment' => $parsed_url['fragment'],
              )
      );
      $result = drupal_http_request($fixed_url);

      if ((int) ($result->code / 100) === 2) {
        $content = $result->data;
      }
      else {
        watchdog('fedora_repository', 'Failed making HTTP request to @URL.  Info: @info', array(
          '@URL' => $fixed_url,
          '@info' => print_r($result, TRUE),
            ), 'warning');
      }
    }

    return $content;
  }

  /**
   *  Wrap modify by value and reference
   *
   *  Wrap modify by value and reference, so that the proper one gets called
   *  in the correct instance. (value if inline XML, reference otherwise)
   *
   *  First tries to treat the passed in value as a filename, tries using as contents second.
   *  Coerces the data into what is required, and passes it on to the relevant function.
   *
   * @param string $filename_or_content
   *   Either a filename to add, or an string of content.
   * @param string $dsid
   *   The DSID to update
   * @param string $label
   *   A label to withwhich to update the datastream
   * @param string $mime_type
   *   Mimetype for the data being pushed into the datastream
   * @param boolean $force
   *   Dunno, refer to underlying functions/SOAP interface.  We just pass it like a chump.
   * @param string $logMessage
   *   A message for the audit log.
   * @param boolean $quiet
   *   Error suppression?  Refer to soap_call for usage
   *   (just passed along here).
   */
  function modify_datastream($filename_or_content, $dsid, $label, $mime_type, $force = FALSE, $logMessage='Modified by Islandora API', $quiet=FALSE) {
    $toReturn = NULL;

    // Determine if it's inline xml; if it is, modify by value.
    if ($this->get_datastream($dsid)->controlGroup === 'X') {
      $content = '<null/>';
      if (is_file($filename_or_content) && is_readable($filename_or_content)) {
        $content = file_get_contents($filename_or_content);
      }
      else {
        // XXX:  Get the contents to deal with fopen not being allowed for remote access
        //in some OSs
        $temp_content = self::try_http_get_content($filename_or_content);
        if ($temp_content !== FALSE) {
          $content = $temp_content;
        }
        else {
          $content = $filename_or_content;
        }
      }

      $toReturn = $this->modify_datastream_by_value($content, $dsid, $label, $mime_type, $force, $logMessage);
    }
    // Otherwise, write to web-accessible temp file and modify by reference.
    else {
      $file = '';
      $created_temp = FALSE;
      if (is_file($filename_or_content) && is_readable($filename_or_content)) {
        $file = $filename_or_content;
        $original_path = $file;
        // Temporarily move file to a web-accessible location.
        file_copy($file, file_directory_path());
        $created_temp = ($original_path != $file);
      }
      else {
        // XXX:  Get the contents to deal with fopen not being allowed for remote access
        //   in some OSs
        $temp_content = self::try_http_get_content($filename_or_content);
        if ($temp_content !== FALSE) {
          $filename_or_content = $temp_content;
        }

        // Push contents to a web-accessible file.
        $file = file_save_data($filename_or_content, file_create_filename($label, file_directory_path()));
        $created_temp = TRUE;
      }

      $parts = explode(DIRECTORY_SEPARATOR, $file);
      $parts = array_map('rawurlencode', $parts);
      $file_url = file_create_url(implode(DIRECTORY_SEPARATOR, $parts));

      $toReturn = $this->modify_datastream_by_reference($file_url, $dsid, $label, $mime_type, $force, $logMessage);

      if ($created_temp && is_file($file) && is_writable($file)) {
        file_delete($file);
      }
    }

    return $toReturn;
  }

  /**
   * Modify datastream by reference
   *
   * @param type $external_url
   * @param type $dsid
   * @param type $label
   * @param type $mime_type
   * @param type $force
   * @param type $logMessage
   * @param type $quiet
   *
   * @return type
   */
  function modify_datastream_by_reference($external_url, $dsid, $label, $mime_type, $force = FALSE, $logMessage = 'Modified by Islandora API', $quiet=FALSE) {
    global $base_url;

    // Fedora has some problems getting files from HTTPS connections sometimes, so if we are getting a file
    // from the local drupal, we try to pass a HTTP url instead of a HTTPS one.
    if (stripos($external_url, 'https://') !== FALSE && stripos($external_url, $base_url) !== FALSE) {
      $external_url = str_ireplace('https://', 'http://', $external_url);
    }

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
      'force' => $force,
    );
    return self::soap_call('modifyDatastreamByReference', $params, $quiet);
  }

  /**
   * Modify datastream by value
   *
   * @param type $content
   * @param type $dsid
   * @param type $label
   * @param type $mime_type
   * @param type $force
   * @param type $logMessage
   * @param type $quiet
   *
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
      'force' => $force,
    );
    return self::soap_call('modifyDatastreamByValue', $params, $quiet);
  }

  /**
   *
   * @param unknown_type $dsid
   * @param unknown_type $state
   * @param unknown_type $log_message
   * @param unknown_type $quiet
   */
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
   * Make a soap call to the fedora API.
   *
   * @param string $function
   *   The name of the soap function to call.
   * @param array $parameters
   *   Paramters to pass onto the soap call
   * @param boolean $quiet
   *   If TRUE suppress drupal messages.
   *
   * @return mixed
   *   The return value from the soap function if successful, NULL otherwise.
   */
  static function soap_call($function, $parameters, $quiet = FALSE) {
    if (!self::$connection_helper) {
      module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
      self::$connection_helper = new ConnectionHelper();
    }
    $url = (
        in_array($function, self::$SoapManagedFunctions) ?
            variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/wsdl?api=API-M') :
            variable_get('fedora_soap_url', 'http://localhost:8080/fedora/services/access?wsdl')
        );
    try {
      $soap_client = self::$connection_helper->getSoapClient($url);
      if (isset($soap_client)) {
        $result = $soap_client->__soapCall($function, array('parameters' => $parameters));
      }
      else {
        if (!$quiet) {
          drupal_set_message(t('Error trying to get SOAP client connection'));
        }
        watchdog('fedora_repository', 'Error trying to get SOAP client connection.');
        return NULL;
      }
    } catch (Exception $e) {
      if (!$quiet) {
        preg_match('/org\.fcrepo\.server\.security\.xacml\.pep\.AuthzDeniedException/', $e->getMessage()) ?
                drupal_set_message(t('Insufficient permissions to call SOAP function "%func".', array('%func' => $function)), 'error') :
                drupal_set_message(t('Error trying to call SOAP function "%func". Check watchdog logs for more information.', array('%func' => $function)), 'error');
      }
      watchdog('fedora_repository', 'Error Trying to call SOAP function "%func". Exception: @e in @f(@l)\n@t', array('%func' => $function, '@e' => $e->getMessage(), '@f' => $e->getFile(), '@l' => $e->getLine(), '@t' => $e->getTraceAsString()), NULL, WATCHDOG_ERROR);
      return NULL;
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
   * @param string $owner
   *   Used to set an object's ownerId attribute.  Defaults to current user's
   *   name. If we are not a Drupal user(ie. Drush) defaults to ''.
   *
   * @return DOMDocument
   */
  static function create_object_FOXML($pid = '', $state = 'A', $label = 'Untitled', $owner = '') {
    $foxml = new DOMDocument("1.0", "UTF-8");
    $foxml->formatOutput = TRUE;
    if (empty($pid)) {
      // Call getNextPid.
      $pid = self::get_next_PID_in_namespace();
    }
    if (empty($owner)) {
      global $user;
      // Default to current Drupal user.
      if (!empty($user->name)) {
        $owner = $user->name;
      }
      // We are annonamous user.
      elseif ($user->uid == 0) {
        $owner = 'anonymous';
      }
    }

    $root_element = $foxml->createElementNS("info:fedora/fedora-system:def/foxml#", "foxml:digitalObject");
    $root_element->setAttribute("VERSION", "1.1");
    $root_element->setAttribute("PID", $pid);
    $root_element->setAttributeNS("http://www.w3.org/2001/XMLSchema-instance", "xsi:schemaLocation", "info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd");
    $foxml->appendChild($root_element);

    // FOXML object properties section.
    $object_properties = $foxml->createElementNS("info:fedora/fedora-system:def/foxml#", "foxml:objectProperties");
    $state_property = $foxml->createElementNS("info:fedora/fedora-system:def/foxml#", "foxml:property");
    $state_property->setAttribute("NAME", "info:fedora/fedora-system:def/model#state");
    $state_property->setAttribute("VALUE", $state);

    $label_property = $foxml->createElementNS("info:fedora/fedora-system:def/foxml#", "foxml:property");
    $label_property->setAttribute("NAME", "info:fedora/fedora-system:def/model#label");
    $label_property->setAttribute("VALUE", truncate_utf8($label, 255, TRUE, TRUE));

    $owner_property = $foxml->createElementNS("info:fedora/fedora-system:def/foxml#", "foxml:property");
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
   *
   * @param type $pid
   * @param type $state
   * @param type $label
   * @param type $owner
   *
   * @return type
   */
  static function ingest_new_item($pid = '', $state = 'A', $label = '', $owner = '') {
    return self::ingest_from_FOXML(self::create_object_FOXML($pid, $state, $label, $owner));
  }

  /**
   * fedora item exists
   *
   * @param type $pid
   *
   * @return type
   */
  static function fedora_item_exists($pid) {
    $item = new Fedora_Item($pid);
    return $item->exists();
  }

  /**
   * This function will retrieve the ownerId
   * object property from Fedora using the SOAP API.
   *
   * @param string $PID
   *   The Fedora PID to retrieve the
   */
  static function getOwnerId($PID) {
    $params = array(
      'query' => array(
        'conditions' => array(
          array(
            'property' => 'pid',
            'operator' => 'eq',
            'value' => $PID,
          ),
        ),
        'terms' => '',
      ),
      'resultFields' => array('pid', 'ownerId'),
      'maxResults' => 1,
    );
    $response = Fedora_Item::soap_call('findObjects', $params);

    if (!$response) {
      return FALSE;
    }
    $ownerId = $response->result->resultList->objectFields->ownerId;

    return $ownerId;
  }
}
