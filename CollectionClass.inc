<?php

/**
 * @file
 * 
 * Collection Class Class
 */
 
/**
 * This CLASS caches the streams so once you call a getstream once it will always return
 * the same stream as long as you are using the instance of this class. Cached to
 * prevent mutiple hits to fedora.  maybe a bit confusing though if this class is used in 
 * a different context.
 */
class CollectionClass {

  public static $COLLECTION_CLASS_COLLECTION_POLICY_STREAM = 'COLLECTION_POLICY';
  public static $COLLECTION_CLASS_COLLECTION_VIEW_STREAM = 'COLLECTION_VIEW';
  private $contentModelStream = NULL;
  private $collectionPolicyStream = NULL;
  private $collectionViewStream = NULL;
  public $collectionObject = NULL;

  /**
   * Creates a collection object. Optionally can associate it with a single collection with parameter $pid.
   *
   * @param string $pid The pid of the collection to represent.
   * @return CollectionClass
   */
  function __construct($pid = NULL) {
    if (!empty($pid)) {
      module_load_include('inc', 'fedora_repository', 'ObjectHelper');
      $this->collectionObject = new ObjectHelper($pid);
      $this->pid = $pid;
    }
  }
  
  public static function getCollectionQuery($pid) {
    if ($query = self::getCollectionQueryFromStream($pid)) {
      return $query;
    }
    else {
      return self::getDefaultCollectionQuery($pid);
    }
  }
  
