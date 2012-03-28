<?php


/**
 * @file 
 * Object Helper Class
 */

/**
 * ObjectHelper Class ??
 */
class ObjectHelper {

//allowed operations
  public static $OBJECT_HELPER_VIEW_FEDORA = 'view fedora collection';
  public static $EDIT_FEDORA_METADATA = 'edit fedora meta data';
  public static $PURGE_FEDORA_OBJECTSANDSTREAMS = 'purge objects and datastreams';
  public static $ADD_FEDORA_STREAMS = 'add fedora datastreams';
  public static $INGEST_FEDORA_OBJECTS = 'ingest new fedora objects';
  public static $EDIT_TAGS_DATASTREAM = 'edit tags datastream';
  public static $VIEW_DETAILED_CONTENT_LIST = 'view detailed list of content';
  public static $DISPLAY_ALWAYS = 0;
  public static $DISPLAY_NEVER = 1;
  public static $DISPLAY_NO_MODEL_OUTPUT = 2;
  // TODO: Make this into a static member constant
  public $availableDataStreamsText = 'Detailed list of content';

  /**
   * Constructor
   */
  function ObjectHelper() {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
    $connectionHelper = new ConnectionHelper();
    //$this->fedoraUser = $connectionHelper->getUser();
    //$this->fedoraPass = $connectionHelper->getPassword();
  }

  /**
   * Grabs a stream from fedora sets the mimetype and returns it. $dsID is the
   * datastream id.   If $forceSoap is set, the function will always buffer the datastream from fedora.  Otherwise, it will 
   * try and use a redirect if possible.
   *
   * @global type $user
   * @param type $pid
   * @param type $dsID
   * @param type $asAttachment
   * @param type $label
   * @param type $filePath
   * @param type $version
   * @param type $forceSoap
   * @return type 
   */
  function makeObject($pid, $dsID, $asAttachment = FALSE, $label = NULL, $filePath=FALSE, $version=NULL, $forceSoap = TRUE) {
    global $user;
    module_load_include('inc', 'fedora_repository', 'ContentModel');
    if ($pid == NULL || $dsID == NULL) {
      drupal_set_message(t("no pid or dsid given to create an object with"), 'error');
      return ' ';
    }
    $headers = module_invoke_all('file_download', "/fedora/repository/$pid/$dsID");
    if (in_array(-1, $headers)) {
      drupal_set_message(t('hello'));
      drupal_access_denied();

      return ' ';
    }

    if (!fedora_repository_access(OBJECTHELPER :: $OBJECT_HELPER_VIEW_FEDORA, $pid, $user)) {
      drupal_set_message(t("You do not have access Fedora objects within the attempted namespace."), 'error');
      drupal_access_denied();
      return ' ';
    }


    if (variable_get('fedora_object_restrict_datastreams', FALSE) == TRUE) {
      if (($cm = ContentModel::loadFromObject($pid)) == FALSE) {
        drupal_set_message(t("You do not have access to objects without an Islandora Content Model."), 'error');
        drupal_access_denied();
        return ' ';
      }

      $cmDatastreams = $cm->listDatastreams();
      if (!((isset($user) && in_array('administrator', $user->roles)) || in_array($dsID, $cmDatastreams))) {
        drupal_set_message(t("You do not have access to the specified datastream."), 'error');
        drupal_access_denied();
        return ' ';
      }
    }

    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);


    if (isset($item->datastreams[$dsID])) {
      $mimeType = $item->datastreams[$dsID]['MIMEType'];
      if ($label == NULL) {
        $label = $item->datastreams[$dsID]['label'];
      }
    }
    else {
      drupal_not_found();
      exit();
    }


    if ((!isset($user)) || $user->uid == 0) {
      $fedoraUser = 'anonymous';
      $fedoraPass = 'anonymous';
      $contentSize = 0;
    }
    else {
      $fedoraUser = $user->name;
      $fedoraPass = $user->pass;
      $dataStreamInfo = $item->get_datastream_info($dsID);
      $contentSize = $dataStreamInfo->datastream->size;
    }

