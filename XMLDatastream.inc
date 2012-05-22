<?php

/**
 * @file 
 * 
 */

/**
 * @file 
 * XMLDatastream Abstract Class
 */

/**
 * XMLDatastream class ??
 */
abstract class XMLDatastream {

  static $XMLNS = 'http://www.islandora.ca';
  static $errors = NULL;
  protected $xml = NULL;
  private $valid = NULL;
  protected $forceSchema = FALSE; // if set, the datastream will be validated against the specified schema in self::$SCHEMA_URI instead of
  // reading the schema URI from the datastream.
  public $pid;
  public $dsid;

  /**
   * Parses an PID from an identifier.
   * @param string $identifier 
   * @return string $pid
   */
  public static function getPidFromIdentifier($identifier) {
    return substr($identifier, 0, strpos($identifier, "/"));
  }

  /**
   * valid_pid
   * Validates a fedora PID based on the regexp provided in the fedora 
   * 3.3 documentation.  
   * http://www.fedora-commons.org/confluence/display/FCR30/Fedora+Identifiers
   *
   * @param String $pid
   * @return boolean $valid
   */
  public static function valid_pid($pid) {
    $valid = FALSE;
    if (strlen(trim($pid)) <= 64 && preg_match('/^([A-Za-z0-9]|-|\.)+:(([A-Za-z0-9])|-|\.|~|_|(%[0-9A-F]{2}))+$/', trim($pid))) {
      $valid = TRUE;
    }

    return $valid;
  }

  /**
   * valid_dsid
   * Validates a fedora Dsid based on the the allowed XML standard NCName. 
   * The regexp is a "regular" subset of names allowed, it excludes some extended hex characters that are
   * technically permitted.
   * http://www.fedora-commons.org/confluence/display/FCR30/Fedora+Identifiers
   *
   * @param String $pid
   * @return boolean $valid
   */
  public static function valid_dsid($dsid) {
    $valid = FALSE;
    if (strlen(trim($dsid)) <= 64 && preg_match('/^[a-zA-Z0-9\_\-\.]+$/', trim($dsid))) {
      $valid = TRUE;
    }

    return $valid;
  }

  /**
   * Parses the DSID from an identifier.
   * TODO: combine this method with getPidFromIdentifier?
   * @param string $identifier
   * @return string $dsid
   */
  public static function getDSIDFromIdentifier($identifier) {
    $temp = strstr($identifier, "/");
    return substr($temp, 1);
  }

  /**
   * Constructs an XMLDatastream object from the XML file specified.
   * Returns FALSE on failure.
   *
   * @param string $filename
   * @return XMLDatastream $cm
   */
  public static function loadFromFile($filename) {
    return new self(file_get_contents($filename));
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
    libxml_use_internal_errors(TRUE);

    $this->pid = $pid;
    $this->dsid = $dsid;
    if ($xmlStr !== NULL) {
      if (is_object($xmlStr) && get_class($xmlStr) == DOMDocument) {
        $this->xml = $xmlStr;
      }
      else {
        $this->xml = new DOMDocument();
        $this->xml->loadXML($xmlStr);
      }
    }
  }

  /**
   * Gets the identifier for this XMLDatastream
   * Returns FALSE on failure.
   * 
   * NOTE: not available if constructed directly from file.
   *
   * @return string identifier
   */
  public function getIdentifier() {
    return ($this->pid != NULL && $this->dsid != NULL) ? $this->pid . '/' . $this->dsid : FALSE;
  }

  /**
   * Dumps the XMLDatastream as an XML String 
   * 
   *
   * @return string xml
   */
  public function dumpXml() {
    if ($this->xml == NULL) {
      $this->fetchXml();
    }
    return $this->xml->saveXml();
  }