  protected static function getCollectionQueryFromStream($pid) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);
    if ($item->exists() && array_key_exists('QUERY', $item->datastreams)) {
      return $item->get_datastream_dissemination('QUERY');
    }
    else {
      return FALSE;
    }
  }
  
  protected static function getDefaultCollectionQuery($pid) {
    return 'select $object $title $content from <#ri>
             where ($object <fedora-model:label> $title
             and $object <fedora-model:hasModel> $content
             and ($object <fedora-rels-ext:isMemberOfCollection> <info:fedora/' . $pid . '>
             or $object <fedora-rels-ext:isMemberOf> <info:fedora/' . $pid . '>)
             and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
             minus $content <mulgara:is> <info:fedora/fedora-system:FedoraObject-3.0>
             order by $title';
  }

  /**
   * Gets objects related to this object.  Must include offset and limit!
   *
   * Calls self::getRelatedItems() but requires limit and offset.
   *
   * @param $pid string
   *   A string containing a Fedora PID.
   * @param $limit
   *   An integer
   * @param type $offset
   * @param type $itqlquery
   * @return type 
   */
  function getRelatedObjects($pid, $limit, $offset, $itqlquery=NULL) {
    return $this->getRelatedItems($pid, $itqlquery, $limit, $offset);
  }

  /**
   * Gets objects related to this item.
   *
   * Query the resource index using the provided iTQL query.  If no query is 
   * provided, one should be obtained via self::getCollectionQuery() which
   * grabs the child objects.
   *
   * @param $pid string
   *   A string containing a PID which may be substituted into the query,
   *   in place of the %parent_collection% placeholder.
   * @param $query_string string
   *   An optional iTQL query.
   * @param $limit int
   *   An optional integer to limit the number of results returned.
   * @param int $offset
   *   An optional integer used to offset the results returned. (Query should
   *   involve a sort to maintain consistency.
   * @return string
   *   Sparql XML results from the resource index.
   */
  function getRelatedItems($pid, $query_string = NULL, $limit = NULL, $offset = NULL) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

    if (!fedora_repository_access(OBJECTHELPER :: $OBJECT_HELPER_VIEW_FEDORA, $pid)) {
      drupal_set_message(t("You do not have access to Fedora objects within the attempted namespace or access to Fedora denied."), 'error');
      return ' ';
    }

    if ($query_string === NULL) {
      $query_string = self::getCollectionQuery($pid);
    }

    // Replace %parent_collection% with the actual collection PID
    $query_string = preg_replace("/\%parent_collection\%/", "<info:fedora/$pid>", $query_string);

    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    
    $settings = array(
      'type' => 'tuples',
      'flush' => TRUE,
      'format' => 'Sparql',
      'lang' => 'itql',
      'stream' => 'on',
      'query' => $query_string
    );

    if ($limit > 0) {
      $settings['limit'] = $limit;
    }
    if ($offset > 0) {
      $settings['offset'] = $offset;
    }
    
    $url .= '?' . http_build_query($settings, NULL, '&');
    $content = do_curl($url);
    return $content;
  }

  /**
   * get Collection Policy Stream ?
   * @param type $collection_pid
   * @return type 
   */
  function getCollectionPolicyStream($collection_pid) {
    if ($this->collectionPolicyStream != NULL) {
      return $this->collectionPolicyStream;
    }
    $this->collectionPolicyStream = $this->getStream($collection_pid, CollectionClass :: $COLLECTION_CLASS_COLLECTION_POLICY_STREAM, 0);
    return $this->collectionPolicyStream;
  }

  /**
   * get Relationship element ?
   * @param type $collection_pid
   * @return type 
   */
  function getRelationshipElement($collection_pid) {
    $stream = $this->getCollectionPolicyStream($collection_pid);
    try {
      $xml = new SimpleXMLElement($stream);
    } catch (Exception $e) {
      drupal_set_message(t('Error getting relationship element from policy stream @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return;
    }
    $relationship = $xml->relationship[0];
    return $relationship;
  }

  /**
   * get Collection View Stream ?
   * @param type $collection_pid
   * @return type 
   */
  function getCollectionViewStream($collection_pid) {
    $this->collectionViewStream = $this->getStream($collection_pid, CollectionClass :: $COLLECTION_CLASS_COLLECTION_VIEW_STREAM, 0);
    return $this->collectionViewStream;
  }

  /**
   * get Stream ?
   * @param type $pid
   * @param type $dsid
   * @param type $showError
   * @return type 
   */
  function getStream($pid, $dsid, $showError = 1) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new fedora_item($pid);
    return isset($item->datastreams[$dsid]) ? $item->get_datastream_dissemination($dsid) : NULL;
  }

  /**
   * get Pid name space ?
   * @param type $pid
   * @param type $dsid
   * @return type 
   */
  function getPidNameSpace($pid, $dsid) {
    $stream = $this->getCollectionPolicyStream($pid);
    try {
      $xml = new SimpleXMLElement($stream);
    } catch (Exception $e) {
      drupal_set_message(t('Error getting PID namespace @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return;
    }
    foreach ($xml->contentmodels->contentmodel as $contentModel) {
      // $contentModelName=strip_tags($contentModel['name']);
      $contentdsid = strip_tags($contentModel->dsid);
      if (strtolower($contentdsid) == strtolower($dsid)) {
        return strip_tags($contentModel->pid_namespace->asXML());
      }
    }
    drupal_set_message(t('Error getting PID namespace! using collection pid of !pid and content model of !dsid', array('!pid' => $pid, '!dsid' => $dsid)), 'error');
    return NULL;
  }

  /**
   * gets a list of content models from a collection policy
   * @param type $collection_pid
   * @param type $showError
   * @return ContentModel 
   */
  function getContentModels($collection_pid, $showError = TRUE) {
    module_load_include('inc', 'Fedora_Repository', 'ContentModel');
    $collection_stream = $this->getCollectionPolicyStream($collection_pid);
    try {
      $xml = new SimpleXMLElement($collection_stream);
    } catch (Exception $e) {
      if ($showError) {
        drupal_set_message(t('@e', array('@e' => check_plain($e->getMessage()))), 'error');
      }
      return NULL;
    }
    foreach ($xml->contentmodels->contentmodel as $content_model) {
      $contentModel = new ContentModel();
      $contentModel->contentModelDsid = $content_model->dsid;
      $contentModel->contentModelPid = $content_model->pid;
      $contentModel->pidNamespace = $content_model->pidNamespace;
      $contentModel->contentModelName = $content_model['name'];
      $models[] = $contentModel;
    }
    return $models;
  }

  /**
   * using the collection policies pid namespace get a new pid by calling fedora' get next pid and appending it to the namespace
   * $pid is the $pid of the content model
   * $dsid is the datastream id of the content model.
   */
  function getNextPid($pid, $dsid) {
    module_load_include('inc', 'Fedora_Repository', 'ConnectionHelper');
    $pidNameSpace = $this->getPidNameSpace($pid, $dsid);
    $pname = substr($pidNameSpace, 0, strpos($pidNameSpace, ":"));
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    return Fedora_Item::get_next_pid_in_namespace($pname);
  }

  /**
   * gets the form handler file, class and method and returns them in an array
   *
   * @param string $pid The content model PID
   * @param string $dsid The content model DSID
   * @return array The file, class and method name to handle the ingest form.
   */
  function getFormHandler($pid, $dsid) {
    $stream = $this->getContentModelStream($pid, $dsid);
    try {
      $xml = new SimpleXMLElement($stream);
    } catch (Exception $e) {
      drupal_set_message(t('Error Getting FormHandler: @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return NULL;
    }
    $formHandler = $xml->ingest_form;
    if ($formHandler != NULL) {
      $handlerDsid = strip_tags($formHandler['dsid']);
      $handlerMethod = strip_tags($formHandler->form_builder_method->form_handler->asXML());
      $handlerFile = strip_tags($formHandler->form_builder_method->file->asXML());
      $handlerClass = strip_tags($formHandler->form_builder_method->class_name->asXML());
      $handlerModule = strip_tags($formHandler->form_builder_method->module->asXML());
      $returnArray = array();
      $returnArray['module'] = $handlerModule;
      $returnArray['method'] = $handlerMethod;
      $returnArray['class'] = $handlerClass;
      $returnArray['file'] = $handlerFile;
      return $returnArray;
    }

    drupal_set_message(t("Error getting form handler. No handler found for !pid - !dsid", array('!pid' => $pid, '!dsid' => $dsid)), 'error');
    return NULL;
  }

  /**
   * get Allowed Mime Types
   * @param type $contentModelPid
   * @param type $contentModel_dsid
   * @return type 
   */
  function getAllowedMimeTypes($contentModelPid, $contentModel_dsid) {
    $stream = $this->getContentModelStream($contentModelPid, $contentModel_dsid);
    try {
      $xml = new SimpleXMLElement($stream);
    } catch (Exception $e) {
      drupal_set_message(t('Error getting content model stream for mime types @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return;
    }
    foreach ($xml->mimetypes->type as $type) {
      $types[] = $type;
    }
    return $types;
  }

  /**
   * Grabs the rules from the content model stream
   * file the file that has been uploaded
   *
   * @param type $file
   * @param type $mimetype
   * @param type $pid
   * @param type $dsid
   * @return type 
   */
  function getAndDoRules($file, $mimetype, $pid, $dsid) {
    if (!user_access('ingest new fedora objects')) {
      drupal_set_message(t('You do not have permission to ingest objects.'));
      return FALSE;
    }

    $stream = $this->getContentModelStream($pid, $dsid);

    try {
      $xml = new SimpleXMLElement($stream);
    } catch (Exception $e) {
      drupal_set_message(t('Error getting content model stream @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return FALSE;
    }
    foreach ($xml->ingest_rules->rule as $rule) {
      foreach ($rule->applies_to as $type) {
        if (!strcmp(trim($type), trim($mimetype))) {
          $methods = $rule->methods->method;
          return $this->callMethods($file, $methods);
        }
      }
    }
  }

  /**
   * calls the methods defined in the content model rules .xml file stored in a Fedora object
   *
   * @param type $file
   * @param type $methods
   * @return type 
   */
  function callMethods($file, $methods) {
    foreach ($methods as $method) {
      $phpFile = strip_tags($method->file->asXML());
      $phpClass = strip_tags($method->class_name->asXML());
      $phpMethod = strip_tags($method->method_name->asXML());
      $file_ext = strip_tags($method->modified_files_ext->asXML());
      $parameters = $method->parameters->parameter;
      $dsid = strip_tags($method->datastream_id);
      $parametersArray = array();
      if (isset($parameters)) {
        foreach ($parameters as $parameter) {
          $name = strip_tags($parameter['name']);
          $value = strip_tags($parameter->asXML());
          $parametersArray["$name"] = $value;
        }
      }
      // module_load_include( $phpFile, 'Fedora_Repository', ' ');
      require_once(drupal_get_path('module', 'fedora_repository') . '/' . $phpFile);
      $thisClass = new $phpClass ();
      $returnValue = $thisClass->$phpMethod($parametersArray, $dsid, $file, $file_ext);
      if (!$returnValue) {
        drupal_set_message(t('Error! Failed running content model method !m !rv', array('!m' => $phpMethod, '!rv' => $returnValue)));
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * grabs a xml form definition from a content model and builds
   * the form using drupals forms api
   *
   * @param type $form
   * @param type $form_state
   * @param type $contentModelPid
   * @param type $contentModelDsid
   * @return type 
   */
  function build_ingest_form(&$form, &$form_state, $contentModelPid, $contentModelDsid) {
    $stream = $this->getContentModelStream($contentModelPid, $contentModelDsid);
    try {
      $xml = new SimpleXMLElement($stream);
    } catch (Exception $e) {
      drupal_set_message(t('Error getting ingest form stream @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return FALSE;
    }
    $docRoot = $_SERVER['DOCUMENT_ROOT'];

    $file = (isset($form_state['values']['ingest-file-location']) ? $form_state['values']['ingest-file-location'] : '');
    // $fullPath = $docRoot . '' . $file;
    $fullpath = $file;
    //	$form = array();
    $form['step'] = array(
      '#type' => 'hidden',
      '#value' => (isset($form_state['values']['step']) ? $form_state['values']['step'] : 0) + 1,
    );
    $form['ingest-file-location'] = array(
      '#type' => 'hidden',
      '#value' => $file,
    );

    $form['content_model_name'] = array(
      '#type' => 'hidden',
      '#value' => $contentModelDsid
    );
    $form['models'] = array(//content models available
      '#type' => 'hidden',
      '#value' => $form_state['values']['models'],
    );
    $form['fullpath'] = array(
      '#type' => 'hidden',
      '#value' => $fullpath,
    );

    $form['#attributes']['enctype'] = 'multipart/form-data';
    $form['indicator']['ingest-file-location'] = array(
      '#type' => 'file',
      '#title' => t('Upload Document'),
      '#size' => 48,
      '#description' => t('Full text'),
    );
    if ($xml->ingest_form->hide_file_chooser == 'TRUE') {
      $form['indicator']['ingest-file-location']['#type'] = 'hidden';
    }
    $ingest_form = $xml->ingest_form; //should only be one
    $drupal_module = strip_tags($ingest_form->form_builder_method->module->asXML());
    if (empty($drupal_module)) {
      $drupal_module = 'fedora_repository';
    }
    $phpFile = strip_tags($ingest_form->form_builder_method->file->asXML());
    $phpClass = strip_tags($ingest_form->form_builder_method->class_name->asXML());
    $phpMethod = strip_tags($ingest_form->form_builder_method->method_name->asXML());
    $dsid = strip_tags($ingest_form['dsid']);
    // module_load_include('php', 'Fedora_Repository', $phpFile);
    require_once(drupal_get_path('module', $drupal_module) . '/' . $phpFile);
    $thisClass = new $phpClass ();

    return $thisClass->$phpMethod($form, $ingest_form, $form_state['values'], $form_state);
  }

  /**
   * this will also create a personal collection for an existing user if they don't have one
   * not using this function currently
   * 
   * @param type $user
   * @return type 
   */
  function createUserCollection(& $user) {
    if (isset($user->fedora_personal_pid)) {
      return;
    }
    $username = array(
      'name' => variable_get('fedora_admin_user', 'fedoraAdmin')
    );
    $admin_user = user_load($username);
    module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
    $connectionHelper = new ConnectionHelper();
    try {
      $soapClient = $connectionHelper->getSoapClient(variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/wsdl?api=API-M'));
      $pidNameSpace = variable_get('fedora_repository_pid', 'vre:');
      $pidNameSpace = substr($pidNameSpace, 0, strpos($pidNameSpace, ":"));
      $params = array(
        'numPIDs' => '',
        'pidNamespace' => $pidNameSpace
      );
      $object = $soapClient->__soapCall('getNextPID', array(
        $params
          ));
    } catch (exception $e) {
      drupal_set_message(t('Error getting Next PID: @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return FALSE;
    }
    $pid = implode(get_object_vars($object));
    $pidNumber = strstr($pid, ":");
    $pid = $pidNameSpace . ':' . 'USER-' . $user->name . '-' . substr($pidNumber, 1);
    $personal_collection_pid = array(
      'fedora_personal_pid' => $pid
    );
    module_load_include('inc', 'fedora_repository', 'plugins/PersonalCollectionClass');
    $personalCollectionClass = new PersonalCollectionClass();
    if (!$personalCollectionClass->createCollection($user, $pid, $soapClient)) {
      drupal_set_message(t("Did not create a personal collection object for !u", array('!u' => $user->name)));
      return FALSE; //creation failed don't save the collection pid in drupal db
    }
    user_save($user, $personal_collection_pid);
    return TRUE;
  }

  /**
   * Queries a collection object for an xslt to format how the
   * collection of objects is displayed.
   *
   * @param type $pid
   * @param type $path
   * @param type $canUseDefault
   * @return type 
   */
  function getXslContent($pid, $path, $canUseDefault = TRUE) {
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    $collectionClass = new CollectionClass();
    $xslContent = $collectionClass->getCollectionViewStream($pid);
    
    //If there's no XSLT from the object, then check if the one which used to exist, does...
    if (!$xslContent && $canUseDefault && file_exists($path . '/xsl/sparql_to_html.xsl')) { 
      $xslContent = file_get_contents($path . '/xsl/sparql_to_html.xsl');
    }
    
    return $xslContent;
  }

  /**
   * show field sets ?
   * @global type $base_url
   * @global type $user
   * @param type $page_number
   * @return string 
   */
  function showFieldSets($page_number) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
    //module_load_include('inc', 'fedora_repository', 'BatchIngest'); //Legacy code?
    global $base_url;
    global $user;
    
    $tabset = array();
    $query = NULL;
    $item = new Fedora_Item($this->pid);
    if ($item->exists() && array_key_exists('QUERY', $item->datastreams)) {
      $query = $item->get_datastream_dissemination('QUERY');
    }
    $results = $this->getRelatedItems($this->pid, $query);

    $collection_items = $this->renderCollection($results, $this->pid, NULL, NULL, $page_number);

    $show_batch_tab = FALSE;
    $policy = CollectionPolicy::loadFromCollection($this->pid, TRUE);

    if (!empty($policy)) {
      $content_models = $policy->getContentModels();
    }
    if (count($content_models) == 1 && $content_models[0]->pid == "islandora:collectionCModel") {
      $show_batch_tab = FALSE;
    }
    
    // Check the form post to see if we are in the middle of an ingest operation.
    $add_selected = ((!empty($_POST['form_id']) && $_POST['form_id'] == 'fedora_repository_ingest_form') || !$collection_items);
    $view_selected = !$add_selected;

    $tabset['view_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('View'),
      '#selected' => $view_selected,
      '#content' => $collection_items,
      '#tab_name' => 'view-tab',
    );
    
    $add_to_collection = $this->getIngestInterface();
    if (!empty($add_to_collection)) {
      $tabset['add_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Add'),
        '#selected' => $add_selected,
        // This will be the content of the tab.
        '#content' => $add_to_collection,
        '#tab_name' => 'add-tab',
      );
    }

    return $tabset;
  }

  /**
   * get Ingest Interface ??
   * @global type $base_url
   * @return string 
   */
  function getIngestInterface() {
    module_load_include('inc', 'Fedora_Repository', 'CollectionPolicy');
    $collectionPolicyExists = $this->collectionObject->getMimeType($this->pid, CollectionPolicy::getDefaultDSID());
    if (user_access(ObjectHelper :: $INGEST_FEDORA_OBJECTS) && $collectionPolicyExists) {
      if (!empty($collectionPolicyExists)) {
        $allow = TRUE;
        if (module_exists('fedora_fesl')) {
          $allow = fedora_fesl_check_roles($this->pid, 'write');
        }
        if ($allow) {
          $ingestObject = drupal_get_form('fedora_repository_ingest_form', $this->pid);
        }
      }
    }
    else {
      $ingestObject = '';
    }

    return $ingestObject;
  }

  /**
   * Unfortunate function, I know...
   *
   * Does just what it says:  Hacks the default Drupal pager such that it might
   * be rendered, likely with: theme('pager', array(), $per_page, $pager_name)
   * (I reccomend seeing the real documentation for more detail, but the first
   * array can be a list of the tags to use for first, previous, next and last
   * (text in the pager), I don't believe per_page is actually used in the theme
   * function, and $pager_name is an integer used to identify the pager (such 
   * that there can be more than one--that is, tracking different lists of 
   * content on a single page.  You can render the exact same pager multiple 
   * times, say if you want one at the top and bottom of a list, using the same 
   * ID/pager_name.
   *
   * @global $pager_total array
   *   Numerically indexed array, where keys are the $pager_names and values
   *   are the number of pages in the given set, based on: ceil($total_items/$per_page);
   * @global $pager_page_array array
   *   Numerically indexed array, where keys are the $pager_names and values
   *   are the page selected in the relevant set.
   * @param $pager_name int
   *   An integer to identify the pager to affect.  Do note that paging in using
   *   this function will add the 'page' HTTP GET parameter to the URL, with 
   *   the value containing a comma-separated list with max($pager_name + 1)
   *   values--that is, if you create a single pager named '10', the 'next'
   *   link will look something like: 0,0,0,0,0,0,0,0,0,0,1
   * @param $per_page int
   *   An integer representing the number of items per page.
   * @param $total_items int
   *   An integer representing the total number of items in the set.
   * @return int
   *   An integer representing what the current page should be.
   */
  protected static function hackPager($pager_name, $per_page = NULL, $total_items = NULL) {
    global $pager_total, $pager_page_array;
    
    if ($per_page !== NULL && $total_items !== NULL) {
      $pager_total[$pager_name] = ceil($total_items / $per_page);
    }
    
    //XXX: Don't know that this is neccessary, to try to load all the time, or
    //  whether Drupal will load it automatically somewhere...  Docs seems a
    //  a little sparse.
    $page_info = explode(',', isset($_GET['page']) ? $_GET['page'] : '');
    $page = $page_info[$pager_name];
    if ($page < 0) {
      $page = 0;
    }
    
    if (!isset($pager_page_array)) {
      $pager_page_array = pager_load_array($page, $pager_name, $page_info);
    }
    else {
      $pager_page_array = pager_load_array($page, $pager_name, $pager_page_array);
    }
    
    $page = $pager_page_array[$pager_name];
    return $page;
  }
  
  /**
   * render collection
   * @global type $base_url
   * @param type $content
   * @param type $pid
   * @param type $dsId
   * @param type $collection
   * @param int $pageNumber
   * @return type 
   */
  function renderCollection($content, $pid, $dsId, $collectionName, $pageNumber = NULL) {
    $path = drupal_get_path('module', 'fedora_repository');
    global $base_url;
    $collection_pid = $pid; //we will be changing the pid later maybe
    $parsedContent = NULL;
    if(!isset($this->collectionObject)){
      $this->collectionObject = new ObjectHelper($pid);
    } 
    $contentModels = $this->collectionObject->get_content_models_list($pid);
    $isCollection = FALSE;
    //if this is a collection object store the $pid in the session as it will come in handy
    //after a purge or ingest to return to the correct collection.

    $fedoraItem = NULL;

    if (empty($collectionName)) {
      $collectionName = menu_get_active_title();
    }
    
    $xslContent = $this->getXslContent($pid, $path);

    $objectList = '';
    if (isset($content) && $content != FALSE) {
      if (!$xslContent) { //Didn't find an XSLT.
        $intermediate_results = ObjectHelper::parse_sparql_results($content);
        unset($content);
        
        $per_page = 20; //XXX:  Make this configurable.
        $pager_name = 0;
        $total = count($intermediate_results);
        $pager_page = self::hackPager($pager_name, $per_page, $total);

        $results = array();
        foreach (array_slice($intermediate_results, $per_page * $pager_page, $per_page) as $result) {
          $title = $result['title'];
          $obj_path = "fedora/repository/{$result['object']}";
          $thumbnail = theme('image', "$obj_path/TN", $title, $title, array(), FALSE);
          $results[] = array(
            'data' => l($thumbnail, $obj_path, array(
              'html' => TRUE,
              'attributes' => array(
                'class' => 'results-image',
              ),
            )) . l($title, $obj_path, array('attributes' => array('class' => 'results-text'))),
          );
        }
        if (!$results) {
          drupal_set_message(t("No objects in this collection (or bad query)."));
        }
        else {
          $first = $per_page * $pager_page;
          $last = (($total - $first) > $per_page)?
            ($first + $per_page):
            $total;
          $results_range_text = t('Results @first to @last of @total', array(
            '@first' => $first + 1,
            '@last' => $last,
            '@total' => $total,
          ));
          //$objectList = '<h3>' . $results_range_text . '</h3>'; 
          $objectList .= theme('pager', array(), $per_page, $pager_name);
          $objectList .= theme('item_list', $results, $result_range_text, 'ul', array(
            'class' => 'islandora-collection-results-list',
          ));
          $objectList .= theme('pager', array(), $per_page, $pager_name);
        }
      }
      else {
        if (!$pageNumber) {
          $pageNumber = 1;
        }
        
        //get collection list and display using xslt-------------------------------------------
        $input = new DomDocument();
        $input->loadXML(trim($content));
        $results = $input->getElementsByTagName('result');
        if ($results->length > 0) {
          try {
            $proc = new XsltProcessor();
            $options = array( //Could make this the return of a hook?  
              'collectionPid' => $collection_pid,
              'collectionTitle' => $collectionName,
              'baseUrl' => $base_url,
              'path' => "$base_url/$path",
              'hitPage' => $pageNumber,
            );
            
            $proc->setParameter('', $options);

            $proc->registerPHPFunctions();
            $xsl = new DomDocument();
            $xsl->loadXML($xslContent);
            // php xsl does not seem to work with namespaces so removing it below
            // I may have just been being stupid here
            // $content = str_ireplace('xmlns="http://www.w3.org/2001/sw/DataAccess/rf1/result"', '', $content);

            $xsl = $proc->importStylesheet($xsl);
            $newdom = $proc->transformToDoc($input);

            $objectList = $newdom->saveHTML(); //is the xml transformed to html as defined in the xslt associated with the collection object

            if (!$objectList) {
              throw new Exception("Invalid XML.");
            }
          } catch (Exception $e) {
            drupal_set_message(check_plain($e->getMessage()), 'error');
            return '';
          }
        }
      }
    }
    else {
      drupal_set_message(t("No objects in this collection (or bad query)."));
    }
    return $objectList;
  }

}

