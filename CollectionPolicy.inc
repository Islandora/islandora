<?php

/**
 * @file 
 * Collection Policy Class
 */
module_load_include('inc', 'fedora_repository', 'XMLDatastream');

/**
 * Collection Policy class ??
 */
class CollectionPolicy extends XMLDatastream {

  static $SCHEMA_URI = 'http://syn.lib.umanitoba.ca/collection_policy.xsd';
  static $DEFAULT_DSID = 'COLLECTION_POLICY';
  private $staging_area = NULL;

  /**
   * Gets the default DSID to use for ContentModel datastreams.
   *
   * @return string $default_dsid
   */
  static function getDefaultDSID() {
    return variable_get('Islandora_Collection_Policy_DSID', CollectionPolicy::$DEFAULT_DSID);
  }

  /**
   * Constructs a new CollectionPolicy object from the specified 
   * collection PID.  If preFetch is disabled, then Islandora will not get the datastream until needed. 
   * (useful when cacheing)
   * Returns FALSE on failure.
   *
   * @param string $pid
   * @param boolean $preFetch = TRUE
   * @return CollectionPolicy $ret
   */
  static function loadFromCollection($pid, $preFetch=TRUE) {
    $ret = FALSE;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');

    try {
      if (self::valid_pid($pid)) {
        $dsid = CollectionPolicy::getDefaultDSID();

        if ($preFetch) {
          $fedoraItem = new Fedora_Item($pid);
          $ds = $fedoraItem->get_datastream_dissemination($dsid);
        }
        else {
          $ds = NULL;
        }
      }

      if (!empty($ds) || !$preFetch) {
        $ret = new CollectionPolicy($ds, $pid, $dsid);
      }
    } catch (SOAPException $e) {

      $ret = FALSE;
    }
    return $ret;
  }

  /**
   * Ingests a new Collection Policy datastream to the specified 
   * PID with the DSID specified.  The file should be a valid collection
   * policy XML.  Returns FALSE on failure.
   *
   * @param string $pid
   * @param string $name
   * @param string $cpDsid
   * @param string $file
   * @return CollectionPolicy $ret
   */
  public static function ingestFromFile($pid, $name, $cpDsid, $file) {
    $ret = FALSE;

    if (($cp = self::loadFromCollection($pid, $cpDsid)) === FALSE && file_exists($file)) {
      $cp = new ContentModel(file_get_contents($file), $pid, $cpDsid);
      $rootEl = $cp->xml->getElementsByTagName('collection_policy')->item(0);
      $rootEl->setAttribute('name', $name);

      module_load_include('inc', 'fedora_repository', 'api/fedora_item');
      $fedoraItem = new Fedora_Item($pid);
      $fedoraItem->add_datastream_from_string($cp->dumpXml(), $cpDsid, $name, 'text/xml', 'X');
      $ret = $cp;
    }

    return $ret;
  }

  /**
   * Ingests a new Collection Policy datastream to the specified 
   * PID with the DSID specified.  Clones the collection policy from the 
   * source collection pid. Returns FALSE on failure.
   *
   * @param string $pid
   * @param string $name
   * @param string $cpDsid
   * @param string $copy_collection_pid
   * @return CollectionPolicy $ret
   */
  public static function ingestFromCollection($pid, $name, $cpDsid, $copy_collection_pid) {
    $ret = FALSE;

    if (($cp = self::loadFromCollection($pid, $cpDsid)) === FALSE && ($copy_cp = self::loadFromCollection($copy_collection_pid)) !== FALSE && $copy_cp->validate()) {
      $newDom = $copy_cp->xml;
      $rootEl = $newDom->getElementsByTagName('collection_policy')->item(0);
      $rootEl->setAttribute('name', $name);

      $cp = new CollectionPolicy($newDom, $pid, $cpDsid);

      module_load_include('inc', 'fedora_repository', 'api/fedora_item');
      $fedoraItem = new Fedora_Item($pid);
      $fedoraItem->add_datastream_from_string($cp->dumpXml(), $cpDsid, $name, 'text/xml', 'X');
      $ret = $cp;
    }

    return $ret;
  }

