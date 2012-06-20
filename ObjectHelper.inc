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

  // TODO: Make this into a static member constant
  public $availableDataStreamsText = 'Detailed list of content';

  /**
   * Constructor
   */
  function ObjectHelper() {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
    $connectionHelper = new ConnectionHelper();
  }
  
  /**
   * Get the size of the indicated datastream.
   * 
   * Note that this only works for Inline XML and Managed datastream, as Fedora
   * does not know anything about external or redirect streams, other than a URI.
   * If run on an external or redirect stream, Fedora (and therefore this
   * function) will return the integer value '0'.
   * This function requires the use of the API-M method getDatastream, and defaults
   * to returning '0' if it is not accessible.
   * 
   * @param $pid string
   *   A string containing the PID of a Fedora object.
   * @param $dsid string
   *   A string containing the datastream ID of datastream on the Fedora object
   *   indicated by the PID.
   * @param $quiet boolean
   *   A boolean indicating whether SOAP errors should be displayed.
   * @return integer
   *   An integer representing the size of the datastream, or zero if it could not be determined.  
   */
  static function getDatastreamSize($pid, $dsid, $quiet = FALSE) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);
    if ($ds_info = $item->get_datastream($dsid, '', $quiet)) {
      return $ds_info->size;
    }
    else {
      return 0;
    }
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
    } else {
      $fedoraUser = $user->name;
      $fedoraPass = $user->pass;
    }
    
    $dataStreamInfo = $item->get_datastream_info($dsID);
    $contentSize = $dataStreamInfo->datastream->size;

    if (function_exists("curl_init")) {
      $url = variable_get('fedora_base_url', 'http://localhost:8080/fedora') . '/objects/' . $pid . '/datastreams/' . $dsID . '/content';
      $query_options = array();
      if ($version) {
        $query_options['asOfDateTime'] = $version; //drupal_urlencode($version);
      }
      if ($query_options) {
        $url = url($url, array(
          'query' => $query_options,
        ));
      }
      $ch = curl_init($url);
      
      $user_agent = "Mozilla/4.0 pp(compatible; MSIE 5.01; Windows NT 5.0)"; 
      $curl_opts = array(
        CURLOPT_SSL_VERIFYPEER => FALSE,
        CURLOPT_SSL_VERIFYHOST => FALSE,
        CURLOPT_FAILONERROR => TRUE, // Fail on errors
        CURLOPT_FOLLOWLOCATION => TRUE, // allow redirects
        //CURLOPT_TIMEOUT => 15, // times out after 15s
        CURLOPT_USERAGENT => $user_agent,
        CURLOPT_USERPWD => "$fedoraUser:$fedoraPass",
        // There seems to be a bug in Fedora 3.1's REST authentication, removing this line fixes the authorization denied error.
        //CURLOPT_HTTPAUTH => CURLAUTH_ANY,
        CURLOPT_RETURNTRANSFER => TRUE, // return into a variable; need to at first, so additional headers can be set later.
      );
      curl_setopt_array($ch, $curl_opts);

      if ($filePath !== FALSE) {
        $fp = fopen($filePath, 'w');
        curl_setopt($ch, CURLOPT_FILE, $fp);
        curl_setopt($ch, CURLOPT_HEADER, 0);
        curl_exec($ch);
        fclose($fp);
      }
      else {
        curl_setopt($ch, CURLOPT_NOBODY, TRUE);
        $curl_stat = curl_exec($ch);
        
        if ($curl_stat !== FALSE) {
          $info = curl_getinfo($ch);
          
          //Set what headers we can...
          if ($mimeType = $info['content_type']) {
            drupal_set_header("Content-Type: $mimeType");
            
            if ($asAttachment) {
              $suggestedFileName = "$label";
              $pos = strpos($suggestedFileName, '.');

              /*
               * Here we used to take an object of, say, type application/pdf with
               * label, say, "My Document" and we assemble the output filename
               * extension based on the post-slash portion of the mimetype. (If the
               * label has a period anywhere in it, we leave it alone.)
               *
               * This is great for simple mimetypes like application/pdf, text/html,
               * image/jpeg, etc., but it's terrible for, say
               * application/vnd.oasis.opendocument.text (.odt).
               *
               * Instead we'll use the get_extension function in MimeClass.inc to
               * discover a valid extension for the mimetype in question.
               */
              if ($pos === FALSE) {
                module_load_include('inc', 'fedora_repository', 'MimeClass');
                $mimeclass = new MimeClass();
                $ext = $mimeclass->get_extension($mimeType);
                $suggestedFileName = "$label.$ext";
              }

              drupal_set_header('Content-Disposition: attachment; filename="' . $suggestedFileName . '"');
            }
          }
          
          $effective_url = $info['url'];
          //dd($info, 'header/nobody info');
          if ($url !== $effective_url) { //Handle redirect streams (the final URL is not the same as the Fedora URL)
          
            //Add the parameters passed to Drupal, leaving out the 'q'
            $query = array();
            parse_str($_SERVER['QUERY_STRING'], $query);
            if (isset($query['q'])) {
              unset($query['q']);
            }
            
            drupal_set_header('HTTP/1.1 307 Moved Temporarily');
            drupal_set_header('Location: ' . url($effective_url, array('query' => $query)));
          }
          elseif ((isset($user) && $user->uid != 0) || $forceSoap || isset($_SERVER['HTTPS'])) { //If not anonymous, soap is force or we're using HTTPS
            //Have the webserver mediate the transfer (download and restream)
            
            if (($contentSize = self::getDatastreamSize($pid, $dsID, TRUE)) > 0) {
              drupal_set_header("Content-Length: $contentSize");
            }
            
            $opts = array(
              CURLOPT_NOBODY => FALSE,
              CURLOPT_HTTPGET => TRUE, //CURLOPT_NOBODY sets it to 'HEAD'
              CURLOPT_RETURNTRANSFER => FALSE, //Want to output as efficiently as possible now...
            );
            curl_setopt_array($ch, $opts);
            $curl_stat = curl_exec($ch);
            
            if (!$curl_stat) {
              watchdog('fedora_repository', 'Error in ObjectHelper->makeObject() for @pid/@dsid.  See link for attempted URL.', array(
                '@pid' => $pid,
                '@dsid' => $dsID,
              ), WATCHDOG_WARNING, $url);
            }
          }
          else { //Try to redirect directly to Fedora.
            drupal_set_header('HTTP/1.1 307 Moved Temporarily');
            drupal_set_header('Location: ' . $url);
          }
        }
        else {
          watchdog('fedora_repository', 'Curl error while trying to get datastream %dsid from Fedora object %pid.  Curl info: !info', array(
            '%pid' => $pid,
            '%dsid' => $dsID,
            '!info' => print_r(curl_getinfo($ch), TRUE),
          ), WATCHDOG_WARNING);
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
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');

    $item = new Fedora_Item($pid);

    $purge_image = '&nbsp;';
    if (user_access(ObjectHelper :: $PURGE_FEDORA_OBJECTSANDSTREAMS)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($pid, 'write');
      }
      if ($allow) {
        $purge_text = t('Purge datastream "@label"', array('@label' => $dataStreamValue->label));
        $purge_path = "fedora/repository/purgeStream/$pid/{$dataStreamValue->ID}/{$dataStreamValue->label}";
        $purge_image = l(theme('image', "$path/images/purge.gif", $purge_text, $purge_text, NULL, FALSE), $purge_path, array(
          'html' => TRUE,
        ));
      }
    }
    else {
      $purge_image = '&nbsp;';
    }

    // Add an icon to replace a datastream
    // @TODO Note: using l(theme_image(..), ...); for these image links (and other links) may remove the need to have clean urls enabled.
    $replace_image = '&nbsp;';
    if (user_access(ObjectHelper :: $ADD_FEDORA_STREAMS)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($pid, 'write');
      }
      if ($allow) {
        $replace_text = t('Replace datastream "@label"', array('@label' => $dataStreamValue->label));
        $replace_path = "fedora/repository/replaceStream/$pid/{$dataStreamValue->ID}/{$dataStreamValue->label}";
        $replace_image = l(theme('image', "$path/images/replace.png", $replace_text, $replace_text, NULL, FALSE), $replace_path, array(
          'html' => TRUE,
        ));
      }
    }

    $content = '';
    $id = $dataStreamValue->ID;
    $label = $dataStreamValue->label;
    //$label = str_replace("_", " ", $label);
    $label_deslashed = preg_replace('/\//i', '${1}_', $label); // Necessary to handle the case of Datastream labels that contain slashes. Ugh.
    $mimeType = $dataStreamValue->MIMEType;

    $view = l(t('View'), "fedora/repository/$pid/$id/$label_deslashed", array(
      'attributes' => array(
        'target' => '_blank',
      ),
    ));
    
    $downloadVersion = drupal_get_form('fedora_repository_download_datastream_form', $pid, $id, $label_deslashed);

    return array(
      array(
        'data' => $label,
      ),
      array(
        'data' => $view,
      ),
      array(
        'data' => $downloadVersion,
      ),
      array(
        'data' => $mimeType
      ),
      array(
        'data' => $replace_image . $purge_image,
      ),
    );
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

    $dsid = array_key_exists('QDC', $item->get_datastreams_list_as_array()) ? 'QDC' : 'DC';
    $xmlstr = $item->get_datastream_dissemination($dsid);
    
    if (empty($xmlstr)) {
      return '';
    }
    
    if (($xsl_path = "$path/xsl/convertQDC.xsl") &&
      (is_readable($xsl_path)) &&
      ($xsl = DOMDocument::load($xsl_path)) &&  //Fails loading XSLT -> FALSE
      ($ds = DOMDocument::loadXML($xmlstr))) {
      $xslt_opts = array(
        'BASEURL' => $base_url,
        'PATH' => url($path, array('absolute' => TRUE)),
        'baseUrl' => $base_url, //XXX: Deprecated; just here for legacy cases.
        'path' => url($path, array('absolute' => TRUE)), //XXX: Deprecated; just here for legacy cases.
      );
      $transform = new XSLTProcessor();
      $transform->importStylesheet($xsl);
      $transform->setParameter('', $xslt_opts);
      $transformed = $transform->transformToDoc($ds);
      return $transformed->saveHTML();
    }
    else {
      $simplexml = new SimpleXMLElement($xmlstr);

      $headers = array(
        array(
          'data' => t('Metadata'),
          'colspan' => 2,
        ),
      );
      $rows = array();
      foreach ($simplexml->getNamespaces(TRUE) as $ns) {
        foreach ($simplexml->children($ns) as $child) {
          $data = array();
          $rendered_data = '';
          if ($grand_children = $child->children()) {
            foreach($grand_children as $grand_child) {
              $data[] = $grand_child->getName() . ' = ' . (string)$grand_child;
            }
          }
          else {
            $rendered_data = (string)$child;
          }
          
          if ($data) {
            $rendered_data = theme('item_list', $data);
          }
          
          if ($rendered_data) {
            $rows[] = array(
              array(
                'data' => $child->getName(),
                'class' => 'dc-tag-name',
              ),
              array(
                'data' => $rendered_data,
                'class' => 'dc-content',
              ),
            );
          }
          
          
        }
      }

      return theme('table', $headers, $rows, array('class' => 'dc-table'));
    }
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

    if (user_access(ObjectHelper :: $EDIT_FEDORA_METADATA)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($pid, 'write');
      }
      if ($allow) {
        $link_image = theme('image', "$path/images/edit.gif", t('Edit Metadata'));
        $link = l($link_image, "fedora/repository/editmetadata/$pid", array(
          'html' => TRUE,
        ));
        $output .= '<br />' . $link;
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
    global $base_url, $user;
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'ContentModel');

    $path = drupal_get_path('module', 'fedora_repository');
    $dataStreamBody = '';
    $fedoraItem = new Fedora_Item($object_pid);

    if (user_access(ObjectHelper :: $VIEW_DETAILED_CONTENT_LIST)) {
      $availableDataStreamsText = 'Detailed List of Content';

      $mainStreamLabel = NULL;
      $object = $fedoraItem->get_datastreams_list_as_SimpleXML();
      if (!isset($object)) {
        drupal_set_message(t("No datastreams available"));
        return ' ';
      }

      $cmDatastreams = array();
      if (variable_get('fedora_object_restrict_datastreams', FALSE) == TRUE && ($cm = ContentModel::loadFromObject($object_pid)) !== FALSE) {
        $cmDatastreams = $cm->listDatastreams();
      }

      $headers = array(
        array(
          'data' => $availableDataStreamsText,
          'colspan' => 4,
        ),
      );
      $DSs = array();
      foreach ($object as $datastream) {
        foreach ($datastream as $datastreamValue) {
          if (variable_get('fedora_object_restrict_datastreams', FALSE) == FALSE || ((isset($user) && in_array('administrator', $user->roles)) || in_array($datastreamValue->ID, $cmDatastreams))) {  
            //create the links to each datastream
            $DSs []= $this->create_link_for_ds($object_pid, $datastreamValue);
          }
        }
      }
      
      $dataStreamBody = theme('table', $headers, $DSs);
      //if they have access let them add a datastream
      if (user_access(ObjectHelper::$ADD_FEDORA_STREAMS) && //If allowed throw Drupal
        ((module_exists('fedora_fesl') && fedora_fesl_check_roles($object_pid, 'write')) || //And allowed throw FESL 
        !module_exists('fedora_fesl'))) { //Or not using FESL, draw the add datastream form.
        $dataStreamBody .= drupal_get_form('add_stream_form', $object_pid);
      }

      return $dataStreamBody;
    }
    return '';
  }

  /**
   * returns a stream from a fedora object given a pid and dsid
   *
   */
  function getStream($pid, $dsid, $showError = FALSE) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new fedora_item($pid);
    return $item->get_datastream_dissemination($dsid);
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
      watchdog(t("fedora_repository"), "Could not find a parent object for %s", $pid, NULL, WATCHDOG_ERROR);
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
  function fedora_repository_access($op, $pid = NULL, $as_user = NULL) {
    $returnValue = FALSE;
    
    if ($pid == NULL) {
      $pid = variable_get('fedora_repository_pid', 'islandora:root');
    }

    $isRestricted = variable_get('fedora_namespace_restriction_enforced', TRUE);
    $namespace_access = NULL;
    if (!$isRestricted) {
      $namespace_access = TRUE;
    }
    else {    
      $pid_namespace = substr($pid, 0, strpos($pid, ':') + 1); //Get the namespace (with colon)
      $allowed_namespaces = explode(" ", variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '));

      $namespace_access = in_array($pid_namespace, $allowed_namespaces);
    }
    
    return ($namespace_access && user_access($op, $as_user));
  }

  /**
   * Get the query to find parent objects.
   *
   * @param $pid string
   *   A string containing a Fedora PID to find the parents for.
   * @return string
   *   A string containing an iTQL query, selecting something into $object and $title 
   */
  static function parentQuery($pid) {
    return 'select $object $title from <#ri>
            where ($object <fedora-model:label> $title
              and <info:fedora/' . $pid . '> <fedora-rels-ext:isMemberOfCollection> $object
              and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
            order by $title';
  }
  
  /**
   * Gets the parent objects that this object is related to
   *
   * @param $pid string
   *   A string containing a Fedora PID to find the parents for.
   * @return string
   *   A string containing Sparql XML (the results of the self::parentQuery())
   */
  function get_parent_objects($pid) {
    $query_string = self::parentQuery();
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    $collection_class = new CollectionClass($pid);
    $objects = CollectionClass::getRelatedItems($pid, $query_string);
    return $objects;
  }

  /**
   * get_parent_objects_asHTML ??
   * @global type $base_url
   * @param type $pid
   * @return string 
   */
  function get_parent_objects_asHTML($pid) {
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    $results = self::performItqlQuery(self::parentQuery($pid));

    $parent_collections = array();
    foreach ($results as $result) {
      $collection_title = $result['title'];
      $collection_pid = $result['object'];
      $path = "fedora/repository/$collection_pid/-/$collection_title";
      $parent = array(
        'data' => l($collection_title, $path),
      );
      
      $parent_collections[] = $parent;
    }
    
    if (!empty($parent_collections)) {
      return theme('item_list', $parent_collections, t('Belongs to these collections'), 'ul');
    }
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
   * FIXME:  This doesn't actually get a tree...
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
    $query_string = 'select $title $description from <#ri> 
      where $o <fedora-model:label> $title
      and $o <dc:description> $desc
      and $o <mulgara:is> <info:fedora/' . $pid . '>';
    
    $results = self::performItqlQuery($query_string);
    
    $pids = array();
    //There should only be one...  Anyway.
    foreach($results as $result) {
      $pids[$pid] = $result;
    }

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
    //Build the parts which are used to filter to the list of input.
    $query_chunks = array();
    foreach ($pids as $pid) {
      $query_chunks[] = '$s <mulgara:is> <info:fedora/' . $pid . '>';
    }
    // Get pid, title and description for children of object $pid
    $query_string = 'select $o $title from <#ri> ' .
        'where $s <info:fedora/fedora-system:def/relations-external#hasMember> $o ' .
        'and $o <fedora-model:label> $title ' .
        'and ( ' . implode(' or ', $query_chunks) . ' )';
    
    $results = self::performItqlQuery($query_string);

    $child_pids = array();
    if ($results) {
      // iterate through each row
      foreach ($results as $result) {
        $child_pids[$result['o']] = array('title' => $result['title']);
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
   * @todo Make fully recursive...
   *
   * @global type $base_url
   * @param type $pid
   * @param type $breadcrumbs
   * @param type $level 
   */
  function getBreadcrumbs($pid, &$breadcrumbs) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    // Before executing the query, we hve a base case of accessing the top-level collection
    global $base_url;
    static $max_level = 10;
    static $level = -1;
    
    if (count($breadcrumbs) === 0) {
      $level = $max_level;
    }
    
    $root = variable_get('fedora_repository_pid', 'islandora:root');
    
    if ($pid == $root) {
      $breadcrumbs[] = l(menu_get_active_title(), 'fedora/repository');
      $breadcrumbs[] = l(t('Home'), '<front>');
    }
    else {
      $sparql_query_string = <<<EOQ
PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
PREFIX rels-ext: <info:fedora/fedora-system:def/relations-external#>
SELECT ?parentObject ?title ?content
FROM <#ri>
WHERE {
  <info:fedora/$pid> ?relationship ?parentObject .
  ?parentObject fedora-model:label ?title ;
                fedora-model:state fedora-model:Active ;
                fedora-model:hasModel ?content .
  FILTER(
    (
      sameTerm(?relationship, rels-ext:isMemberOfCollection) ||
      sameTerm(?relationship, rels-ext:isMemberOf) ||
      sameTerm(?relationship, rels-ext:isPartOf)
    ) &&
    !sameTerm(?content, <info:fedora/fedora-system:FedoraObject-3.0>)
  ) .
}
ORDER BY DESC(?title)
EOQ;

      $results = self::performSparqlQuery($sparql_query_string);
      $next_pid = NULL;
      
      if (count($results) > 0  && $level > 0) {
        $parent_pid = $results[0]['parentObject'];
        $parent_title = $results[0]['title'];

        if (empty($this_title)) {
          $this_title = t('Unlabeled Object');
        }
        
        if ($parent_pid != $root) {
          $breadcrumbs[] = l($parent_title, "fedora/repository/$parent_pid");
        }
        
        $next_pid = $parent_pid;
      }
      else {
        watchdog('fedora_repository', 'Error generating breadcrumbs for %pid.  Verify there exists relationships back up to %root. (May also be due to a hierarchy deeper than %max_depth).', array('%pid' => $pid, '%root' => $root, '%max_depth' => $max_level), WATCHDOG_WARNING);
        $breadcrumbs[] = '...'; //Add an non-link, as we don't know how to get back to the root.
        $next_pid = $root;  //And cue the last two links to render and break recursion (on the next pass).
      }
      
      if ($next_pid !== NULL) {
        $level--;
        $this->getBreadcrumbs($next_pid, $breadcrumbs);
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

  /**
   * Parse the passed in Sparql XML string into a more easily usable format.
   *
   * @param $sparql string
   *   A string containing Sparql result XML.
   * @return array
   *   Indexed (numerical) array, containing a number of associative arrays,
   *   with keys being the same as the variable names in the query.
   *   URIs beginning with 'info:fedora/' will have this beginning stripped
   *   off, to facilitate their use as PIDs.
   */
  public static function parseSparqlResults($sparql) {
    //Load the results into a SimpleXMLElement
    $doc = new SimpleXMLElement($sparql, 0, FALSE, 'http://www.w3.org/2001/sw/DataAccess/rf1/result');
    
    $results = array(); //Storage.
    
    //Build the results.
    foreach ($doc->results->children() as $result) {
      //Built a single result.
      $r = array();
      foreach ($result->children() as $element) {
        $val = NULL;
        
        $attrs = $element->attributes();
        if (!empty($attrs['uri'])) {
          $val = self::pidUriToBarePid((string)$attrs['uri']);
        }
        elseif(!empty($attrs['datatype'])) {
          $dt = (string)$attrs['datatype'];
          $val = (string)$element; //Default to a string...
          if ($dt == 'http://www.w3.org/2001/XMLSchema#int') {
            $val = intval($val);
          }
          else {
            watchdog('fedora_repository', 'Datatype @dt_uri handled as string.', array(
              '@dt_uri' => $dt,
            ), WATCHDOG_DEBUG);
          }
        }
        else {
          $val = (string)$element;
        }
        
        //Map the name to the value in the array.
        $r[$element->getName()] = $val;
      }
      
      //Add the single result to the set to return.
      $results[] = $r;
    }
    return $results;
  }
  
  /**
   * Performs the given Resource Index query and return the results.
   *
   * @param $query string
   *   A string containing the RI query to perform.
   * @param $type string
   *   The type of query to perform, as used by the risearch interface.
   * @param $limit int
   *   An integer, used to limit the number of results to return.
   * @param $offset int
   *   An integer, used to offset the results (results should be ordered, to
   *   maintain consistency.
   * 
   * @return array
   *   Indexed (numerical) array, containing a number of associative arrays,
   *   with keys being the same as the variable names in the query.
   *   URIs beginning with 'info:fedora/' will have this beginning stripped
   *   off, to facilitate their use as PIDs.
   */
  static function performRiQuery($query, $type = 'itql', $limit = -1, $offset = 0) {
    //Setup the query options...
    $options = array(
      'type' => 'tuples',
      'flush' => TRUE,
      'format' => 'Sparql', //Sparql XML is processed into the array below.
      'lang' => $type,
      'query' => $query
    );
    //Add limit if provided.
    if ($limit > 0) {
      $options['limit'] = $limit;
    }
    //Add offset if provided.
    if ($offset > 0) {
      $options['offset'] = $offset;
    }
    
    //Construct the query URL.
    $queryUrl = url(variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch'), array('query' => $options));
    
    //Perform the query.
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    $curl_result = do_curl_ext($queryUrl);
    
    //If the query failed, write message to the logs and return.
    if (!$curl_result[0]) {
      watchdog('fedora_repository', 'Failed to perform %type resource index query: %query', array('%type' => $type, '%query' => $query), WATCHDOG_ERROR);
      return FALSE;
    }
    
    //Pass the query's results off to a decent parser.
    return self::parseSparqlResults($curl_result[0]);
  }
  /**
   * Thin wrapper for self::_performRiQuery().
   *
   * @see self::performRiQuery()
   */
  public static function performItqlQuery($query, $limit = -1, $offset = 0) {
    return self::performRiQuery($query, 'itql', $limit, $offset);
  }
  /**
   * Thin wrapper for self::performRiQuery().
   *
   * @see self::_performRiQuery()
   */
  public static function performSparqlQuery($query, $limit = -1, $offset = 0) {
    return self::performRiQuery($query, 'sparql', $limit, $offset);
  }
  /**
   * Utility function used in self::performRiQuery().
   *
   * Strips off the 'info:fedora/' prefix from the passed in string.
   *
   * @param $uri string
   *   A string containing a URI.
   *
   * @return string
   *   The input string less the 'info:fedora/' prefix (if it has it).
   *   The original string otherwise.
   */
  protected static function pidUriToBarePid($uri) {
    $chunk = 'info:fedora/';
    $pos = strpos($uri, $chunk);
    if ($pos === 0) { //Remove info:fedora/ chunk
      return substr($uri, strlen($chunk));
    }
    else { //Doesn't start with info:fedora/ chunk...
      return $uri;
    }
  }
}