  /**
   * Validates the XMLDatastream against the schema location 
   * defined by the xmlns:schemaLocation attribute of the root 
   * element. If the xmlns:schemaLocation attribute does not exist,
   * then it is assumed to be the old schema and it attempts to convert 
   * using the convertFromOldSchema method.
   *
   * TODO:  Maybe change it so that it always validates against a known 
   * schema.  This makes more sense because this class assumes the structure
   * to be known after it has been validated.
   *
   * @return boolean $valid
   */
  public function validate() {
    global $user;
    if ($this->valid === NULL) {
      $ret = TRUE;
      if ($this->xml == NULL) {
        $this->fetchXml();
      }
      // figure out if we're dealing with a new or old schema
      $rootEl = $this->xml->firstChild;
      if (!$rootEl->hasAttributes() || $rootEl->attributes->getNamedItem('schemaLocation') === NULL) {
        //$tmpname = substr($this->pid, strpos($this->pid, ':') + 1);
        $tmpname = user_password(10);
        $this->convertFromOldSchema();
        drupal_add_js("fedora_repository_print_new_schema_$tmpname = function(tagID) {
        var target = document.getElementById(tagID);
        var content = target.innerHTML;
        var text = '<html><head><title>Title'   +
        '</title><body>' + content +'</body></html>';
        printerWindow = window.open('', '', 'toolbar=no,location=no,' + 'status=no,menu=no,scrollbars=yes,width=650,height=400');
        printerWindow.document.open();
        printerWindow.document.write(text);
        }", 'inline');

        if (user_access('administer site configuration')) {
          drupal_set_message('<span id="new_schema_' . $tmpname . '" style="display: none;">' . htmlentities($this->xml->saveXML()) . '</span>Warning: XMLDatastream performed conversion of \'' . $this->getIdentifier() . '\' from old schema.  Please update the datastream. The new datastream contents are <a href="javascript:fedora_repository_print_new_schema_' . $tmpname . '(\'new_schema_' . $tmpname . '\')">here.</a> ');
        }

        $rootEl = $this->xml->firstChild;
      }

      $schemaLocation = NULL;
      if ($this->forceSchema) {
        // hack because you cant easily get the static property value from
        // a subclass.
        $vars = get_class_vars(get_class($this));
        $schemaLocation = $vars['SCHEMA_URI'];
      }
      elseif ($rootEl->attributes->getNamedItem('schemaLocation') !== NULL) {
        //figure out where the schema is located and validate.
        list(, $schemaLocation) = preg_split('/\s+/', $rootEl->attributes->getNamedItem('schemaLocation')->nodeValue);
      }
      $schemaLocation = NULL;
      return TRUE;
      if ($schemaLocation !== NULL) {
        if (!$this->xml->schemaValidate($schemaLocation)) {
          $ret = FALSE;
          $errors = libxml_get_errors();
          foreach ($errors as $err) {
            self::$errors[] = 'XML Error: Line ' . $err->line . ': ' . $err->message;
          }
        }
        else {
          $this->name = $rootEl->attributes->getNamedItem('name')->nodeValue;
        }
      }
      else {
        $ret = FALSE;
        self::$errors[] = 'Unable to load schema.';
      }

      $this->valid = $ret;
    }

    return $this->valid;
  }

  /**
   * Saves the current XML datastream back to fedora.   The XML must validate.
   *
   * @return boolean $success
   */
  public function saveToFedora() {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if ($this->validate()) {
      $item = new Fedora_Item($this->pid);
      $item->modify_datastream_by_value($this->dumpXml(), $this->dsid, $this->name, 'application/xml');
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Purges veersions of the datastream newer than and including the start_date.  If 
   * End date is specified, it can be used to purge a range of versions instead. Date should be in 
   * DATE_RFC822 format
   *
   * @param string $start_date
   * @param string $end_date
   * @return boolean $valid
   */
  public function purgeVersions($start_date, $end_date = NULL) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $fedora_item = new Fedora_Item($this->pid);
    return $fedora_item->purge_datastream($this->dsid, $start_date, $end_date);
  }

  /**
   * Gets the history of the datastream from fedora.
   * Returns FALSE on failure.
   *
   * @return string[] $ret
   */
  public function getHistory() {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $fedora_item = new Fedora_Item($this->pid);
    $history = $fedora_item->get_datastream_history($this->dsid);

    $ret = FALSE;
    if ($history !== FALSE) {
      $ret = array();
      foreach ($history as $version) {
        if ($version->versionID !== NULL)
          $ret[] = array('versionID' => $version->versionID, 'createDate' => $version->createDate);
      }
    }
    return $ret;
  }

  /**
   * Attempts to convert from the old XML schema to the new by 
   * traversing the XML DOM and building a new DOM.  When done
   * $this->xml is replaced by the newly created DOM..
   *
   * @return void
   */
  abstract protected function convertFromOldSchema();
}