  /**
   * Ingests a new minimum  Collection Policy datastream to the specified 
   * PID with the DSID specified.  Initializes the policy with the specified values.
   * Returns FALSE on failure
   *
   * @param string $pid
   * @param string $name
   * @param string $cpDsid
   * @param string $model_pid
   * @param string $model_namespace
   * @param string $relationshiop
   * @param string $searchField
   * @param string $searchValue
   * @return CollectionPolicy $ret
   */
  public static function ingestBlankPolicy($pid, $name, $policyDsid, $model_pid, $model_namespace, $relationship, $searchField, $searchValue) {
    $ret = FALSE;
    if (($cp = self::loadFromCollection($pid)) === FALSE) {   //removed second, non-existant variable
      module_load_include('inc', 'fedora_repository', 'ContentModel');
      if (($cm = ContentModel::loadFromModel($model_pid)) !== FALSE && $cm->validate()) {
        $newDom = new DOMDocument('1.0', 'utf-8');
        $newDom->formatOutput = TRUE;
        $rootEl = $newDom->createElement('collection_policy');
        $rootEl->setAttribute('name', $name);
        $rootEl->setAttribute('xmlns', self::$XMLNS);
        $rootEl->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
        $rootEl->setAttribute('xsi:schemaLocation', self::$XMLNS . ' ' . self::$SCHEMA_URI);

        $modelsEl = $newDom->createElement('content_models');

        $cmEl = $newDom->createElement('content_model');
        $cmEl->setAttribute('name', $cm->getName());
        $cmEl->setAttribute('dsid', $cm->dsid);
        $cmEl->setAttribute('namespace', $model_namespace);
        $cmEl->setAttribute('pid', $cm->pid);

        $modelsEl->appendChild($cmEl);
        $rootEl->appendChild($modelsEl);

        $relEl = $newDom->createElement('relationship', $relationship);
        $rootEl->appendChild($relEl);

        $searchTermsEl = $newDom->createElement('search_terms');
        $newTermEl = $newDom->createElement('term', $searchValue);
        $newTermEl->setAttribute('field', $searchField);
        $searchTermsEl->appendChild($newTermEl);
        $rootEl->appendChild($searchTermsEl);

        $newDom->appendChild($rootEl);

        $cp = new CollectionPolicy($newDom, $pid, $policyDsid);

        module_load_include('inc', 'fedora_repository', 'api/fedora_item');


        $fedoraItem = new Fedora_Item($pid);
        $fedoraItem->add_datastream_from_string($cp->dumpXml(), $policyDsid, $name, 'text/xml', 'X');
        $ret = $cp;
      }
    }

    return $ret;
  }

  /**
   * Constructor
   * NOTE: Use the static constructor methods whenever possible.
   *
   * @param string $xmlStr
   * @param string $pid
   * @param string $dsid
   * @return XMLDatastream $cm
   */
  public function __construct($xmlStr, $pid = NULL, $dsid = NULL) {
    parent::__construct($xmlStr, $pid, $dsid);
    $this->name = 'Collection Policy';
  }

  /**
   * Attempts to convert from the old XML schema to the new by 
   * traversing the XML DOM and building a new DOM.  When done
   * $this->xml is replaced by the newly created DOM..
   *
   * @return void
   */
  protected function convertFromOldSchema() {
    if ($this->xml == NULL) {
      $this->fetchXml();
    }
    $sXml = simplexml_load_string($this->xml->saveXML());
    $newDom = new DOMDocument('1.0', 'utf-8');
    $newDom->formatOutput = TRUE;

    $rootEl = $newDom->createElement('collection_policy');
    $rootEl->setAttribute('name', $sXml['name']);
    $rootEl->setAttribute('xmlns', self::$XMLNS);
    $rootEl->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
    $rootEl->setAttribute('xsi:schemaLocation', self::$XMLNS . ' ' . self::$SCHEMA_URI);

    $content_modelsEl = $newDom->createElement('content_models');
    foreach ($sXml->contentmodels->contentmodel as $contentmodel) {
      $content_modelEl = $newDom->createElement('content_model');
      $content_modelEl->setAttribute('name', $contentmodel['name']);
      $content_modelEl->setAttribute('dsid', $contentmodel->dsid);
      $content_modelEl->setAttribute('namespace', $contentmodel->pid_namespace);
      $content_modelEl->setAttribute('pid', $contentmodel->pid);
      $content_modelsEl->appendChild($content_modelEl);
    }
    $rootEl->appendChild($content_modelsEl);

    $search_termsEl = $newDom->createElement('search_terms');
    foreach ($sXml->search_terms->term as $term) {
      $termEl = $newDom->createElement('term', $term->value);
      $termEl->setAttribute('field', $term->field);
      if (strval($sXml->search_terms->default) == $term->field) {
        $termEl->setAttribute('default', 'TRUE');
      }
      $search_termsEl->appendChild($termEl);
    }
    $rootEl->appendChild($search_termsEl);

    $relationshipEl = $newDom->createElement('relationship', $sXml->relationship);
    $rootEl->appendChild($relationshipEl);

    $newDom->appendChild($rootEl);
    $this->xml = new DOMDocument();
    $this->xml->loadXML($newDom->saveXml());
  }