    if (function_exists("curl_init")) {
      if (!isset($mimeType)) {
        $pid = variable_get('fedora_default_display_pid', 'demo:10');
        $dsID = variable_get('fedora_default_display_dsid', 'TN');
        $mimeType = 'image/jpeg';
      }
      $url = variable_get('fedora_base_url', 'http://localhost:8080/fedora') . '/objects/' . $pid . '/datastreams/' . $dsID . '/content';
      if ($version) {
        $url .= '/' . $version; //drupal_urlencode($version);
      }
      $ch = curl_init();
      $user_agent = "Mozilla/4.0 pp(compatible; MSIE 5.01; Windows NT 5.0)";
      curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
      curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
      curl_setopt($ch, CURLOPT_FAILONERROR, 1); // Fail on errors
      curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1); // allow redirects
      //curl_setopt($ch, CURLOPT_TIMEOUT, 15); // times out after 15s
      curl_setopt($ch, CURLOPT_USERAGENT, $user_agent);
      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_USERPWD, "$fedoraUser:$fedoraPass");
      // There seems to be a bug in Fedora 3.1's REST authentication, removing this line fixes the authorization denied error.
      // curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, 0); // return into a variable

      curl_setopt($ch, CURLOPT_URL, $url);

      if ($filePath !== FALSE) {
        $fp = fopen($filePath, 'w');
        curl_setopt($ch, CURLOPT_FILE, $fp);
        curl_setopt($ch, CURLOPT_HEADER, 0);
        curl_exec($ch);
        fclose($fp);
      }
      else {

        header("Content-type: $mimeType");
        if ($contentSize > 0) {
          header("Content-length: $contentSize");
        }

        if ($asAttachment) {
          $suggestedFileName = "$label";
          $pos = strpos($suggestedFileName, '.');

          /*
           * Here we used to take an object of, say, type application/pdf with label, say, "My Document"
           * and we assemble the output filename extension based on the post-slash portion of the mimetype.
           * (If the label has a period anywhere in it, we leave it alone.)
           *
           * This is great for simple mimetypes like application/pdf, text/html, image/jpeg, etc.
           * but it's terrible for, say, application/vnd.oasis.opendocument.text (.odt).
           *
           * Instead we'll use the get_extension function in MimeClass.inc to discover a valid extension
           * for the mimetype in question.
           */
          if ($pos === FALSE) {
            module_load_include('inc', 'fedora_repository', 'MimeClass');
            $mimeclass = new MimeClass();
            $ext = $mimeclass->get_extension($mimeType);
            $suggestedFileName = "$label.$ext";
          }

          header('Content-Disposition: attachment; filename="' . $suggestedFileName . '"');
        }

        if ((isset($user) && $user->uid != 0) || $forceSoap || isset($_SERVER['HTTPS'])) {
          curl_exec($ch);
        }
        else {
          header('Location: ' . $url);
        }
      }
      curl_close($ch);
    }
    else {
      drupal_set_message(t('No curl support.'), 'error');
    }
  }

  /**
   * Gets collection objects t
   *
   * @param type $pid
   * @param type $query
   * @return type 
   */
  function getCollectionInfo($pid, $query = NULL) {
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    $collectionClass = new CollectionClass();
    $results = $collectionClass->getRelatedItems($pid, $query);
    return $results;
  }

  /**
   * returns the mime type
   *
   * @global type $user
   * @param type $pid
   * @param type $dsID
   * @return type 
   */
  function getMimeType($pid, $dsID) {
    global $user;
    if (empty($pid) || empty($dsID)) {
      drupal_set_message(t('You must specify an object pid and datastream ID.'), 'error');
      return '';
    }
    if (!fedora_repository_access(ObjectHelper :: $OBJECT_HELPER_VIEW_FEDORA, $pid, $user)) {
      drupal_set_message(t('You do not have the appropriate permissions'), 'error');
      return;
    }

    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new fedora_item($pid);
    $datastream_list = $item->get_datastreams_list_as_SimpleXML();
    if (!isset($datastream_list)) {
      //drupal_set_message( t("No datastreams available."), 'status' );
      return ' ';
    }
    foreach ($datastream_list as $datastream) {
      foreach ($datastream as $datastreamValue) {
        if ($datastreamValue->ID == $dsID) {
          return $datastreamValue->MIMEType;
        }
      }
    }
    return '';
  }

  /**
   * getDatastreamInfo ??
   * @global type $user
   * @param type $pid
   * @param type $dsID
   * @return type 
   */
  function getDatastreamInfo($pid, $dsID) {
    global $user;
    if (empty($pid) || empty($dsID)) {
      drupal_set_message(t('You must specify an object pid and datastream ID.'), 'error');
      return '';
    }
    if (!fedora_repository_access(ObjectHelper :: $OBJECT_HELPER_VIEW_FEDORA, $pid, $user)) {
      drupal_set_message(t('You do not have the appropriate permissions'), 'error');
      return;
    }

    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new fedora_item($pid);
    $datastream_list = $item->get_datastreams_list_as_SimpleXML();
    if (!isset($datastream_list)) {
      //drupal_set_message( t("No datastreams available."), 'status' );
      return ' ';
    }
    foreach ($datastream_list as $datastream) {
      foreach ($datastream as $datastreamValue) {
        if ($datastreamValue->ID == $dsID) {
          return $datastreamValue;
        }
      }
    }
    return '';
  }

  /**
   * internal function
   * @param $pid String
   * @param $dataStreamValue Object
   */
  function create_link_for_ds($pid, $dataStreamValue) {
    global $base_url;
    $path = drupal_get_path('module', 'fedora_repository');

    require_once($path . '/api/fedora_item.inc');
    $item = new Fedora_Item($pid);

    if (user_access(ObjectHelper :: $PURGE_FEDORA_OBJECTSANDSTREAMS)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($pid, 'write');
      }
      if ($allow) {
        $purgeImage = '<a title="purge datastream ' . $dataStreamValue->label . '" href="' . $base_url . '/fedora/repository/purgeStream/' .
            $pid . '/' . $dataStreamValue->ID . '/' . $dataStreamValue->label . '"><img src="' . $base_url . '/' . $path .
            '/images/purge.gif" alt="purge datastream" /></a>';
      }
    }
    else {
      $purgeImage = '&nbsp;';
    }
    $fullPath = base_path() . $path;

    // Add an icon to replace a datastream
    // @TODO Note: using l(theme_image(..), ...); for these image links (and other links) may remove the need to have clean urls enabled.
    $replaceImage = '&nbsp;';
    if (user_access(ObjectHelper :: $ADD_FEDORA_STREAMS)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($pid, 'write');
      }
      if ($allow) {
        $replaceImage = '<a title="' . t("Replace datastream") . " " . $dataStreamValue->label . '" href="' . $base_url . '/fedora/repository/replaceStream/' . $pid . '/' . $dataStreamValue->ID . '/' . $dataStreamValue->label . '"><img src="' . $base_url . '/' . $path . '/images/replace.png" alt="replace datastream" /></a>';
      }
    }

    $content = '';
    $id = $dataStreamValue->ID;
    $label = $dataStreamValue->label;
    $label = str_replace("_", " ", $label);
    $mimeType = $dataStreamValue->MIMEType;

    $view = '<a href="' . $base_url . '/fedora/repository/' . drupal_urlencode($pid) . '/' . $id . '/' . drupal_urlencode($label) .
        '" target="_blank" >' . t('View') . '</a>';
    $action = "$base_url/fedora/repository/object_download/" . drupal_urlencode($pid) . '/' . $id . '/' . drupal_urlencode(preg_replace('/\//i', '${1}_', $label)); // Necessary to handle the case of Datastream labels that contain slashes. Ugh.
    $downloadVersion = '<form method="GET" action="' . $action . '"><input type="submit" value="' . t('Download') . '"></form>';
    if (user_access(ObjectHelper :: $EDIT_FEDORA_METADATA)) {
      $versions = $item->get_datastream_history($id);
      if (is_array($versions)) {
        $downloadVersion = '<form method="GET" action="' . $action . '" onsubmit="this.action=\'' . $action . '\' + \'/\'+this.version.value;">';
        $downloadVersion .= '<input type="submit" value="' . t('Download') . '">';
        $downloadVersion .= '<select name="version">';
        foreach ($versions as $version) {
          $downloadVersion .= '<option>' . $version->createDate . '</option>';
        }
        $downloadVersion .= '</select></form>';
      }
    }

    $content .= "<tr><td>$label</td><td>&nbsp;$view</td><td>&nbsp;$downloadVersion</td><td>&nbsp;$mimeType</td><td>&nbsp;$replaceImage&nbsp;$purgeImage</td></tr>\n";
    return $content;
  }

  /**
   * getFormattedDC ??
   * @global type $base_url
   * @param type $item
   * @return type 
   */
  function getFormattedDC($item) {
    global $base_url;
    $path = drupal_get_path('module', 'fedora_repository');
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');

    $dsid = array_key_exists('QDC', $item->get_datastreams_list_as_array()) ? 'QDC' : 'DC';
    $xmlstr = $item->get_datastream_dissemination($dsid);


    if (empty($xmlstr)) {
      return '';
    }

    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      return;
    }

    $proc->setParameter('', 'baseUrl', $base_url);
    $proc->setParameter('', 'path', $base_url . '/' . $path);
    $input = NULL;
    $xsl = new DomDocument();
    try {
      $xsl->load($path . '/xsl/convertQDC.xsl');
      $input = new DomDocument();
      $input->loadXML(trim($xmlstr));
    } catch (Exception $e) {
      watchdog('fedora_repository', "Problem loading XSL file: @e", array('@e' => $e->getMessage()), NULL, WATCHDOG_ERROR);
    }
    $xsl = $proc->importStylesheet($xsl);
    $newdom = $proc->transformToDoc($input);
    $output = $newdom->saveHTML();
    return $output;
  }

  /**
   * Queries fedora for what we call the qualified dublin core.  Currently only dc.coverage has
   * any qualified fields
   * Transforms the returned xml to html
   * This is the default metadata view.  With icons for searching a dublin core field
   * @param $pid String
   * @return String
   */
  function getQDC($pid) {
    global $base_url;
    $item = new Fedora_Item($pid);
    $ds_list = $item->get_datastreams_list_as_array();
    $output = $this->getFormattedDC($item);
    $dsid = array_key_exists('QDC', $ds_list) ? 'QDC' : 'DC';
    $path = drupal_get_path('module', 'fedora_repository');

    //$baseUrl=substr($baseUrl, 0, (strpos($baseUrl, "/")-1));
    if (user_access(ObjectHelper :: $EDIT_FEDORA_METADATA)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($pid, 'write');
      }
      if ($allow) {

        $output .= '<br /><a title = "' . t('Edit Meta Data') . '" href="' . $base_url . '/fedora/repository/' . 'editmetadata/' . $pid . '/' .
            $dsid . '"><img src="' . $base_url . '/' . $path . '/images/edit.gif" alt="' . t('Edit Meta Data') . '" /></a>';
      }
    }
    return $output;
  }

  /**
   * Gets a list of datastreams from an object using its pid
   *
   * We make some assumptions here.  We have implemented a policy that
   * we ingest in our repository will have TN (thumbnail) datastream.  Even audio
   * will have a picture of a speaker or something.  This is not critical
   * but makes searches etc. look better if there is a TN stream.
   * This diplays all the streams in a collapsed fieldset at the bottom of the object page.
   * you can implement a content model if you would like certain streams displayed in certain ways.
   * @param $object_pid String
   * @return String
   *
   */
  function get_formatted_datastream_list($object_pid, $contentModels, &$fedoraItem) {
    global $fedoraUser, $fedoraPass, $base_url, $user;
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'ContentModel');

    $path = drupal_get_path('module', 'fedora_repository');
    $dataStreamBody = '';
    $fedoraItem = new Fedora_Item($object_pid);

    if (user_access(ObjectHelper :: $VIEW_DETAILED_CONTENT_LIST)) {
      $availableDataStreamsText = 'Detailed List of Content';
      //$metaDataText='Description';
      $mainStreamLabel = NULL;
      $object = $fedoraItem->get_datastreams_list_as_SimpleXML();
      if (!isset($object)) {
        drupal_set_message(t("No datastreams available"));
        return ' ';
      }
      $hasOBJStream = NULL;
      $hasTNStream = FALSE;
      $dataStreamBody = "<br /><table>\n";

      $cmDatastreams = array();
      if (variable_get('fedora_object_restrict_datastreams', FALSE) == TRUE && ($cm = ContentModel::loadFromObject($object_pid)) !== FALSE) {
        $cmDatastreams = $cm->listDatastreams();
      }

      $dataStreamBody .= $this->get_parent_objects_asHTML($object_pid);
      $dataStreamBody .= '<tr><th colspan="4"><h3>' . t("!text", array('!text' => $availableDataStreamsText)) . '</h3></th></tr>';
      foreach ($object as $datastream) {
        foreach ($datastream as $datastreamValue) {
          if (variable_get('fedora_object_restrict_datastreams', FALSE) == FALSE || ((isset($user) && in_array('administrator', $user->roles)) || in_array($datastreamValue->ID, $cmDatastreams))) {
            if ($datastreamValue->ID == 'OBJ') {
              $hasOBJStream = '1';
              $mainStreamLabel = $datastreamValue->label;
              $mainStreamLabel = str_replace("_", " ", $mainStreamLabel);
            }
            if ($datastreamValue->ID == 'TN') {
              $hasTNStream = TRUE;
            }
            //create the links to each datastream
            $dataStreamBody .= $this->create_link_for_ds($object_pid, $datastreamValue); //"<tr><td><b>$key :</b></td><td>$value</td></tr>\n";
          }
        }
      }
      $dataStreamBody .= "</table>\n";
      //if they have access let them add a datastream
      if (user_access(ObjectHelper :: $ADD_FEDORA_STREAMS)) {
        $allow = TRUE;
        if (module_exists('fedora_fesl')) {
          $allow = fedora_fesl_check_roles($object_pid, 'write');
        }
        if ($allow) {
          $dataStreamBody .= drupal_get_form('add_stream_form', $object_pid);
        }
      }
      $fieldset = array(
        '#title' => t("!text", array('!text' => $availableDataStreamsText)),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $dataStreamBody
      );
      $dataStreamBody = '<div class = "fedora_detailed_list">' . theme('fieldset', $fieldset) . '</div>';

      return $dataStreamBody;
    }
    return '';
  }

  /**
   * returns a stream from a fedora object given a pid and dsid
   *
   */
  function getStream($pid, $dsid, $showError = FALSE) {
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
    $soapHelper = new ConnectionHelper();
    try {
      $client = $soapHelper->getSoapClient(variable_get('fedora_soap_url', 'http://localhost:8080/fedora/services/access?wsdl'));
      $params = array(
        'pid' => "$pid",
        'dsID' => "$dsid",
        'asOfDateTime' => ""
      );

      if (!isset($client)) {
        drupal_set_message(t('Error connection to Fedora using soap client.'));
        return NULL;
      }
      $object = $client->__soapCall('getDatastreamDissemination', array('parameters' => $params));
    } catch (Exception $e) {
      if ($showError) {
        drupal_set_message(t('Error getting Datastream for %pid and %datastream<br />', array('%pid' => $pid, '%datastream' => $dsid)), 'error');
      }
      return NULL;
    }
    $content = $object->dissemination->stream;
    $content = trim($content);
    return $content;
  }

  /**
   * gets the name of the content models for the specified object
   * this now returns an array of pids as in Fedora 3 we can have more then one Cmodel for an object
   * @param type $pid
   * @param type $include_fedora_system_content_models
   * @return array 
   */
  function get_content_models_list($pid, $include_fedora_system_content_models = FALSE) {
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    module_load_include('inc', 'fedora_repository', 'ContentModel');
    $collectionHelper = new CollectionClass();
    $pids = array();
    $query = 'select $object from <#ri>
            where <info:fedora/' . $pid . '> <fedora-model:hasModel> $object
            and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>';
    $content_models = $collectionHelper->getRelatedItems($pid, $query);

    if (empty($content_models)) {
      return $pids;
    }

    try {
      $sxml = new SimpleXMLElement($content_models);
    } catch (exception $e) {
      watchdog(t("Fedora_Repository"), "Could not find a parent object for %s", $pid, NULL, WATCHDOG_ERROR);
      return $pids;
    }

    if (!isset($sxml)) {
      return $pids;
    }
    $cmodels = array();
    foreach ($sxml->xpath('//@uri') as $uri) {
      if (strpos($uri, 'fedora-system:FedoraObject-3.0') != FALSE && $include_fedora_system_content_models == FALSE) {
        continue;
      }
      $cmodel_pid = substr(strstr($uri, '/'), 1);
      $cm = ContentModel::loadFromModel($cmodel_pid);
      if ($cm) {
        $cmodels[] = $cm;
      }
    }

    return $cmodels;
  }

  /**
   * determines whether we can see the object or not
   * checks PID namespace permissions, and user permissions
   * @global type $user
   * @param type $op
   * @param type $pid
   * @return type 
   */
  function fedora_repository_access($op, $pid) {
    global $user;

    $returnValue = FALSE;
    $isRestricted = variable_get('fedora_namespace_restriction_enforced', TRUE);
    if (!$isRestricted) {
      $namespaceAccess = TRUE;
    }
    if ($pid == NULL) {
      $pid = variable_get('fedora_repository_pid', 'islandora:root');
    }
    $nameSpaceAllowed = explode(" ", variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '));
    $pos = NULL;
    foreach ($nameSpaceAllowed as $nameSpace) {
      $pos = stripos($pid, $nameSpace);
      if ($pos === 0) {
        $namespaceAccess = TRUE;
      }
    }
    if ($namespaceAccess) {
      $user_access = user_access($op);
      if ($user_access == NULL) {
        return FALSE;
      }
      return $user_access;
    }
    else {
      return FALSE;
    }
  }

  /**
   * internal function
   * uses an xsl to parse the sparql xml returned from the ITQL query
   *
   *
   * @param $content String
   */
  function parseContent($content, $pid, $dsId, $collection, $pageNumber = NULL) {
    $path = drupal_get_path('module', 'fedora_repository');
    global $base_url;
    $collection_pid = $pid; //we will be changing the pid later maybe
    //module_load_include('php', ''Fedora_Repository'', 'ObjectHelper');
    $objectHelper = $this;
    $parsedContent = NULL;
    $contentModels = $objectHelper->get_content_models_list($pid);
    $isCollection = FALSE;
    //if this is a collection object store the $pid in the session as it will come in handy
    //after a purge or ingest to return to the correct collection.

    $fedoraItem = NULL;
    $datastreams = $this->get_formatted_datastream_list($pid, $contentModels, $fedoraItem);

    if (!empty($contentModels)) {
      foreach ($contentModels as $contentModel) {
        if ($contentModel == variable_get('fedora_collection_model_pid', 'islandora:collectionCModel')) {
          $_SESSION['fedora_collection'] = $pid;
          $isCollection = TRUE;
        }
      }
    }

    if ($fedoraItem !== NULL) {
      $dslist = $fedoraItem->get_datastreams_list_as_array();
      if (isset($dslist['COLLECTION_POLICY'])) {
        $isCollection = TRUE;
      }
    }
    //--------------------------------------------------------------------------------
    //show the collections datastreams
    if ($results->length > 0 || $isCollection == TRUE) {
      //  if(strlen($objectList)>22||$contentModel=='Collection'||$contentModel=='Community')//length of empty dom still equals 22 because of <table/> etc
      module_load_include('inc', 'Fedora_Repository', 'CollectionPolicy');
      $collectionPolicyExists = $objectHelper->getMimeType($pid, CollectionPolicy::getDefaultDSID());
      if (user_access(ObjectHelper :: $INGEST_FEDORA_OBJECTS) && $collectionPolicyExists) {
        if (!empty($collectionPolicyExists)) {
          $allow = TRUE;
          if (module_exists('fedora_fesl')) {
            $allow = fedora_fesl_check_roles($pid, 'write');
          }
          if ($allow) {
            // $ingestObject = '<a title="'. t('Ingest a New object into ') . $collectionName . ' '. $collection_pid . '" href="'. base_path() .
            $ingestObject = '<a title="' . t('Ingest a New object into !collection_name PID !collection_pid', array('!collection_name' => $collectionName, '!collection_pid' => $collection_pid)) . '" href="' . base_path() .
                'fedora/ingestObject/' . $collection_pid . '/' . $collectionName . '"><img src="' . $base_url . '/' . $path .
                '/images/ingest.png" alt="' . t('Add a New Object') . '" class="icon"></a> ' . t('Add to this Collection');
          }
        }
      }
      else {
        $ingestObject = '&nbsp;';
      }
    }

    $datastreams .= $ingestObject;


    $output .= $datastreams;

    $showDesc = FALSE;
    switch (variable_get('fedora_object_display_description', ObjectHelper :: $DISPLAY_NO_MODEL_OUTPUT)) {
      case ObjectHelper :: $DISPLAY_NEVER: break;
      case ObjectHelper :: $DISPLAY_NO_MODEL_OUTPUT:
        if (trim($datastreams) == '') {
          $showDesc = TRUE;
        }
        break;

      case ObjectHelper :: $DISPLAY_ALWAYS:
      default:
        $showDesc = TRUE;
        break;
    }
    if ($showDesc) {
      //just show default dc or qdc as we could not find a content model
      $metaDataText = t('Description');
      $body = $this->getQDC($pid);
      $fieldset = array(
        '#title' => t("!metaDataText", array('!metaDataText' => $metaDataText)),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $body
      );
      $output .= theme('fieldset', $fieldset);
    }


    return $output;
  }

  /**
   * Gets the parent objects that this object is related to
   *
   * @param unknown_type $pid
   * @return unknown
   */
  function get_parent_objects($pid) {
    $query_string = 'select $object $title from <#ri>
                            where ($object <fedora-model:label> $title
                              and <info:fedora/' . $pid . '> <fedora-rels-ext:isMemberOfCollection> $object
                              and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
                            order by $title';
    $objects = $this->getCollectionInfo($pid, $query_string);
    return $objects;
  }

  /**
   * get_parent_objects_asHTML ??
   * @global type $base_url
   * @param type $pid
   * @return string 
   */
  function get_parent_objects_asHTML($pid) {
    global $base_url;
    $parent_collections = $this->get_parent_objects($pid);
    try {
      $parent_collections = new SimpleXMLElement($parent_collections);
    } catch (exception $e) {
      drupal_set_message(t('Error getting parent objects @e', array('@e' => check_plain($e->getMessage()))));
      return;
    }

    $parent_collections_HTML = '';
    foreach ($parent_collections->results->result as $result) {
      $collection_label = $result->title;
      foreach ($result->object->attributes() as $a => $b) {
        if ($a == 'uri') {
          $uri = (string) $b;
          $uri = $base_url . '/fedora/repository' . substr($uri, strpos($uri, '/')) . '/-/' . $collection_label;
        }
      }
      $parent_collections_HTML .= '<a href="' . $uri . '">' . $collection_label . '</a><br />';
    }
    if (!empty($parent_collections_HTML)) {
      $parent_collections_HTML = '<tr><td><h3>' . t("Belongs to these collections:") . ' </h3></td><td colspan="4">' . $parent_collections_HTML . '</td></tr>';
    }

    return $parent_collections_HTML;
  }

  /**
   * gets a list of datastreams and related function that we should use to show datastreams in their own fieldsets
   * from the content model associated with the object
   *
   * @param type $pid
   * @param type $contentModel
   * @param type $page_number
   * @return type 
   */
  function createExtraFieldsets($pid, $contentModel, $page_number) {
    //$models = $collectionHelper->getContentModels($collectionPid, FALSE);
    // possible problem in below if the collection policy has multiple content models
    //with different pids but same dsid we could get wrong one,  low probability and functionality
    // will probably change for fedora version 3.
    if (empty($contentModel)) {
      return NULL;
    }
    $output = '';
    module_load_include('inc', 'fedora_repository', 'ContentModel');
    if (($cm = ContentModel :: loadFromModel($contentModel)) !== FALSE && $cm->validate()) {
      $output .= $cm->displayExtraFieldset($pid, $page_number);
    }
    return $output;
  }

  /**
   * Look in the content model for rules to run on the specified datastream.
   *
   * @param string $pid
   * @param string $dsid
   * @return boolean
   */
  function get_and_do_datastream_rules($pid, $dsid, $file = '') {
    if (!user_access('ingest new fedora objects')) {
      drupal_set_message(t('You do not have permission to add datastreams.'));
      return FALSE;
    }

    module_load_include('inc', 'fedora_repository', 'ContentModel');
    if ($dsid != NULL && $pid != NULL && ($cm = ContentModel::loadFromObject($pid)) !== FALSE) {
      $cm->execAddDatastreamMethods($dsid, $file);
    }
  }

  /**
   * Get a tree of related pids - for the basket functionality
   *
   * @param type $pid
   * @return type 
   */
  function get_all_related_pids($pid) {
    if (!$pid) {
      return FALSE;
    }
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

    // Get title and descriptions for $pid 
    $query_string = 'select $title $desc from <#ri> 
      where $o <fedora-model:label> $title
      and $o <dc:description> $desc
      and $o <mulgara:is> <info:fedora/' . $pid . '>';

    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    $url .= "?type=tuples&flush=TRUE&format=csv&limit=1000&lang=itql&stream=on&query=";
    $content = do_curl($url . htmlentities(urlencode($query_string)));

    $rows = explode("\n", $content);
    $fields = explode(',', $rows[1]);

    $pids[$pid] = array('title' => $fields[0], 'description' => $fields[1]);

//    $pids += $this->get_child_pids(array($pid));

    return $pids;
  }

  /**
   * Get children of PID - but only 2 levels deep
   *
   * @param type $pids
   * @return type 
   */
  function get_child_pids($pids) {
    // Get pid, title and description for children of object $pid
    $query_string = 'select $o $title from <#ri> ' .
//    $query_string = 'select $o $title $desc from <#ri> '.
        'where $s <info:fedora/fedora-system:def/relations-external#hasMember> $o ' .
        'and $o <fedora-model:label> $title ' .
//    'and $o <dc:description> $desc '.
        'and ( ';

    foreach ($pids as $pid) {
      $query_string .= '$s <mulgara:is> <info:fedora/' . $pid . '> or ';
    }
    $query_string = substr($query_string, 0, -3) . ' )';

    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    $url .= "?type=tuples&flush=TRUE&format=csv&limit=1000&lang=itql&stream=on&query=";
    $url .= htmlentities(urlencode($query_string));
    $content = $this->doCurl($url);

    $rows = explode("\n", $content);
    // Knock of the first heading row
    array_shift($rows);

    $child_pids = array();
    if (count($rows)) {
      // iterate through each row
      foreach ($rows as $row) {
        if ($row == "") {
          continue;
        }
        $fields = explode(',', $row);
        $child_pid = substr($fields[0], 12);
        $child_pids[$child_pid] = array('title' => $fields[1], 'description' => $fields[2]);
      }
      if (!empty($child_pids)) {
        $child_pids += $this->get_child_pids(array_keys($child_pids));
      }
    }

    return $child_pids;
  }

  /**
   * Returns XML description of the object (export).
   *
   * @param type $pid
   * @param type $context
   * @param type $format
   * @return type 
   */
  function getObject($pid, $context = 'archive', $format = FOXML_11) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

    $url = variable_get('fedora_base_url', 'http://localhost:8080/fedora') . '/objects/' . $pid . '/export?context=' . $context . '&format=' . $format;
    $result_data = do_curl($url);
    return $result_data;
  }

  /**
   * Builds an array of drupal links for use in breadcrumbs. 
   *
   * @global type $base_url
   * @param type $pid
   * @param type $breadcrumbs
   * @param type $level 
   */
  function getBreadcrumbs($pid, &$breadcrumbs, $level=10) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    // Before executing the query, we hve a base case of accessing the top-level collection
    global $base_url;
    if ($pid == variable_get('fedora_repository_pid', 'islandora:root')) {
      //$breadcrumbs[] = l(t('Digital repository'), 'fedora/repository');
      $breadcrumbs[] = l(variable_get('fedora_repository_title', 'Digital repository'), 'fedora/repository');
      $breadcrumbs[] = l(t('Home'), $base_url);
    }
    else {
      $query_string = 'select $parentObject $title $content from <#ri>
                               where (<info:fedora/' . $pid . '> <fedora-model:label> $title
                               and $parentObject <fedora-model:hasModel> $content
                               and (<info:fedora/' . $pid . '> <fedora-rels-ext:isMemberOfCollection> $parentObject
                               or <info:fedora/' . $pid . '> <fedora-rels-ext:isMemberOf> $parentObject
                               or <info:fedora/' . $pid . '> <fedora-rels-ext:isPartOf> $parentObject)
                               and $parentObject <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
                               minus $content <mulgara:is> <info:fedora/fedora-system:FedoraObject-3.0>
                               order by $title desc';
      $query_string = htmlentities(urlencode($query_string));

      $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
      $url .= "?type=tuples&flush=TRUE&format=CSV&limit=1&offset=0&lang=itql&stream=on&query=" . $query_string;
      
      $result = preg_split('/[\r\n]+/', do_curl($url));
      array_shift($result); // throw away first line
      $matches = str_getcsv(join("\n", $result));
      if (count($matches) >= 2) {
        $parent = preg_replace('/^info:fedora\//', '', $matches[0]);

        if (0 == strlen($matches[1])) {
          $matches[1] = "Unlabeled Object";
        }
        
        $breadcrumbs[] = l($matches[1], 'fedora/repository/' . $pid);
        if ($parent == variable_get('fedora_repository_pid', 'islandora:root')) {
          //$breadcrumbs[] = l(t('Digital repository'), 'fedora/repository');
          $breadcrumbs[] = l(variable_get('fedora_repository_title', 'Digital repository'), 'fedora/repository');
          $breadcrumbs[] = l(t('Home'), $base_url);
        }
        elseif ($level > 0) {
          $this->getBreadcrumbs($parent, $breadcrumbs, $level - 1);
        }
      }
      
      else {
        $breadcrumbs[] = l(t("Path Calculation Error"), 'fedora/repository/' . $pid);
      }
    }
  }

  /**
   * warnIfMisconfigured ??
   * @param type $app 
   */
  public static function warnIfMisconfigured($app) {
    $messMap = array(
      'Kakadu' => 'Full installation instructions for Kakadu can be found
          <a href=http://www.kakadusoftware.com/index.php?option=com_content&task=view&id=27&Itemid=23>Here</a>',
      'ImageMagick' => 'Check the path settings in the configuration of your <b>imageapi</b> module.<br/>
          Further details can be found <a href=http://www.imagemagick.org/script/install-source.php>Here</a>',
    );

    $warnMess = "Creation of one or more datastreams failed.<hr  width='40%' align = 'left'/>";
    $configMess = "Please ensure that %app is installed and configured for this site. ";
    drupal_set_message($warnMess, 'warning', FALSE);
    drupal_set_message(t($configMess . "<br />" . $messMap[$app] . "<hr  width='40%' align = 'left'/>", array('%app' => $app)), 'warning', FALSE);
  }

}

