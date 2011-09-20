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

  /**
   * gets objects related to this object.  must include offset and limit
   * calls getRelatedItems but enforces limit and offset
   * @param type $pid
   * @param type $limit
   * @param type $offset
   * @param type $itqlquery
   * @return type 
   */
  function getRelatedObjects($pid, $limit, $offset, $itqlquery=NULL) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $objectHelper = new ObjectHelper();
    if (!isset($itqlquery)) {
      // $query_string = $objectHelper->getStream($pid, 'QUERY', 0);
      $itqlquery = $objectHelper->getStream($pid, 'QUERY', 0);
    }
    return $this->getRelatedItems($pid, $itqlquery, $limit, $offset);
  }

  /**
   * Gets objects related to this item.  It will query the object for a Query stream and use that as a itql query
   * or if there is no query stream it will use the default.  If you pass a query to this method it will use the passed in query no matter what
   * @global type $user
   * @param type $pid
   * @param type $itqlquery
   * @param int $limit
   * @param int $offset
   * @return type 
   */
  function getRelatedItems($pid, $itqlquery = NULL, $limit = NULL, $offset = NULL) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    if (!isset($limit)) {
      $limit = 1000;
    }
    if (!isset($offset)) {
      $offset = 0;
    }
    global $user;
    if (!fedora_repository_access(OBJECTHELPER :: $OBJECT_HELPER_VIEW_FEDORA, $pid, $user)) {
      drupal_set_message(t("You do not have access to Fedora objects within the attempted namespace or access to Fedora denied."), 'error');
      return ' ';
    }
    $objectHelper = new ObjectHelper();
    $query_string = $itqlquery;
    if (!isset($query_string)) {
      $query_string = $objectHelper->getStream($pid, 'QUERY', 0);
      if ($query_string == NULL) {
        $query_string = 'select $object $title $content from <#ri>
                             where ($object <dc:title> $title
                             and $object <fedora-model:hasModel> $content
                             and ($object <fedora-rels-ext:isMemberOfCollection> <info:fedora/' . $pid . '>
                             or $object <fedora-rels-ext:isMemberOf> <info:fedora/' . $pid . '>)
                             and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
                             minus $content <mulgara:is> <info:fedora/fedora-system:FedoraObject-3.0>
                             order by $title';
      }
    }
    else {
      // Replace %parent_collection% with the actual collection PID
      $query_string = preg_replace("/\%parent_collection\%/", "<info:fedora/$pid>", $query_string);
    }

    $query_string = htmlentities(urlencode($query_string));

    $content = '';
    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=$limit&offset=$offset&lang=itql&stream=on&query=" . $query_string;
    $content .= do_curl($url);

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
      drupal_set_message(t('Error getting relationship element from policy stream !e', array('!e' => $e->getMessage())), 'error');
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
      drupal_set_message(t('Error getting PID namespace !e', array('!e' => $e->getMessage())), 'error');
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
        drupal_set_message(t("!e", array('!e' => $e->getMessage())), 'error');
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
      drupal_set_message(t('Error Getting FormHandler: !e', array('!e' => $e->getMessage())), 'error');
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
      drupal_set_message(t('Error getting content model stream for mime types !e', array('!e' => $e->getMessage())), 'error');
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
      drupal_set_message(t('Error getting content model stream !e', array('!e' => $e->getMessage())), 'error');
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
        drupal_set_message('Error! Failed running content model method !m !rv', array('!m' => $phpMethod, '!rv' => $returnValue));
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
      drupal_set_message(t('Error getting ingest form stream !e', array('!e' => $e->getMessage())), 'error');
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
      $soapClient = $connectionHelper->getSoapClient(variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/services/management?wsdl'));
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
      drupal_set_message(t('Error getting Next PID: !e', array('!e' => $e->getMessage())), 'error');
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
      drupal_set_message("Did not create a personal collection object for !u", array('!u' => $user->name));
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
    if (!$xslContent && $canUseDefault) { //no xslt so we will use the default sent with the module
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
    global $base_url;
    $tabset = array();
    global $user;
    $objectHelper = new ObjectHelper();
    $item = new Fedora_Item($this->pid);
    $query = NULL;
    if ($item->exists() && array_key_exists('QUERY', $item->datastreams)) {
      $query = $item->get_datastream_dissemination('QUERY');
    }
    $results = $this->getRelatedItems($this->pid, $query);

    $collection_items = $this->renderCollection($results, $this->pid, NULL, NULL, $page_number);
    $collection_item = new Fedora_Item($this->pid);
    // Check the form post to see if we are in the middle of an ingest operation.
    $show_ingest_tab = (!empty($_POST['form_id']) && $_POST['form_id'] == 'fedora_repository_ingest_form');
    $add_to_collection = $this->getIngestInterface();
    if (!$show_ingest_tab) {
      $view_selected = true;
    }
    drupal_set_message();


    $tabset['view_tab'] = array(
      '#type' => 'tabpage',
      '#title' => 'View',
      '#selected' => $view_selected,
      '#content' => $collection_items
    );
    $tabset['add_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('Add'),
      '#selected' => $show_ingest_tab,
      // This will be the content of the tab.
      '#content' => $add_to_collection,
    );

    $tabset['add_collection_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('Add Child Collection'),
      // This will be the content of the tab.
      '#content' => drupal_get_form('collection_creation_form', $this->pid),
    );
    return $tabset;
  }

  /**
   * get Ingest Interface ??
   * @global type $base_url
   * @return string 
   */
  function getIngestInterface() {
    global $base_url;
    $objectHelper = new ObjectHelper();
    module_load_include('inc', 'Fedora_Repository', 'CollectionPolicy');
    $collectionPolicyExists = $objectHelper->getMimeType($this->pid, CollectionPolicy::getDefaultDSID());
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
   * render collection
   * @global type $base_url
   * @param type $content
   * @param type $pid
   * @param type $dsId
   * @param type $collection
   * @param int $pageNumber
   * @return type 
   */
  function renderCollection($content, $pid, $dsId, $collection, $pageNumber = NULL) {
    $path = drupal_get_path('module', 'fedora_repository');
    global $base_url;
    $collection_pid = $pid; //we will be changing the pid later maybe
    $objectHelper = new ObjectHelper();
    $parsedContent = NULL;
    $contentModels = $objectHelper->get_content_models_list($pid);
    $isCollection = FALSE;
    //if this is a collection object store the $pid in the session as it will come in handy
    //after a purge or ingest to return to the correct collection.

    $fedoraItem = NULL;



    $collectionName = $collection;
    if (!$pageNumber) {
      $pageNumber = 1;
    }

    if (!isset($collectionName)) {
      $collectionName = variable_get('fedora_repository_name', 'Collection');
    }
    $xslContent = $this->getXslContent($pid, $path);

    //get collection list and display using xslt-------------------------------------------
    $objectList = '';
    if (isset($content) && $content != FALSE) {
      $input = new DomDocument();
      $input->loadXML(trim($content));
      $results = $input->getElementsByTagName('result');
      if ($results->length > 0) {
        try {
          $proc = new XsltProcessor();
          $proc->setParameter('', 'collectionPid', $collection_pid);
          $proc->setParameter('', 'collectionTitle', $collectionName);
          $proc->setParameter('', 'baseUrl', $base_url);
          $proc->setParameter('', 'path', $base_url . '/' . $path);
          $proc->setParameter('', 'hitPage', $pageNumber);
          $proc->registerPHPFunctions();
          $xsl = new DomDocument();
          $xsl->loadXML($xslContent);
          // php xsl does not seem to work with namespaces so removing it below
          // I may have just been being stupid here
          // $content = str_ireplace('xmlns="http://www.w3.org/2001/sw/DataAccess/rf1/result"', '', $content);

          $xsl = $proc->importStylesheet($xsl);
          $newdom = $proc->transformToDoc($input);

          $objectList = $newdom->saveXML(); //is the xml transformed to html as defined in the xslt associated with the collection object

          if (!$objectList) {
            throw new Exception("Invalid XML.");
          }
        } catch (Exception $e) {
          drupal_set_message(t('!e', array('!e' => $e->getMessage())), 'error');
          return '';
        }
      }
    }
    else {
      drupal_set_message(t("No Objects in this collection or bad query."));
    }
    return $objectList;
  }

}

/**
 * collection creation form ?
 * @param type $form_state
 * @param type $parent_collection_pid
 * @return type 
 */
function collection_creation_form(&$form_state, $parent_collection_pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  $restricted = FALSE;
  if (variable_get('fedora_namespace_restriction_enforced', true)) {
    $restricted = true;
    $allowed_string = variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora:');
    $namespaces = explode(':', $allowed_string);
    foreach ($namespaces as $namespace) {
      if ($namespace) {
        $allowed[trim($namespace)] = trim($namespace);
      }
    }
  }

  $content_models = get_content_models_as_option_array();
  $form['titlebox'] = array(
    '#type' => 'item',
    '#value' => t("Create New Child Collection within $parent_collection_pid"),
  );

  $form['collection_name'] = array(
    '#title' => "Collection Name",
    '#type' => 'textfield',
    '#description' => t("Human readable name for this collection"),
  );
  $form['collection_pid'] = array(
    '#title' => "Collection PID",
    '#type' => 'textfield',
    '#size' => 15,
    '#description' => t("Unique PID for this collection.  <br />Pids take the general form of namespace:collection eg islandora:pamphlets"),
  );
  if (!$restricted) {
    $form['collection_namespace'] = array(
      '#title' => "Collection Namespace",
      '#type' => 'textfield',
      '#size' => 15,
      '#default_value' => 'default',
      '#description' => t("Namespace for objects in this collection."),
    );
  }
  else {
    $form['collection_namespace'] = array(
      '#title' => "Collection Namespace",
      '#type' => 'select',
      '#options' => $allowed,
      '#default_value' => 'default',
      '#description' => t("Namespace for objects in this collection."),
    );
  }
  $form['parent_collection'] = array(
    '#type' => 'hidden',
    '#value' => $parent_collection_pid,
  );

  $form['content_models'] = array(
    '#title' => "Choose allowable content models for this collection",
    '#type' => 'checkboxes',
    '#options' => $content_models,
    '#required' => true,
    '#description' => t("Content models describe the behaviours of objects with which they are associated."),
  );


  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );


  return($form);
}

/**
 * collection creation form validate ?
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function collection_creation_form_validate($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  $pid = $form_state['values']['collection_pid'];
  $item = new fedora_item($pid);
  if ($item->exists()) {
    form_set_error('', t("$pid already exists within your repository. the PID must be unique"));
    return;
  }
  if (!valid_pid($pid)){
     form_set_error('', t("$pid is not a valid identifier"));
     return;
  }
}

/**
 * collection creation form submit
 * @global type $user
 * @param type $form
 * @param type $form_state 
 */
function collection_creation_form_submit($form, &$form_state) {
  global $user;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/dublin_core');
  $module_path = drupal_get_path('module', 'fedora_repository');
  $thumbnail = drupal_get_path('module', 'Fedora_Repository') . '/images/Crystal_Clear_filesystem_folder_grey.png';
  $collection_pid = $form_state['values']['collection_pid'];
  $new_collection_label = $form_state['values']['collection_name'];
  $parent_collection = $form_state['values']['parent_collection'];
  $pid_namespace = $form_state['values']['collection_namespace'];
  $all_cModels = get_content_models_as_option_array();
  $collection_policy = '<?xml version="1.0" encoding="UTF-8"?>
<collection_policy xmlns="http://www.islandora.ca" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" name="" xsi:schemaLocation="http://www.islandora.ca http://syn.lib.umanitoba.ca/collection_policy.xsd">
    <content_models>
    </content_models>
    <search_terms>
    </search_terms>
    <staging_area></staging_area>
    <relationship>isMemberOfCollection</relationship>
</collection_policy>';
  $content_models = $form_state['values']['content_models'];
  $collection_policy_xml = simplexml_load_string($collection_policy);
  foreach ($content_models as $content_model) {
    if ($content_model) {
      $node = $collection_policy_xml->content_models->addChild('content_model');
      $node->addAttribute('dsid', 'ISLANDORACM');
      $node->addAttribute('name', $all_cModels[$content_model]);
      $node->addAttribute('namespace', $pid_namespace . ':1');
      $node->addAttribute('pid', $content_model);
    }
  }
  $item = fedora_item::ingest_new_item($collection_pid, 'A', $new_collection_label, $user->name);
  $item->add_relationship('isMemberOfCollection', $parent_collection, RELS_EXT_URI);
  $item->add_relationship('hasModel', 'islandora:collectionCModel', FEDORA_MODEL_URI);
  $item->add_datastream_from_string($collection_policy_xml->saveXML(), 'COLLECTION_POLICY');
  $item->add_datastream_from_file($thumbnail, 'TN');
  drupal_goto("/fedora/repository/$collection_pid");
}