  /**
   * Gets the name of the relationship to use 
   * for members of this collection. 
   * Returns FALSE on failure.
   *
   * @return string $relationship
   */
  public function getRelationship() {
    $ret = FALSE;
    if ($this->validate()) {
      $ret = trim($this->xml->getElementsByTagName('relationship')->item(0)->nodeValue);
    }
    return $ret;
  }

  /**
   * Sets the name of the relationship to use 
   * for members of this collection. 
   * Returns FALSE on failure.
   *
   * @param string $relationship
   * @return boolean $ret
   */
  public function setRelationship($relationship) {
    $ret = FALSE;
    if ($this->validate()) {
      $relationshipEl = $this->xml->getElementsByTagName('relationship')->item(0);
      $relationshipEl->nodeValue = trim($relationship);
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * Gets the path to the staging area to use for this 
   * collection. By default recurses to the parent collection
   * if the staging area is undefined
   *
   * @param BOOLEAN $recurse 
   * @return string $path
   */
  public function getStagingArea($recurse=TRUE) {
    $ret = FALSE;
    if ($this->validate()) {
      if ($this->staging_area === NULL) {
        $stagingEl = $this->xml->getElementsByTagName('staging_area');

        if ($stagingEl->length > 0) {
          $this->staging_area = trim($stagingEl->item(0)->nodeValue);
        }
        elseif ($recurse) {
          module_load_include('inc', 'fedora_repository', 'api/fedora_item');
          $item = new Fedora_Item($this->pid);
          $rels = $item->get_relationships();
          if (count($rels) > 0) {
            foreach ($rels as $rel) {
              $cp = CollectionPolicy::loadFromCollection($rel['object']);
              if ($cp !== FALSE) {
                $this->staging_area = $cp->getStagingArea();
                break;
              }
            }
          }
        }
      }

      $ret = $this->staging_area;
    }
    return $ret;
  }

  /**
   * Sets the path to the staging area to use for this 
   * collection. If specified path is blank (or FALSE) it will
   * remove the staging are path element from the collection policy.
   *
   * @param string $path 
   *
   * @return string $relationship
   */
  public function setStagingArea($path) {
    $ret = FALSE;
    if ($this->validate()) {
      $rootEl = $this->xml->getElementsByTagName('collection_policy')->item(0);
      $stagingEl = $this->xml->getElementsByTagName('staging_area');
      if ($stagingEl->length > 0) {
        $stagingEl = $stagingEl->item(0);
        if (trim($path) == '') {
          $rootEl->removeChild($stagingEl);
        }
        else {
          $stagingEl->nodeValue = trim($path);
        }
      }
      elseif (trim($path) != '') {
        $stagingEl = $this->xml->createElement('staging_area', trim($path));
        $rootEl->appendChild($stagingEl);
      }

      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * Gets the next available PID for the
   * content model specified by the DSID
   * parameter.
   *
   * @param string $dsid
   * @return string $nextPid
   */
  public function getNextPid($dsid, $content_model = null) {
    $ret = FALSE;

    if (self::valid_dsid($dsid) && $this->validate()) {
      $content_models = $this->xml->getElementsByTagName('content_models')->item(0)->getElementsByTagName('content_model');
      $namespace = FALSE;
      for ($i = 0; $namespace === FALSE && $i < $content_models->length; $i++) {
        if (strtolower($content_models->item($i)->getAttribute('dsid')) == strtolower($dsid) && (strtolower($content_models->item($i)->getAttribute('pid') == $content_model) || $content_model == null)) {
          $namespace = $content_models->item($i)->getAttribute('namespace');
        }
      }

      $pname = substr($namespace, 0, strpos($namespace, ":"));

      module_load_include('inc', 'fedora_repository', 'api/fedora_item');
      $ret = Fedora_Item::get_next_pid_in_namespace($pname);
    }
    return $ret;
  }

  /**
   * Gets a list of ContentModel objects supported by this collection.
   *
   * @return ContentModel[] $models
   */
  function getContentModels() {
    $ret = FALSE;
    if ($this->validate()) {
      module_load_include('inc', 'Fedora_Repository', 'ContentModel');
      $ret = array();
      $content_models = $this->xml->getElementsByTagName('content_models')->item(0)->getElementsByTagName('content_model');
      for ($i = 0; $i < $content_models->length; $i++) {
        $cm = ContentModel::loadFromModel($content_models->item($i)->getAttribute('pid'),
            $content_models->item($i)->getAttribute('dsid'),
            $content_models->item($i)->getAttribute('namespace'),
            $content_models->item($i)->getAttribute('name'));
        if ($cm !== FALSE) {
          $ret[] = $cm;
        }
      }
    }
    return $ret;
  }

  /**
   * Gets a list of search terms from the Collection Policy.  If asArray is set
   * it will return an associative array with the value, field name, and the default value. 
   * If not set, an array of just the values will be returned.  If $recurse is TRUE, it will 
   * recurseively return the parents search terms if it has none until it returns a set of terms or reaches
   * the top level collection.  If $cache is TRUE, it will return a cached version (if available).
   * 
   * @param boolean $asArray
   * @param boolean $recurse
   * @param boolean $cache
   * @return string[] $ret
   */
  function getSearchTerms($asArray = FALSE, $recurse = FALSE, $cache = FALSE) {
    $ret = FALSE;

    if ($cache == TRUE && ($cache = cache_get('collection_policy_search_terms-' . $this->pid)) !== 0) {
      $ret = $cache->data;
    }
    else {

      if ($this->xml == NULL) {
        $fedoraItem = new Fedora_Item($this->pid);
        $ds = $fedoraItem->get_datastream_dissemination($this->dsid);
        $this->xml = new DOMDocument();
        $this->xml->loadXML($ds);
      }


      if ($this->validate()) {
        $ret = array();
        $terms = $this->xml->getElementsByTagName('search_terms')->item(0)->getElementsByTagName('term');
        for ($i = 0; $i < $terms->length; $i++) {
          $default = $terms->item($i)->attributes->getNamedItem('default');
          $default = ($default !== NULL) ? (strtolower($default->nodeValue) == 'TRUE') : FALSE;
          $ret[] = ($asArray) ? array('value' => $terms->item($i)->nodeValue,
            'field' => $terms->item($i)->getAttribute('field'),
            'default' => $default) : $terms->item($i)->nodeValue;
        }


        if ($recurse && count($ret) == 0) {
          module_load_include('inc', 'fedora_repository', 'api/fedora_item');
          $item = new Fedora_Item($this->pid);
          $rels = $item->get_relationships();
          if (count($rels) > 0) {
            foreach ($rels as $rel) {
              $cp = CollectionPolicy::loadFromCollection($rel['object']);
              if ($cp !== FALSE) {
                $ret = $cp->getSearchTerms($asArray, $recurse);
                break;
              }
            }
          }
        }
        cache_set('collection_policy_search_terms-' . $this->pid, $ret, 'cache', time() + 3600);
      }
    }
    return $ret;
  }

  /**
   * Adds a search term to the collection policy.  
   * Returns fase on failure.
   *
   * @param string $field
   * @param string $value 
   * @return boolean $success
   */
  function addTerm($field, $value) {
    $ret = FALSE;
    if ($this->validate()) {
      $search_termsEl = $this->xml->getElementsByTagName('search_terms')->item(0);
      $terms = $search_termsEl->getElementsByTagName('term');
      $found = FALSE;
      for ($i = 0; !$found && $i < $terms->length; $i++) {
        if ($terms->item($i)->getAttribute('field') == $field) {
          $found = TRUE;
        }
      }

      if (!$found) {
        $newTermEl = $this->xml->createElement('term', $value);
        $newTermEl->setAttribute('field', $field);
        $search_termsEl->appendChild($newTermEl);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * Removes the search term specified by the field parameter from the collection policy.
   *
   * @param string $field
   * @return boolean $success
   */
  function removeTerm($field) {
    $ret = FALSE;
    if ($this->validate()) {
      $search_termsEl = $this->xml->getElementsByTagName('search_terms')->item(0);
      $terms = $search_termsEl->getElementsByTagName('term');
      $found = FALSE;
      for ($i = 0; !$found && $i < $terms->length; $i++) {
        if ($terms->item($i)->getAttribute('field') == $field) {
          $found = $terms->item($i);
        }
      }

      if ($found !== FALSE) {
        $search_termsEl->removeChild($found);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * setDefaultTerm ??
   * @param type $field
   * @return boolean 
   */
  function setDefaultTerm($field) {
    $ret = FALSE;
    if ($this->validate()) {
      $search_termsEl = $this->xml->getElementsByTagName('search_terms')->item(0);
      $terms = $search_termsEl->getElementsByTagName('term');
      $found = FALSE;
      for ($i = 0; !$found && $i < $terms->length; $i++) {
        if ($terms->item($i)->getAttribute('field') == $field) {
          $found = $terms->item($i);
        }
      }

      if ($found !== FALSE) {
        for ($i = 0; $i < $terms->length; $i++)
          if ($terms->item($i)->attributes->getNamedItem('default') !== NULL) {
            $terms->item($i)->removeAttribute('default');
          }
        $found->setAttribute('default', 'TRUE');
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * Removes the specified content model from the collection policy.  This will only 
   * prevent future ingests of the removed model to the collection.  $cm should be 
   * a valid ContentModel object.  Returns FALSE on failure or when the CM was not found in
   * the collection policy.
   *
   * @param ContentModel $cm
   * @return boolean $valid
   */
  function removeModel($cm) {
    $ret = FALSE;
    if ($this->validate() && $cm->validate()) {
      $contentmodelsEl = $this->xml->getElementsByTagName('content_models');
      $models = $contentmodelsEl->item(0)->getElementsByTagName('content_model');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $models->length; $i++) {
        if ($models->item($i)->getAttribute('pid') == $cm->pid) {
          $found = $models->item($i);
        }
      }

      if ($found !== FALSE && $models->length > 1) {
        $contentmodelsEl->item(0)->removeChild($found);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * addModel ??
   * @param ContentModel $cm
   * @param type $namespace
   * @return type 
   */
  function addModel($cm, $namespace) {
    $ret = FALSE;
    if (self::valid_pid($namespace) && $this->validate() && $cm->validate()) {
      $contentmodelsEl = $this->xml->getElementsByTagName('content_models');
      $models = $contentmodelsEl->item(0)->getElementsByTagName('content_model');
      $found = FALSE;
      for ($i = 0; !$found && $i < $models->length; $i++) {
        if ($models->item($i)->getAttribute('pid') == $cm->pid)
          $found = TRUE;
      }

      if (!$found) {
        $cmEl = $this->xml->createElement('content_model');
        $cmEl->setAttribute('name', $cm->getName());
        $cmEl->setAttribute('dsid', $cm->dsid);
        $cmEl->setAttribute('namespace', $namespace);
        $cmEl->setAttribute('pid', $cm->pid);
        $contentmodelsEl->item(0)->appendChild($cmEl);
      }

      $ret = !$found;
    }
    return $ret;
  }

  /**
   * getName ??
   * @return type 
   */
  function getName() {
    $ret = FALSE;
    if ($this->validate()) {
      $ret = $this->xml->getElementsByTagName('collection_policy')->item(0)->getAttribute('name');
    }
    return $ret;
  }

}
