<?php

/**
 * @file 
 * 
 */
/**
 * @file 
 * Content Model Class
 */
module_load_include('inc', 'fedora_repository', 'XMLDatastream');

/**
 * Conent Model ??
 */
class ContentModel extends XMLDatastream {

  static $SCHEMA_URI = 'http://localhost/islandoracm.xsd';
  static $DEFAULT_DSID = 'ISLANDORACM';
  private $mimes = NULL;
  public $pid_namespace;
  public $name;
  private $xpath;

  /**
   * Gets the default DSID to use for ContentModel datastreams.
   *
   * @return string $default_dsid
   */
  static function getDefaultDSID() {
    return variable_get('Islandora_Content_Model_DSID', ContentModel::$DEFAULT_DSID);
  }

  /**
   * Constructs a ContentModel object from a Fedora item
   * by getting the first content model from the hasModel relationship.
   * Returns FALSE on failure.
   * @param string $pid
   * @return ContentModel $cm
   */
  public static function loadFromObject($pid) {
    $ret = FALSE;
    if (self::valid_pid($pid)) {
      module_load_include('inc', 'fedora_repository', 'ObjectHelper');
      $objectHelper = new ObjectHelper();

      $content_models = $objectHelper->get_content_models_list($pid);
      foreach ($content_models as $content_model) {
        if ($content_model != 'fedora-system:FedoraObject-3.0') {
          $ret = $content_model;
          break;
        }
      }
    }
    return $ret;
  }

  /**
   * Ingests a Content Model from a file to the specified pid/dsid . 
   * Returns FALSE on failure.
   *
   * @param string $pid
   * @param string $name
   * @param string $modelDsid
   * @param string $file
   * @return ContentModel $cm
   */
  public static function ingestFromFile($pid, $name, $modelDsid, $file) {
    $ret = FALSE;

    if (($cm = self::loadFromModel($pid, $modelDsid)) === FALSE && file_exists($file)) {
      $cm = new ContentModel(file_get_contents($file), $pid, $modelDsid);
      $rootEl = $cm->xml->getElementsByTagName('content_model')->item(0);
      $rootEl->setAttribute('name', $name);

      module_load_include('inc', 'fedora_repository', 'api/fedora_item');
      $fedoraItem = new Fedora_Item($pid);
      $fedoraItem->add_datastream_from_string($cm->dumpXml(), $modelDsid, $name, 'text/xml', 'X');
      $ret = $cm;
    }

    return $ret;
  }

  /**
   * Ingests a Content Model from an existing model to the specified pid/dsid . 
   * Returns FALSE on failure.
   *
   * @param string $pid
   * @param string $name
   * @param string $modelDsid
   * @param string $copy_model_pid
   * @return ContentModel $cm
   */
  public static function ingestFromModel($pid, $name, $modelDsid, $copy_model_pid) {
    $ret = FALSE;

    if (($cm = self::loadFromModel($pid, $modelDsid)) === FALSE && ($copy_cm = self::loadFromModel($copy_model_pid)) !== FALSE && $copy_cm->validate()) {
      $newDom = $copy_cm->xml;
      $rootEl = $newDom->getElementsByTagName('content_model')->item(0);
      $rootEl->setAttribute('name', $name);

      $cm = new ContentModel($newDom, $pid, $modelDsid);

      module_load_include('inc', 'fedora_repository', 'api/fedora_item');
      $fedoraItem = new Fedora_Item($pid);
      $fedoraItem->add_datastream_from_string($cm->dumpXml(), $modelDsid, $name, 'text/xml', 'X');
      $ret = $cm;
    }

    return $ret;
  }

  /**
   * Ingests a minimum Content Model to the specified pid/dsid. 
   * Returns FALSE on failure.
   *
   * @param string $pid
   * @param string $name
   * @param string $modelDsid
   * @param string $defaultMimetype
   * @param string $ingestFromDsid
   * @param integer $ingestFormPage
   * @param boolean $ingestFormHideFileChooser
   * @param string $ingestFormModule
   * @param string $ingestFormModule
   * @param string $ingestFormFile
   * @param string $ingestFormClass
   * @param string $ingestFormMethod
   * @param string $ingestFormHandler
   *
   * @return ContentModel $cm
   */
  public static function ingestBlankModel($pid, $name, $modelDsid, $defaultMimetype, $ingestFormDsid, $ingestFormPage, $ingestFormHideChooser, $ingestFormModule, $ingestFormModule, $ingestFormFile, $ingestFormClass, $ingestFormMethod, $ingestFormHandler) {
    $ret = FALSE;
    if (($cm = self::loadFromModel($pid, $modelDsid)) === FALSE) {
      $newDom = new DOMDocument('1.0', 'utf-8');
      $newDom->formatOutput = TRUE;
      $rootEl = $newDom->createElement('content_model');
      $rootEl->setAttribute('name', $name);
      $rootEl->setAttribute('xmlns', self::$XMLNS);
      $rootEl->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
      $rootEl->setAttribute('xsi:schemaLocation', self::$XMLNS . ' ' . self::$SCHEMA_URI);

      $mimeEl = $newDom->createElement('mimetypes');
      $typeEl = $newDom->createElement('type', $defaultMimetype);
      $mimeEl->appendChild($typeEl);
      $rootEl->appendChild($mimeEl);

      $ingestRulesEl = $newDom->createElement('ingest_rules');
      $rootEl->appendChild($ingestRulesEl);

      $ingestFormEl = $newDom->createElement('ingest_form');
      $ingestFormEl->setAttribute('dsid', $ingestFormDsid);
      $ingestFormEl->setAttribute('page', $ingestFormPage);
      if ($ingestFormHideChooser == 'TRUE') {
        $ingestFormEl->setAttribute('hide_file_chooser', 'TRUE');
      }

      $builderEl = $newDom->createElement('form_builder_method');
      $builderEl->setAttribute('module', $ingestFormModule);
      $builderEl->setAttribute('file', $ingestFormFile);
      $builderEl->setAttribute('class', $ingestFormClass);
      $builderEl->setAttribute('method', $ingestFormMethod);
      $builderEl->setAttribute('handler', $ingestFormHandler);
      $ingestFormEl->appendChild($builderEl);

      $elementsEl = $newDom->createElement('form_elements');
      $ingestFormEl->appendChild($elementsEl);
      $rootEl->appendChild($ingestFormEl);
      $newDom->appendChild($rootEl);

      $cm = new ContentModel($newDom, $pid, $modelDsid);

      module_load_include('inc', 'fedora_repository', 'api/fedora_item');
      $fedoraItem = new Fedora_Item($pid);
      $fedoraItem->add_datastream_from_string($cm->dumpXml(), $modelDsid, $name, 'text/xml', 'X');
      $ret = $cm;
    }

    return $ret;
  }

  /**
   * Constructs a ContentModel object from the PID of the model in Fedora.
   * If DSID is specified it will use that datastream as the model, otherwise it will
   * use the default (usually ISLANDORACM).  PID_NAMESPACE and name can also be initialized
   * from the collection policy.
   * Returns FALSE on failure.
   *
   * NOTE: $name will be overwritten with the content model name found in the datastream
   * when the model is first validated.\
   *
   * @param string $pid
   * @param string $dsid
   * @param string $pid_namespace
   * @param string $name
   * @return ContentModel $cm
   */
  public static function loadFromModel($pid, $dsid = NULL, $pid_namespace = NULL, $name = NULL) {
    $ret = FALSE;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    try {
      if (self::valid_pid($pid)) {
        $fedoraItem = new Fedora_Item($pid);
        if (!$name) {
          $name = $fedoraItem->objectProfile->objLabel;
        }
        $dsid = ($dsid != NULL && self::valid_dsid($dsid)) ? $dsid : ContentModel::getDefaultDSID();
        $ds = $fedoraItem->get_datastream_dissemination($dsid);
        if (!empty($ds)) {
          $ret = new ContentModel($ds, $pid, $dsid, $pid_namespace, $name);
        }
      }
    } catch (SOAPException $e) {
      $ret = FALSE;
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
   * @param string $pid_namespace
   * @param string $name
   * @return XMLDatastream $cm
   */
  public function __construct($xmlStr, $pid = NULL, $dsid = NULL, $pid_namespace = NULL, $name = NULL) {
    parent::__construct($xmlStr, $pid, $dsid);
    $this->pid_namespace = $pid_namespace;
    $this->name = ($name == NULL) ? 'Islandora Content Model' : $name;
    $this->xpath = new DOMXPath($this->xml);
    $this->xpath->registerNamespace('cm', 'http://www.islandora.ca');
  }

  /**
   * Attempts to convert from the old XML schema to the new by
   * traversing the XML DOM and building a new DOM.  When done
   * $this->xml is replaced by the newly created DOM..
   *
   * @return void
   */
  protected function convertFromOldSchema() {
    $sXml = simplexml_load_string($this->xml->saveXML());
    $newDom = new DOMDocument('1.0', 'utf-8');
    $newDom->formatOutput = TRUE;
    $rootEl = $newDom->createElement('content_model');
    $rootEl->setAttribute('name', $sXml['name']);
    $rootEl->setAttribute('xmlns', self::$XMLNS);
    $rootEl->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
    $rootEl->setAttribute('xsi:schemaLocation', self::$XMLNS . ' ' . self::$SCHEMA_URI);

    $mimeEl = $newDom->createElement('mimetypes');
    foreach ($sXml->mimetypes->type as $mime) {
      $typeEl = $newDom->createElement('type', $mime);
      $mimeEl->appendChild($typeEl);
    }
    $rootEl->appendChild($mimeEl);

    $ingestRulesEl = $newDom->createElement('ingest_rules');
    foreach ($sXml->ingest_rules->rule as $rule) {
      $ruleEl = $newDom->createElement('rule');
      foreach ($rule->applies_to as $appTo) {
        $applToEl = $newDom->createElement('applies_to', trim($appTo));
        $ruleEl->appendChild($applToEl);
      }
      if (isset($rule->methods->method) && count($rule->methods->method) > 0) {
        $methodsEl = $newDom->createElement('ingest_methods');
        foreach ($rule->methods->method as $method) {
          $methodEl = $newDom->createElement('ingest_method');
          $methodEl->setAttribute('module', $method->module);
          $methodEl->setAttribute('file', $method->file);
          $methodEl->setAttribute('class', $method->class_name);
          $methodEl->setAttribute('method', $method->method_name);
          $methodEl->setAttribute('dsid', $method->datastream_id);
          $methodEl->setAttribute('modified_files_ext', $method->modified_files_ext);

          if (isset($method->parameters)) {
            $paramsEl = $newDom->createElement('parameters');
            foreach ($method->parameters->parameter as $param) {
              $paramEl = $newDom->createElement('parameter', $param);
              $paramEl->setAttribute('name', $param['name']);
              $paramsEl->appendChild($paramEl);
            }
            $methodEl->appendChild($paramsEl);
          }


          $methodsEl->appendChild($methodEl);
        }
        $ruleEl->appendChild($methodsEl);
        $ingestRulesEl->appendChild($ruleEl);
      }
    }
    $rootEl->appendChild($ingestRulesEl);

    if (isset($sXml->display_in_fieldset) && count($sXml->display_in_fieldset->datastream) > 0) {
      $datastreamsEl = $newDom->createElement('datastreams');

      foreach ($sXml->display_in_fieldset->datastream as $ds) {
        $dsEl = $newDom->createElement('datastream');
        $dsEl->setAttribute('dsid', $ds['id']);

        if (isset($ds->add_datastream_method)) {
          $add_ds_methodEl = $newDom->createElement('add_datastream_method');
          $add_ds_methodEl->setAttribute('module', $ds->add_datastream_method->module);
          $add_ds_methodEl->setAttribute('file', $ds->add_datastream_method->file);
          $add_ds_methodEl->setAttribute('class', $ds->add_datastream_method->class_name);
          $add_ds_methodEl->setAttribute('method', $ds->add_datastream_method->method_name);
          $add_ds_methodEl->setAttribute('dsid', $ds->add_datastream_method->datastream_id);
          $add_ds_methodEl->setAttribute('modified_files_ext', $ds->add_datastream_method->modified_files_ext);

          if (isset($ds->add_datastream_method->parameters)) {
            $paramsEl = $newDom->createElement('parameters');
            foreach ($ds->add_datastream_method->parameters->parameter as $param) {
              $paramEl = $newDom->createElement('parameter', $param);
              $paramEl->setAttribute('name', $param['name']);
              $paramsEl->appendChild($paramEl);
            }
            $add_ds_methodEl->appendChild($paramsEl);
          }

          $dsEl->appendChild($add_ds_methodEl);
        }

        foreach ($ds->method as $disp_meth) {
          $disp_methEl = $newDom->createElement('display_method');
          $disp_methEl->setAttribute('module', $disp_meth->module);
          $disp_methEl->setAttribute('file', $disp_meth->file);
          $disp_methEl->setAttribute('class', $disp_meth->class_name);
          $disp_methEl->setAttribute('method', $disp_meth->method_name);
          $dsEl->appendChild($disp_methEl);
        }
        $datastreamsEl->appendChild($dsEl);
      }
      $rootEl->appendChild($datastreamsEl);
    }

    $ingest_formEl = $newDom->createElement('ingest_form');
    $ingest_formEl->setAttribute('dsid', $sXml->ingest_form['dsid']);
    $ingest_formEl->setAttribute('page', $sXml->ingest_form['page']);
    if (isset($sXml->ingest_form['hide_file_chooser'])) {
      $ingest_formEl->setAttribute('hide_file_chooser', (strtolower($sXml->ingest_form['hide_file_chooser']) == 'TRUE') ? 'TRUE' : 'FALSE');
    }

    $form_builderEl = $newDom->createElement('form_builder_method');
    $form_builderEl->setAttribute('module', $sXml->ingest_form->form_builder_method->module);
    $form_builderEl->setAttribute('file', $sXml->ingest_form->form_builder_method->file);
    $form_builderEl->setAttribute('class', $sXml->ingest_form->form_builder_method->class_name);
    $form_builderEl->setAttribute('method', $sXml->ingest_form->form_builder_method->method_name);
    $form_builderEl->setAttribute('handler', $sXml->ingest_form->form_builder_method->form_handler);
    $ingest_formEl->appendChild($form_builderEl);

    $form_elementsEl = $newDom->createElement('form_elements');
    foreach ($sXml->ingest_form->form_elements->element as $element) {

      //I found an XML where the label was HTML..  this code will attempt to
      //walk the object setting the label to be the first string it finds.
      if (count(get_object_vars($element->label)) > 0) {
        $obj = $element->label;
        while ($obj != NULL && !is_string($obj)) {
          $keys = get_object_vars($obj);
          $obj = array_shift($keys);
        }
        $element->label = ($obj == NULL) ? '' : $obj;
      }

      $elEl = $newDom->createElement('element');
      $elEl->setAttribute('label', $element->label);
      $elEl->setAttribute('name', $element->name);
      $elEl->setAttribute('type', $element->type);
      if (strtolower($element->required) == 'TRUE') {
        $elEl->setAttribute('required', 'TRUE');
      }
      if (isset($element->description) && trim($element->description) != '') {
        $descEl = $newDom->createElement('description', trim($element->description));
        $elEl->appendChild($descEl);
      }
      if (isset($element->authoritative_list)) {
        $authListEl = $newDom->createElement('authoritative_list');
        foreach ($element->authoritative_list->item as $item) {
          $itemEl = $newDom->createElement('item', trim($item->value));
          if (trim($item->value) != trim($item->field)) {
            $itemEl->setAttribute('field', trim($item->field));
          }
          $authListEl->appendChild($itemEl);
        }
        $elEl->appendChild($authListEl);
      }
      $form_elementsEl->appendChild($elEl);
    }
    $ingest_formEl->appendChild($form_builderEl);
    $ingest_formEl->appendChild($form_elementsEl);
    $rootEl->appendChild($ingest_formEl);

    if (isset($sXml->edit_metadata) &&
        trim($sXml->edit_metadata->build_form_method->module) != '' &&
        trim($sXml->edit_metadata->build_form_method->file) != '' &&
        trim($sXml->edit_metadata->build_form_method->class_name) != '' &&
        trim($sXml->edit_metadata->build_form_method->method_name) != '' &&
        trim($sXml->edit_metadata->submit_form_method->method_name) != '' &&
        trim($sXml->edit_metadata->build_form_method['dsid']) != ''
    ) {
      $edit_metadata_methodEl = $newDom->createElement('edit_metadata_method');
      $edit_metadata_methodEl->setAttribute('module', $sXml->edit_metadata->build_form_method->module);
      $edit_metadata_methodEl->setAttribute('file', $sXml->edit_metadata->build_form_method->file);
      $edit_metadata_methodEl->setAttribute('class', $sXml->edit_metadata->build_form_method->class_name);
      $edit_metadata_methodEl->setAttribute('method', $sXml->edit_metadata->build_form_method->method_name);
      $edit_metadata_methodEl->setAttribute('handler', $sXml->edit_metadata->submit_form_method->method_name);
      $edit_metadata_methodEl->setAttribute('dsid', $sXml->edit_metadata->build_form_method['dsid']);
      $rootEl->appendChild($edit_metadata_methodEl);
    }

    $newDom->appendChild($rootEl);

    $this->xml = DOMDocument::loadXML($newDom->saveXml());
  }

  /**
   * Gets a list of service deployments that this model has.
   *
   * NOTE: Not currently being used.
   *
   * @return String[] $serviceDepPids
   */
  public function getServices() {
    $query = 'select $object $title from <#ri>
      where ($object <fedora-model:label> $title
      and $object <fedora-model:isDeploymentOf> $deploymentOf
      and $object <fedora-model:hasModel> <info:fedora/fedora-system:ServiceDeployment-3.0>
      and $object <fedora-model:isContractorOf> <info:fedora/' . $this->pid . '>
      and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
      order by $title';

    module_load_include('inc', 'fedora_repository', 'CollectionClass');

    $collectionHelper = new CollectionClass();
    $xml = simplexml_load_string($collectionHelper->getRelatedItems($this->pid, $query));

    $results = array();
    foreach ($xml->results->result as $result) {
      $pid = strval(($result->object['uri']));
      $pid = substr($pid, strpos($pid, "/") + 1, strlen($pid));
      $results[] = $pid;
    }

    return $results;
  }

  /**
   * Gets the name of the ContentModel
   * Returns FALSE on failure.
   *
   * @return String $name
   */
  public function getName() {
    $ret = FALSE;
    if ($this->name != NULL) {
      $ret = $this->name;
    }
    elseif ($this->validate()) {
      $rootEl = $this->xml->getElementsByTagName('content_model')->item(0);
      $this->name = $rootEl->getAttribute('name');
      $ret = $this->name;
    }
    return $ret;
  }

  /**
   * Gets the element corresponding to the datastream specified
   * in the <datastreams> element of the schema.
   * Returns FALSE on failure.
   *
   * @param $dsid
   * @return DOMElement $datastream
   */
  private function getDSModel($dsid) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && $this->validate()) {
      $result = $this->xml->getElementsByTagName('datastreams');
      if ($result->length > 0) {
        $result = $result->item(0)->getElementsByTagName('datastream');
        for ($i = 0; $ret == FALSE && $i < $result->length; $i++) {
          if ($result->item($i)->getAttribute('dsid') == $dsid)
            $ret = $result->item($i);
        }
      }
    }
    return $ret;
  }

  /**
   * Gets the DSID of the default datastream
   * from the <datastreams> element of the schema.
   * Returns FALSE on failure.
   *
   * @return String
   */
  public function getDatastreamNameDSID() {
    $datastreams = $this->xml->getElementsByTagName('datastreams');
    $datastream = $datastreams->item(0);
    if ($datastreams->length > 0) {
      $datastream = $datastreams->item(0)->getElementsByTagName('datastream');
      return $datastream->item(0)->getAttribute('dsid');
    }
  }

  /**
   * Gets an array of form elements to use in the ingest form.  The results of this array are passed
   * to the specified ingest form builder.  The form builder can optionally not use the elements as defined
   * in the form builder if more complex forms or behaviour is required.
   * Each element has the following keys: 'label', 'type', 'required', 'description', and if defined, 'authoritative_list' and/or 'parameters'
   *
   * @return string[] $elements 
   */
  public function getIngestFormElements() {
    $ret = FALSE;
    if ($this->validate()) {
      $elements_array = array();
      $form_elements_wrapper = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements');

      if ($form_elements_wrapper->length == 0) {
        return $ret;
      }
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0)->getElementsByTagName('element');
      for ($i = 0; $i < $elements->length; $i++) {
        $desc = $elements->item($i)->getElementsByTagName('description');
        $desc = ($desc->length > 0) ? $desc->item(0)->nodeValue : '';

        $label = $elements->item($i)->getAttribute('label');
        if ($label == NULL) {
          $label = $elements->item($i)->getAttribute('name');
        }

        $element = array('name' => $elements->item($i)->getAttribute('name'),
          'label' => $label,
          'type' => $elements->item($i)->getAttribute('type'),
          'required' => ($elements->item($i)->getAttribute('required') == 'TRUE') ? TRUE : FALSE,
          'description' => $desc
        );

        $auth_list = $elements->item($i)->getElementsByTagName('authoritative_list');
        if ($auth_list->length > 0) {
          $list = array();
          $items = $auth_list->item(0)->getElementsByTagName('item');
          for ($j = 0; $j < $items->length; $j++) {
            $field = $items->item($j)->attributes->getNamedItem('field');
            $list[$items->item($j)->nodeValue] = ($field !== NULL) ? $field->nodeValue : $items->item($j)->nodeValue;
          }
          $element['authoritative_list'] = $list;
        }


        $params = $elements->item($i)->getElementsByTagName('parameters');
        $list = array();
        if ($params->length > 0) {
          $items = $params->item(0)->getElementsByTagName('parameter');
          for ($j = 0; $j < $items->length; $j++) {
            $value = $items->item($j)->nodeValue;
            $list[$items->item($j)->getAttribute('name')] = (strtolower($value) == 'TRUE' ? TRUE : (strtolower($value) == 'FALSE' ? FALSE : $value));
          }
        }
        $element['parameters'] = $list;

        $elements_array[] = $element;
      }
      $ret = $elements_array;
    }

    return $ret;
  }

  /**
   * Decrements an ingest form element in the list of elements.  
   * Updates the "order".  This method is simply an overload to the incIngestFormElement
   * which has a direction parameter.
   *
   * TODO: Might be useful to move multiple places at once, or define 
   * a method to move to an absolute position.  
   *
   * @param String $name 
   * @return boolean $success
   */
  public function decIngestFormElement($name) {
    return $this->incIngestFormElement($name, 'dec');
  }

  /**
   * Increments (or decrements) ingest form element in the list of elements.  
   * Updates the "order".  The $reorder parameter accepts 'inc' or 'dec' to
   * specify the direction to move (defaults to increment.)
   *
   * TODO: Might be useful to move multiple places at once, or define 
   * a method to move to an absolute position.  
   *
   * @param String $name 
   * @param String $reorder
   * @return boolean $success
   */
  public function incIngestFormElement($name, $reorder = 'inc') {
    $ret = FALSE;
    if ($this->validate()) {
      $elementsEl = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0);
      $elements = $elementsEl->getElementsByTagName('element');
      $found = FALSE;
      $ref_el = FALSE;
      for ($i = 0; $found === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == trim($name)) {
          if ($reorder == 'inc') {
            $found = $elements->item($i);
            $ref_el = ($i > 0) ? $elements->item($i - 1) : FALSE;
          }
          else {
            $found = ($i + 1 < $elements->length) ? $elements->item($i + 1) : FALSE;
            $ref_el = $elements->item($i);
          }
        }
      }

      if ($found !== FALSE) {
        $elementsEl->removeChild($found);
        $elementsEl->insertBefore($found, $ref_el);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * Removes an ingest form element from the list of ingest form elements.
   * @param String $name
   * @return boolean $success
   */
  public function removeIngestFormElement($name) {
    $ret = FALSE;
    if ($this->validate()) {
      $elementsEl = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0);
      $elements = $elementsEl->getElementsByTagName('element');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == trim($name)) {
          $found = $elements->item($i);
        }
      }

      if ($found !== FALSE) {
        $elementsEl->removeChild($found);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * Sets a parameter of an ingest form element.  If the value of the element is FALSE the parameter
   * will be removed entirely (if you want to store FALSE as a value, then send the String "FALSE").
   * 
   * @param String $elementName
   * @param String $paramName
   * @param String $paramValue
   * @return boolean success
   */
  public function setIngestFormElementParam($name, $paramName, $paramValue) {
    $ret = FALSE;

    if ($this->validate()) {
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0)->getElementsByTagName('element');
      $element = FALSE;
      for ($i = 0; $element === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == $name) {
          $element = $elements->item($i);
        }
      }

      if ($element !== FALSE) {
        $paramsEl = $element->getElementsByTagName('parameters');
        if ($paramsEl->length == 0) {
          if ($paramValue !== FALSE) {
            $paramsEl = $this->xml->createElement('parameters');
            $element->appendChild($paramsEl);
          }
          else {
            $ret = TRUE;
          }
        }
        else {
          $paramsEl = $paramsEl->item(0);
        }

        if (!$ret) {
          $params = $paramsEl->getElementsByTagName('parameter');
          $found = FALSE;
          for ($i = 0; $found === FALSE && $i < $params->length; $i++) {
            if ($params->item($i)->getAttribute('name') == $paramName) {
              $found = $params->item($i);
            }
          }

          if ($paramValue === FALSE) {
            if ($found !== FALSE) {
              $paramsEl->removeChild($found);
              if ($params->length == 0) {
                $element->removeChild($paramsEl);
              }
            }
            $ret = TRUE;
          }
          else {
            if ($found !== FALSE) {
              $found->nodeValue = $paramValue;
            }
            else {
              $paramEl = $this->xml->createElement('parameter', $paramValue);
              $paramEl->setAttribute('name', $paramName);
              $paramsEl->appendChild($paramEl);
            }
            $ret = TRUE;
          }
        }
      }
    }

    return $ret;
  }

  /**
   * Gets a list of all parameters that belong to the specified ingest form element.
   * 
   * @param String $elementName
   * @return boolean success
   */
  public function getIngestFormElementParams($name) {
    $ret = FALSE;

    if ($this->validate()) {
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0)->getElementsByTagName('element');
      $element = FALSE;
      for ($i = 0; $element === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == $name) {
          $element = $elements->item($i);
        }
      }

      if ($element !== FALSE) {
        $ret = array();
        $paramsEl = $element->getElementsByTagName('parameters');

        if ($paramsEl->length > 0) {
          $params = $paramsEl->item(0)->getElementsByTagName('parameter');
          for ($i = 0; $i < $params->length; $i++) {
            $ret[$params->item($i)->getAttribute('name')] = $params->item($i)->nodeValue;
          }
        }
      }
    }

    return $ret;
  }

  /**
   * Edits the ingest form element specified. 
   * NOTE: The element name can not be changed.  To update an elements name
   * it must be deleted and added with the new name.
   *
   * @param String $name
   * @param String $label
   * @param String $type
   * @param boolean $required
   * @param String description
   * @return boolean success
   */
  public function editIngestFormElement($name, $label, $type, $required, $description) {
    $ret = FALSE;
    if ($this->validate()) {
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0)->getElementsByTagName('element');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == $name) {
          $found = $elements->item($i);
        }
      }

      $found->setAttribute('name', $name);
      $found->setAttribute('type', $type);
      $found->setAttribute('required', $required ? 'TRUE' : 'FALSE');
      if (trim($label) != '' && trim($label) != trim($name)) {
        $found->setAttribute('label', $label);
      }
      elseif ($found->getAttribute('label') != '') {
        $found->removeAttribute('label');
      }

      $descEl = $found->getElementsByTagName('description');
      if (trim($description) != '') {
        if ($descEl->length > 0) {
          $descEl = $descEl->item(0);
          $descEl->nodeValue = $description;
        }
        else {
          $descEl = $this->xml->createElement('description', $description);
          $found->appendChild($descEl);
        }
      }
      elseif ($descEl->length > 0) {
        $found->removeChild($descEl->item(0));
      }

      if ($found->getAttribute('type') != 'select' && $found->getAttribute('type') != 'radio') {
        $authList = $found->getElementsByTagName('authoritative_list');
        if ($authList->length > 0) {
          $found->removeChild($authList->item(0));
        }
      }

      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * Add an ingest form element to the model.
   *
   * @param String $name
   * @param String $label
   * @param String $type
   * @param boolean $required
   * @param String $description
   * @return boolean $success
   */
  public function addIngestFormElement($name, $label, $type, $required, $description = '') {
    $ret = FALSE;
    if ($this->validate()) {
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0);
      $elementEl = $this->xml->createElement('element');
      $elementEl->setAttribute('name', $name);
      $elementEl->setAttribute('type', $type);
      $elementEl->setAttribute('required', $requiredi ? 'TRUE' : 'FALSE');
      if (trim($label) != '' && trim($label) != trim($name)) {
        $elementEl->setAttribute('label', $label);
      }
      if (trim($description) != '') {
        $descEl = $this->xml->createElement('description', $description);
        $elementEl->appendChild($descEl);
      }
      $elements->appendChild($elementEl);

      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * Decrements an authority list item from a form element in the list of elements.  
   * Updates the "order".  This method is simply an overload to the incAuthListItem
   * which has a direction parameter.

   *
   * @param String $elementName 
   * @param String $value
   * @return boolean $success
   */
  public function decAuthListItem($elementName, $value) {
    return $this->incAuthListItem($elementName, $value, 'dec');
  }

  /**
   * Increments (or decrements) an authority list item from a form element in the list of elements.  
   * Updates the "order".  
   *
   * @param String $elementName 
   * @param String $value
   * @param String $direction
   * @return boolean $success
   */
  public function incAuthListItem($elementName, $value, $reorder = 'inc') {
    $ret = FALSE;
    if ($this->validate()) {
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0)->getElementsByTagName('element');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == $elementName) {
          $found = $elements->item($i);
        }
      }

      if ($found !== FALSE) {

        $authListEl = $found->getElementsByTagName('authoritative_list');
        if ($authListEl->length > 0) {
          $authListEl = $authListEl->item(0);

          $items = $authListEl->getElementsByTagName('item');
          $found = FALSE;
          $ref_el = FALSE;
          for ($i = 0; $found === FALSE && $i < $items->length; $i++) {
            if ($items->item($i)->nodeValue == $value) {
              if ($reorder == 'inc') {
                $ref_el = ($i > 0) ? $items->item($i - 1) : FALSE;
                $found = $items->item($i);
              }
              else {
                $ref_el = $items->item($i);
                $found = ($i + 1 < $items->length) ? $items->item($i + 1) : FALSE;
              }
            }
          }

          if ($found !== FALSE && $ref_el !== FALSE) {
            $authListEl->removeChild($found);
            $authListEl->insertBefore($found, $ref_el);
            $ret = TRUE;
          }
        }
      }
    }
    return $ret;
  }

  /**
   * Removes an authority list item from a form element.  
   * @param String $elementName
   * @param String $value
   * @return boolean $success
   */
  public function removeAuthListItem($elementName, $value) {
    $ret = FALSE;
    if ($this->validate()) {
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0)->getElementsByTagName('element');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == $elementName) {
          $found = $elements->item($i);
        }
      }

      if ($found !== FALSE) {
        $authListEl = $found->getElementsByTagName('authoritative_list');
        if ($authListEl->length > 0) {
          $authListEl = $authListEl->item(0);
          $items = $authListEl->getElementsByTagName('item');
          $found = FALSE;
          for ($i = 0; $found === FALSE && $i < $items->length; $i++) {
            if ($items->item($i)->nodeValue == $value) {
              $found = $items->item($i);
            }
          }

          if ($found !== FALSE) {
            if ($items->length == 1) {
              $found->removeChild($authListEl);
            }
            else {
              $authListEl->removeChild($found);
            }

            $ret = TRUE;
          }
        }
      }
    }
    return $ret;
  }

  /**
   * Adds an authority list item to a form element.  
   * @param String $elementName
   * @param String $value
   * @param String $label (optional)
   * @return boolean $success
   */
  public function addAuthListItem($elementName, $value, $label = '') {
    $ret = FALSE;
    if ($this->validate()) {
      $elements = $this->xml->getElementsbyTagName('ingest_form')->item(0)->getElementsByTagName('form_elements')->item(0)->getElementsByTagName('element');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $elements->length; $i++) {
        if ($elements->item($i)->getAttribute('name') == $elementName) {
          $found = $elements->item($i);
        }
      }

      if ($found !== FALSE) {
        $authListEl = $found->getElementsByTagName('authoritative_list');
        if ($authListEl->length == 0) {
          $authListEl = $this->xml->createElement('authoritative_list');
          $found->appendChild($authListEl);
        }
        else {
          $authListEl = $authListEl->item(0);
        }

        $items = $authListEl->getElementsByTagName('item');
        $found = FALSE;
        for ($i = 0; $found == FALSE && $i < $items->length; $i++) {
          if ($items->item($i)->nodeValue == $value) {
            $found = TRUE;
          }
        }

        if (!$found) {
          $itemEl = $this->xml->createElement('item', trim($value));
          if (trim($label) != '' && trim($label) != trim($value)) {
            $itemEl->setAttribute('field', trim($label));
          }
          $authListEl->appendChild($itemEl);
          $ret = TRUE;
        }
      }
    }
    return $ret;
  }

  /**
   * Builds an ingest form using the method specified in  <form_builder_method> element of <ingest_form>
   * Returns FALSE on failure.
   *
   * @param &$form
   * @param &$form_state
   * @return string identifier
   */
  public function buildIngestForm(&$form, &$form_state) {
    $ret = FALSE;
    if ($this->validate()) {
      $docRoot = $_SERVER['DOCUMENT_ROOT'];
      $file = (isset($form_state['values']['ingest-file-location']) ? $form_state['values']['ingest-file-location'] : '');

      $fullpath = $file;

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
        '#value' => 'ISLANDORACM',
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

      $ingest_form = $this->xml->getElementsByTagName('ingest_form')->item(0);

      if (!empty($ingest_form)) {
        if (strtolower($ingest_form->getAttribute('hide_file_chooser')) == 'TRUE') {
          $form['indicator']['ingest-file-location']['#type'] = 'hidden';
        }
      }
      $dsid = $ingest_form->getAttribute('dsid');

      $method = $ingest_form->getElementsByTagName('form_builder_method')->item(0);
      $module = $method->getAttribute('module');
      $path = drupal_get_path('module', !empty($module) ? $module : 'fedora_repository') . '/' . $method->getAttribute('file');
      if (!file_exists($path)) {
        self::$errors[] = 'Build Ingest Form: file \'' . $path . '\' does not exist.';
      }
      else {
        @require_once($path);
        $className = $method->getAttribute('class');
        $methodName = ($method->getAttribute('method'));
        if (!class_exists($className)) {
          self::$errors[] = 'Build Ingest Form: class \'' . $className . ' does not exist.';
        }
        else {
          $class = new $className;
          $elements_array = $this->getIngestFormElements();
          if (method_exists($class, $methodName)) {
            $ret = $class->$methodName($form, $elements_array, $form_state);
          }
          else {
            self::$errors[] = 'Build Ingest Form: method \'' . $className . '->' . $methodName . '\' does not exist.';
          }
        }
      }
    }
    return $ret;
  }

  /**
   * Builds an edit metadata form using the method specified in the <edit_metadata_method> element
   * The DSID specified must match the DSID attribute of <edit_metadata_method>. Returns FALSE on failure.
   *
   * @param string $dsid
   * @return $form
   */
  public function buildEditMetadataForm($pid, $dsid) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && $this->validate()) {
      $method = $this->xml->getElementsByTagName('edit_metadata_method');
      if ($method->length > 0 && $method->item(0)->getAttribute('dsid') == $dsid) {
        $method = $method->item(0);
        $module = $method->getAttribute('module');
        $path = drupal_get_path('module', !empty($module) ? $module : 'fedora_repository') . '/' . $method->getAttribute('file');
        if (!file_exists($path)) {
          self::$errors[] = 'Build Edit Metadata Form: file  \'' . $path . '\' does not exist.';
        }
        else {
          @require_once($path );
          $className = $method->getAttribute('class');
          $methodName = ($method->getAttribute('method'));
          if (!class_exists($className)) {
            self::$errors[] = 'Build Edit Metadata Form: class \'' . $className . '\' does not exist.';
          }
          else {
            $class = new $className($pid);
            if (!method_exists($class, $methodName)) {
              self::$errors[] = 'Build Edit Metadata Form: method \'' . $className . '->' . $methodName . '\' does not exist.';
            }
            else {
              $ret = $class->$methodName();
            }
          }
        }
      }
    }
    return $ret;
  }

  /**
   * Handles the edit metadata form using the handler specified in the <edit_metadata_method> element
   * Returns FALSE on failure.
   *
   * @param &$form_id
   * @param &$form_values
   * @param &$soap_client
   * @return $result
   */
  public function handleEditMetadataForm(&$form_id, &$form_state, &$soap_client) {
    global $base_url;
    $ret = FALSE;
    if ($this->validate()) {
      $method = $this->xml->getElementsByTagName('edit_metadata_method');
      if ($method->length > 0) {
        $method = $method->item(0);
        $module = $method->getAttribute('module');
        $path = drupal_get_path('module', !empty($module) ? $module : 'fedora_repository') . '/' . $method->getAttribute('file');
        if (!file_exists($path)) {
          self::$errors[] = 'Handle Edit Metadata Form: file \'' . $path . '\' does not exist.';
        }
        else {
          @require_once($path);
          $className = $method->getAttribute('class');
          $methodName = ($method->getAttribute('handler'));
          if (!class_exists($className)) {
            self::$errors[] = 'Handle Edit Metadata Form: class \'' . $className . '\' does not exist.';
          }
          else {
            $class = new $className($form_state['values']['pid']);
            if (!method_exists($class, $methodName)) {
              self::$errors[] = 'Handle Edit Metadata Form: metho \'' . $className . '->' . $methodName . '\' does not exist.';
            }
            else {
              $ret = $class->$methodName($form_id, $form_state['values'], $soap_client);
            }
          }
        }
      }
      else {
        // Assume DC form if none is specified.
        module_load_include('inc', 'fedora_repository', 'formClass');
        $metaDataForm = new formClass();
        $ret = $metaDataForm->updateMetaData($form_state['values']['form_id'], $form_state['values'], $soap_client);
        $form_state['storage'] = NULL;
        $form_state['redirect'] = $base_url . '/fedora/repository/' . $form_state['values']['pid'];
      }
    }
    return $ret;
  }

  /**
   * Gets an associative array describing the edit metadata method.
   * Array has the following keys: 'file', 'class', 'method', 'handler', 'dsid'
   * @return String[] $method
   */
  public function getEditMetadataMethod() {
    $ret = FALSE;
    if ($this->validate()) {
      $method = $this->xml->getElementsByTagName('content_model')->item(0)->getElementsByTagName('edit_metadata_method');
      if ($method->length > 0) {
        $method = $method->item(0);
        $ret = array('module' => $method->getAttribute('module') == '' ? 'fedora_repository' : $method->getAttribute('module'),
          'file' => $method->getAttribute('file'),
          'class' => $method->getAttribute('class'),
          'method' => $method->getAttribute('method'),
          'handler' => $method->getAttribute('handler'),
          'dsid' => $method->getAttribute('dsid')
        );
      }
    }
    return $ret;
  }

  /**
   * Removes the edit data method from the Content Model.
   * @return boolean $success
   */
  public function removeEditMetadataMethod() {
    $ret = FALSE;
    if ($this->validate()) {
      $rootEl = $this->xml->getElementsByTagName('content_model')->item(0);
      $method = $rootEl->getElementsByTagName('edit_metadata_method');
      if ($method->length > 0) {
        $rootEl->removeChild($method->item(0));
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * Update the Edit Metadata Method defined in the Content Model
   * @param String $module
   * @param String $file
   * @param String $class
   * @param String $method
   * @param String $handler
   * @param String $dsid
   * @return boolean $success
   */
  public function updateEditMetadataMethod($module, $file, $class, $method, $handler, $dsid) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && $this->validate()) {
      $methodEl = $this->xml->getElementsByTagName('content_model')->item(0)->getElementsByTagName('edit_metadata_method');
      if ($methodEl->length > 0) {
        $methodEl = $methodEl->item(0);
      }
      else {
        $methodEl = $this->xml->createElement('edit_metadata_method');
        $this->xml->getElementsByTagName('content_model')->item(0)->appendChild($methodEl);
      }
      $methodEl->setAttribute('module', $module);
      $methodEl->setAttribute('file', $file);
      $methodEl->setAttribute('class', $class);
      $methodEl->setAttribute('method', $method);
      $methodEl->setAttribute('handler', $handler);
      $methodEl->setAttribute('dsid', $dsid);
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * Executes the add datastream method for the specified datastream on the specified file.
   * Returns FALSE on failure.
   *
   * @param string $dsid
   * @param string $filen
   * @return $result
   */
  public function execAddDatastreamMethods($dsid, $file) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && $this->validate() && ($ds = $this->getDSModel($dsid)) !== FALSE) {
      $addMethod = $ds->getElementsByTagName('add_datastream_method');
      if ($addMethod->length > 0) {
        $addMethod = $addMethod->item(0);
        $paramArray = array();
        $params = $addMethod->getElementsByTagName('parameters');
        if ($params->length > 0) {
          $params = $params->item(0)->getElementsByTagName('parameter');
          for ($i = 0; $i < $params->length; $i++) {
            $paramsArray[$params->item($i)->getAttribute('name')] = $params->item($i)->nodeValue;
          }
        }
        $module = $addMethod->getAttribute('module');
        $path = drupal_get_path('module', !empty($module) ? $module : 'fedora_repository') . '/' . $addMethod->getAttribute('file');
        if (!file_exists($path)) {
          self::$errors[] = 'Execute Add Datastream Methods: file \'' . $path . '\' does not exist.';
        }
        else {
          @require_once($path);
          $className = $addMethod->getAttribute('class');
          $methodName = $addMethod->getAttribute('method');
          if (!class_exists($className)) {
            self::$errors[] = 'Execute Add Datastream Methods: class \'' . $className . '\' does not exist.';
          }
          else {
            $class = new $className;
            if (!method_exists($class, $methodName)) {
              self::$errors[] = 'Execute Add Datastream Methods: method \'' . $className . '->' . $methodName . '\' does not exist.';
            }
            else {
              $ret = $class->$methodName($paramsArray, $addMethod->getAttribute('dsid'), $file, $addMethod->getAttribute('modified_files_ext'));
            }
          }
        }
      }
    }
    return $ret;
  }

  /**
   * Executes the ingest rules that apply to the specified file/mimetype.
   * Returns FALSE on failure. 
   * 
   * If $preview is TRUE, then only execute rules with 
   * a parameter 'preview'.  Used to generate previews for the file chooser.
   *
   * @param string $file
   * @param string $mimetype
   * @param boolean $preview
   * @return $result
   */
  public function execIngestRules($file, $mimetype, $preview = FALSE) {
    $ret = FALSE;
    if ($this->validate()) {
      $ret = TRUE;
      $rules = $this->xml->getElementsByTagName('ingest_rules')->item(0)->getElementsByTagName('rule');
      for ($i = 0; $i < $rules->length; $i++) {
        $rule = $rules->item($i);
        $types = $rule->getElementsbyTagName('applies_to');
        $valid_types = array();
        for ($j = 0; $j < $types->length; $j++) {
          $valid_types[] = trim($types->item($j)->nodeValue);
        }

        if (in_array($mimetype, $valid_types)) {
          $methods = $rule->getElementsByTagName('ingest_methods');
          if ($methods->length > 0) {
            $methods = $methods->item(0)->getElementsbyTagName('ingest_method');
            for ($j = 0; $j < $methods->length; $j++) {
              $method = $methods->item($j);
              $param_array = array();
              $params = $method->getElementsByTagName('parameters');
              $param_array['model_pid'] = $this->pid;
              if ($params->length > 0) {
                $params = $params->item(0)->getElementsByTagName('parameter');
                for ($k = 0; $k < $params->length; $k++)
                  $param_array[$params->item($k)->getAttribute('name')] = $params->item($k)->nodeValue;
              }


              if (!$preview || isset($param_array['preview'])) {
                $module = $method->getAttribute('module');
                $path = drupal_get_path('module', !empty($module) ? $module : 'fedora_repository') . '/' . $method->getAttribute('file');

                if (!file_exists($path) || substr_compare($path, 'fedora_repository/', -strlen('fedora_repository/'), strlen('fedora_repository/')) === 0) {
                  self::$errors[] = 'Execute Ingest Rules: file \'' . $path . '\' does not exist.';
                  $ret = FALSE;
                }
                else {
                  require_once($path);
                  $className = $method->getAttribute('class');
                  $methodName = ($method->getAttribute('method'));
                  if (!class_exists($className)) {
                    self::$errors[] = 'Execute Ingest Rules: class \'' . $className . '\' does not exist.';
                    $ret = FALSE;
                  }
                  else
                    $class = new $className;
                  if (!method_exists($class, $methodName)) {
                    self::$errors[] = 'Execute Ingest Rules: method \'' . $className . '->' . $methodName . '\' does not exist.';
                    $ret = FALSE;
                  }
                  else {
                    $status = $class->$methodName($param_array, $method->getAttribute('dsid'), $file, $method->getAttribute('modified_files_ext'));
                    if ($status !== TRUE) {
                      self::$errors[] = 'Execute Ingest Rules: method \'' . $className . '->' . $methodName . '\' failed.';
                      $ret = FALSE;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    return $ret;
  }

  /**
   * Executes the form handler from the <form_builder_method> element of <ingest_form>.
   *
   * @param &$formData
   * @param &$form_state
   * @return boolean $success
   */
  public function execFormHandler(&$data, &$form_state) {
    $ret = FALSE;
    if ($this->validate()) {
      $method = $this->xml->getElementsByTagName('ingest_form')->item(0)->getElementsByTagName('form_builder_method')->item(0);
      $module = $method->getAttribute('module');
      $path = drupal_get_path('module', !empty($module) ? $module : 'fedora_repository') . '/' . $method->getAttribute('file');
      if (!file_exists($path)) {
        self::$errors[] = 'Execute Form Handler: file \'' . $path . '\' does not exist.';
      }
      else {
        require_once($path);
        $className = $method->getAttribute('class');
        $methodName = ($method->getAttribute('handler'));
        if (!class_exists($className)) {
          self::$errors[] = 'Execute Form Handler: class \'' . $className . '\' does not exist.';
        }
        else {
          $class = new $className;
          if (!method_exists($class, $methodName)) {
            self::$errors[] = 'Execute Form Handler: method \'' . $className . '->' . $methodName . '\' does not exist.';
          }
          else {
            $class->$methodName($data, $form_state);
            $ret = TRUE;
          }
        }
      }
    }

    return $ret;
  }

  /**
   * Gets a list of valid mimetypes that can apply to this model.
   * Returns FALSE on failure.
   *
   * @return string[] $mimetypes
   */
  public function getMimetypes() {
    //only proceed if the xml is valid.
    if ($this->validate()) {
      if ($this->mimes === NULL) {
        $result = $this->xml->getElementsByTagName('mimetypes');
        $result = $result->item(0)->getElementsByTagName('type');
        $mimes = array();
        for ($i = 0; $i < $result->length; $i++) {
          $mimes[] = trim($result->item($i)->nodeValue);
        }
        $this->mimes = $mimes;
      }
      return $this->mimes;
    }
  }

  /**
   * Calls all defined display methods for the ContentModel.
   * The PID specified is passed to the constructor of the display
   * class(es) specified in the Content Model.
   *
   * @param string $pid
   * @return string $output
   */
  public function displayExtraFieldset($pid, $page_number) {
    $output = '';
    if ($this->validate()) {
      $datastreams = $this->xml->getElementsByTagName('datastreams');
      if ($datastreams->length > 0) {
        $datastreams = $datastreams->item(0)->getElementsByTagName('datastream');
        for ($i = 0; $i < $datastreams->length; $i++) {
          $ds = $datastreams->item($i);
          if ($ds->attributes->getNamedItem('display_in_fieldset') == NULL || strtolower($ds->getAttribute('display_in_fieldset')) != 'FALSE') {
            $dispMethods = $ds->getElementsByTagName('display_method');
            for ($j = 0; $j < $dispMethods->length; $j++) {
              $method = $dispMethods->item($j);
              $module = $method->getAttribute('module');
              $path = drupal_get_path('module', !empty($module) ? $module : 'fedora_repository') . '/' . $method->getAttribute('file');
              if (!file_exists($path)) {
                self::$errors[] = 'Execute Form Handler: file \'' . $path . '\' does not exist.';
              }
              else {
                require_once($path);
                $className = $method->getAttribute('class');
                $methodName = ($method->getAttribute('method'));
                if (!class_exists($className)) {
                  self::$errors[] = 'Execute Form Handler: class \'' . $className . '\' does not exist.';
                }
                else {
                  $class = new $className($pid);
                  if (!method_exists($class, $methodName)) {
                    self::$errors[] = 'Execute Form Handler: method \'' . $className . '->' . $methodName . '\' does not exist.';
                  }
                  else {
                    $output = $class->$methodName($page_number);
                  }
                }
              }
            }
          }
        }
      }
    }

    return $output;
  }

  /**
   * Gets a list of datastreams from the ContentModel
   * (not including the QDC ds if it is listed).
   * Returns FALSE on failure.
   *
   * @return string[] $datastreams
   */
  public function listDatastreams() {
    $ds_array = array();
    if ($this->validate()) {
      $datastreams = $this->xml->getElementsByTagName('datastreams');
      if ($datastreams->length > 0) {
        $datastreams = $datastreams->item(0)->getElementsByTagName('datastream');
        for ($i = 0; $i < $datastreams->length; $i++) {
          $dsName = $datastreams->item($i)->getAttribute('dsid');
          // if ($dsName != 'QDC')
          // {
          $ds_array[] = $dsName;
          // }
        }
      }
    }

    return $ds_array;
  }

  /**
   * Adds an allowed mimetype to the model. 
   * 
   * @param String $type
   * @return boolean $success
   */
  public function addMimetype($type) {
    $ret = FALSE;
    if ($this->validate()) {
      $mimetypesEl = $this->xml->getElementsByTagName('mimetypes')->item(0);
      $types = $mimetypesEl->getElementsByTagName('type');

      $found = FALSE;
      for ($i = 0; !$found && $i < $types->length; $i++) {
        if ($types->item($i)->nodeValue == $type)
          $found = TRUE;
      }

      if (!$found) {
        $newTypeEl = $this->xml->createElement('type', $type);
        $mimetypesEl->appendChild($newTypeEl);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * Removes an allowed mimetype from the model.
   *
   * @param String $type
   * @return boolean $success
   */
  public function removeMimetype($type) {
    $ret = FALSE;
    if ($this->validate()) {
      $mimetypesEl = $this->xml->getElementsByTagName('mimetypes')->item(0);
      $types = $mimetypesEl->getElementsByTagName('type');
      $found = FALSE;
      for ($i = 0; !$found && $i < $types->length; $i++) {
        if ($types->item($i)->nodeValue == $type)
          $found = $types->item($i);
      }

      if ($found !== FALSE && $types->length > 1) {
        $mimetypesEl->removeChild($found);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * getDisplayMethods ??
   * @param type $ds
   * @return type 
   */
  public function getDisplayMethods($ds) {
    $ret = FALSE;
    if (($ds = $this->getDSModel($ds)) !== FALSE) {
      $ret = array();
      $dispMethods = $ds->getElementsByTagName('display_method');
      for ($i = 0; $i < $dispMethods->length; $i++) {
        $ret[] = array('module' => $dispMethods->item($i)->getAttribute('module') == '' ? 'fedora_repository' : $dispMethods->item($i)->getAttribute('module'),
          'file' => $dispMethods->item($i)->getAttribute('file'),
          'class' => $dispMethods->item($i)->getAttribute('class'),
          'method' => $dispMethods->item($i)->getAttribute('method'),
          'default' => ($dispMethods->item($i)->attributes->getNamedItem('default') !== NULL ? strtolower($dispMethods->item($i)->getAttribute('default')) == 'TRUE' : FALSE));
      }
    }
    return $ret;
  }

  /**
   * addDs ??
   * @param type $dsid
   * @param type $display_in_fieldset
   * @return boolean 
   */
  public function addDs($dsid, $display_in_fieldset = FALSE) {
    $ret = FALSE;

    if (self::valid_dsid($dsid) && ($ds = $this->getDSModel($dsid)) === FALSE) {
      $datastreamsEl = $this->xml->getElementsByTagName('datastreams');
      if ($datastreamsEl->length > 0) {
        $datastreamsEl = $datastreamsEl->item(0);
      }
      else {
        $datastreamsEl = $this->xml->createElement('datastreams');
        $this->xml->getElementsByTagName('content_model')->item(0)->appendChild($datastreamsEl);
      }

      $dsEl = $this->xml->createElement('datastream');
      $dsEl->setAttribute('dsid', $dsid);
      if ($display_in_fieldset == TRUE) {
        $dsEl->setAttribute('display_in_fieldset', 'TRUE');
      }
      $datastreamsEl->appendChild($dsEl);
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * removeDs ??
   * @param type $dsid
   * @return boolean 
   */
  public function removeDs($dsid) {
    $ret = FALSE;

    if (self::valid_dsid($dsid) && ($ds = $this->getDSModel($dsid)) !== FALSE) {
      $datastreamsEl = $this->xml->getElementsByTagName('datastreams')->item(0);
      $datastreamsEl->removeChild($ds);
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * displayInFieldset ??
   * @param type $dsid
   * @return type 
   */
  public function displayInFieldset($dsid) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && ($ds = $this->getDSModel($dsid)) !== FALSE) {
      $ret = strtolower($ds->getAttribute('display_in_fieldset')) == 'TRUE';
    }
    return $ret;
  }

  /**
   * setDisplayInFieldset ??
   * @param type $dsid
   * @param type $value
   * @return boolean 
   */
  public function setDisplayInFieldset($dsid, $value = TRUE) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && ($ds = $this->getDSModel($dsid)) !== FALSE) {
      if ($value == FALSE && $ds->attributes->getNamedItem('display_in_fieldset') !== NULL) {
        $ds->removeAttribute('display_in_fieldset');
        $ret = TRUE;
      }
      elseif ($value == TRUE) {
        $ds->setAttribute('display_in_fieldset', 'TRUE');
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * setDefaultDispMeth ??
   * @param type $dsid
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @return boolean 
   */
  public function setDefaultDispMeth($dsid, $module, $file, $class, $method) {
    $ret = FALSE;

    if (self::valid_dsid($dsid) && ($ds = $this->getDSModel($dsid)) !== FALSE) {
      $found = FALSE;
      $dispMethods = $ds->getElementsByTagName('display_method');
      for ($i = 0; !$found && $i < $dispMethods->length; $i++) {
        if ($module == ($dispMethods->item($i)->getAttribute('module') == '' ? 'fedora_repository' : $dispMethods->item($i)->getAttribute('module')) &&
            $file == $dispMethods->item($i)->getAttribute('file') &&
            $class == $dispMethods->item($i)->getAttribute('class') &&
            $method == $dispMethods->item($i)->getAttribute('method')) {
          $found = $dispMethods->item($i);
        }
      }

      if ($found !== FALSE) {

        for ($i = 0; $i < $dispMethods->length; $i++) {
          if ($dispMethods->item($i)->attributes->getNamedItem('default') !== NULL) {
            $dispMethods->item($i)->removeAttribute('default');
          }
        }

        $found->setAttribute('default', 'TRUE');
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * removeDispMeth ??
   * @param type $dsid
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @return boolean 
   */
  public function removeDispMeth($dsid, $module, $file, $class, $method) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && ($ds = $this->getDSModel($dsid)) !== FALSE) {
      $found = FALSE;
      $dispMethods = $ds->getElementsByTagName('display_method');
      for ($i = 0; !$found && $i < $dispMethods->length; $i++) {
        if ($module == ($dispMethods->item($i)->getAttribute('module') == '' ? 'fedora_repository' : $dispMethods->item($i)->getAttribute('module') == '') &&
            $file == $dispMethods->item($i)->getAttribute('file') &&
            $class == $dispMethods->item($i)->getAttribute('class') &&
            $method == $dispMethods->item($i)->getAttribute('method')) {
          $found = $dispMethods->item($i);
        }
      }

      if ($found !== FALSE) {
        $ds->removeChild($found);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * addDispMeth ??
   * @param type $dsid
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @param type $default
   * @return boolean 
   */
  public function addDispMeth($dsid, $module, $file, $class, $method, $default = FALSE) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && ($ds = $this->getDSModel($dsid)) !== FALSE) {
      $dispMethEl = $this->xml->createElement('display_method');
      $dispMethEl->setAttribute('module', $module);
      $dispMethEl->setAttribute('file', $file);
      $dispMethEl->setAttribute('class', $class);
      $dispMethEl->setAttribute('method', $method);
      if ($default == TRUE) {
        $dispMethEl->setAttribute('default', TRUE);
      }
      $ds->appendChild($dispMethEl);
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * getAddDsMethod ??
   * @param type $ds
   * @return type 
   */
  public function getAddDsMethod($ds) {
    $ret = FALSE;
    if (($ds = $this->getDSModel($ds)) !== FALSE) {
      $addDsMethod = $ds->getElementsByTagName('add_datastream_method');
      if ($addDsMethod !== FALSE && $addDsMethod->length > 0) {
        $ret = array('module' => $addDsMethod->item(0)->getAttribute('module') == '' ? 'fedora_repository' : $addDsMethod->item(0)->getAttribute('module'),
          'file' => $addDsMethod->item(0)->getAttribute('file'),
          'class' => $addDsMethod->item(0)->getAttribute('class'),
          'method' => $addDsMethod->item(0)->getAttribute('method'),
          'modified_files_ext' => $addDsMethod->item(0)->getAttribute('modified_files_ext'),
          'dsid' => $addDsMethod->item(0)->getAttribute('dsid')
        );
      }
    }
    return $ret;
  }

  /**
   * getIngestRule ??
   * @param type $rule_id
   * @return type 
   */
  public function getIngestRule($rule_id) {
    $ret = FALSE;
    if ($this->validate()) {
      $rules = $this->xml->getElementsByTagName('ingest_rules')->item(0)->getElementsByTagName('rule');
      if ($rule_id < $rules->length)
        $ret = $rules->item($rule_id);
    }
    return $ret;
  }

  /**
   * removeAppliesTo ??
   * @param type $rule_id
   * @param type $type
   * @return boolean 
   */
  public function removeAppliesTo($rule_id, $type) {
    $ret = FALSE;
    if (($rule = $this->getIngestRule($rule_id)) !== FALSE) {
      $applies = $rule->getElementsByTagName('applies_to');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $applies->length; $i++) {
        if ($type == $applies->item($i)->nodeValue) {
          $found = $applies->item($i);
        }
      }

      if ($found) {
        $rule->removeChild($found);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * addAppliesTo ??
   * @param type $rule_id
   * @param type $type
   * @return boolean 
   */
  public function addAppliesTo($rule_id, $type) {
    $ret = FALSE;
    if (($rule = $this->getIngestRule($rule_id)) !== FALSE) {
      $applies = $rule->getElementsByTagName('applies_to');
      $found = FALSE;
      for ($i = 0; !$found && $i < $applies->length; $i++) {
        $found = ($type == $applies->item($i)->nodeValue);
      }

      if (!$found) {
        $newAppliesTo = $this->xml->createElement('applies_to', $type);
        $rule->insertBefore($newAppliesTo, $rule->getElementsByTagName('ingest_methods')->item(0));
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * addIngestMethods ??
   * @param type $rule_id
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @param type $dsid
   * @param type $modified_files_ext
   * @return boolean 
   */
  public function addIngestMethod($rule_id, $module, $file, $class, $method, $dsid, $modified_files_ext) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && ($rule = $this->getIngestRule($rule_id)) !== FALSE) {
      $methodsEl = $rule->getElementsByTagName('ingest_methods')->item(0);
      $meth = $this->xml->createElement('ingest_method');
      $meth->setAttribute('module', $module);
      $meth->setAttribute('file', $file);
      $meth->setAttribute('class', $class);
      $meth->setAttribute('method', $method);
      $meth->setAttribute('dsid', $dsid);
      $meth->setAttribute('modified_files_ext', $modified_files_ext);

      $methodsEl->appendChild($meth);
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * removeIngestMethod ??
   * @param type $rule_id
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @return boolean 
   */
  public function removeIngestMethod($rule_id, $module, $file, $class, $method) {
    $ret = FALSE;
    if (($rule = $this->getIngestRule($rule_id)) !== FALSE) {
      $found = FALSE;
      $methodsEl = $rule->getElementsByTagName('ingest_methods')->item(0);
      $methods = $methodsEl->getElementsByTagName('ingest_method');
      for ($i = 0; !$found && $i < $methods->length; $i++) {
        if ($methods->item($i)->getAttribute('module') == $module && $methods->item($i)->getAttribute('file') == $file && $methods->item($i)->getAttribute('class') == $class && $methods->item($i)->getAttribute('method') == $method) {
          $found = $methods->item($i);
        }
      }

      if ($found !== FALSE && $methods->length > 1) {
        $methodsEl->removeChild($found);
        $ret = TRUE;
      }
    }
    return $ret;
  }

  /**
   * addIngestMethodParam ??
   * @param type $rule_id
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @param type $name
   * @param type $value
   * @return boolean 
   */
  public function addIngestMethodParam($rule_id, $module, $file, $class, $method, $name, $value) {
    $ret = FALSE;
    if (($rule = $this->getIngestRule($rule_id)) !== FALSE) {
      $methods = $rule->getElementsByTagName('ingest_methods')->item(0)->getElementsByTagName('ingest_method');
      $found = FALSE;
      for ($i = 0; $found === FALSE && $i < $methods->length; $i++) {
        if (($methods->item($i)->getAttribute('module') == '' ? 'fedora_repository' : $methods->item($i)->getAttribute('module')) == $module &&
            $methods->item($i)->getAttribute('file') == $file &&
            $methods->item($i)->getAttribute('class') == $class &&
            $methods->item($i)->getAttribute('method') == $method) {
          $found = $methods->item($i);
        }
      }

      if ($found !== FALSE) {
        $paramsEl = $found->getElementsByTagName('parameters');
        if ($paramsEl->length == 0) {
          $paramsEl = $found->appendChild($this->xml->createElement('parameters'));
        }
        else {
          $paramsEl = $paramsEl->item(0);
        }

        $params = $paramsEl->getElementsByTagName('parameter');
        $found = FALSE;
        for ($i = 0; $found === FALSE && $i < $params->length; $i++) {
          if ($params->item($i)->getAttribute('name') == $name) {
            $found = $params->item($i);
          }
        }

        if ($found === FALSE) {
          $param = $this->xml->createElement('parameter', $value);
          $param->setAttribute('name', $name);
          $paramsEl->appendChild($param);
          $ret = TRUE;
        }
      }
    }
    return $ret;
  }

  /**
   * removeIngestMethodParam ??
   * @param type $rule_id
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @param type $name
   * @return boolean 
   */
  public function removeIngestMethodParam($rule_id, $module, $file, $class, $method, $name) {
    $ret = FALSE;
    if (($rule = $this->getIngestRule($rule_id)) !== FALSE) {
      $found = FALSE;
      $methodsEl = $rule->getElementsByTagName('ingest_methods')->item(0);
      $methods = $methodsEl->getElementsByTagName('ingest_method');
      for ($i = 0; !$found && $i < $methods->length; $i++) {
        if (( trim($methods->item($i)->getAttribute('module')) == $module || (trim($methods->item($i)->getAttribute('module')) == '' && $module == 'fedora_repository')) && trim($methods->item($i)->getAttribute('file')) == $file && trim($methods->item($i)->getAttribute('class')) == $class && trim($methods->item($i)->getAttribute('method')) == $method) {
          $found = $methods->item($i);
        }
      }
      if ($found !== FALSE) {
        $methodEl = $found;
        $paramsEl = $found->getElementsByTagName('parameters');
        if ($paramsEl->length > 0) {
          $paramsEl = $paramsEl->item(0);
          $params = $paramsEl->getElementsByTagName('parameter');
          $found = FALSE;
          for ($i = 0; $found === FALSE && $i < $params->length; $i++) {
            if ($params->item($i)->getAttribute('name') == $name) {
              $found = $params->item($i);
            }
          }

          if ($found !== FALSE) {
            $paramsEl->removeChild($found);
            if ($params->length == 0) {
              $methodEl->removeChild($paramsEl);
            }

            $ret = TRUE;
          }
        }
      }
    }
    return $ret;
  }

  /**
   * removeIngestRule ??
   * @param type $rule_id
   * @return type 
   */
  public function removeIngestRule($rule_id) {
    $ret = FALSE;
    if (($rule = $this->getIngestRule($rule_id)) !== FALSE) {
      $ret = $this->xml->getElementsByTagName('ingest_rules')->item(0)->removeChild($rule);
    }
    return $ret;
  }

  /**
   * addIngestRule ??
   * @param type $applies_to
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @param type $dsid
   * @param type $modified_files_ext
   * @return type 
   */
  public function addIngestRule($applies_to, $module, $file, $class, $method, $dsid, $modified_files_ext) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && $this->validate()) {
      $ingestRulesEl = $this->xml->getElementsByTagName('ingest_rules')->item(0);
      $rule = $this->xml->createElement('rule');
      $ingestMethodsEl = $this->xml->createElement('ingest_methods');
      $rule->appendChild($ingestMethodsEl);
      $ingestRulesEl->appendChild($rule);
      $newRuleId = $ingestRulesEl->getElementsByTagName('rule')->length - 1;
      $ret = ($this->addAppliesTo($newRuleId, $applies_to) && $this->addIngestMethod($newRuleId, $module, $file, $class, $method, $dsid, $modified_files_ext));
    }
    return $ret;
  }

  /**
   * getIngestRules ??
   * @return type 
   */
  public function getIngestRules() {
    $ret = FALSE;
    if ($this->validate()) {
      $ret = array();
      $rules = $this->xml->getElementsByTagName('ingest_rules')->item(0)->getElementsByTagName('rule');
      for ($i = 0; $i < $rules->length; $i++) {
        $rule = array('applies_to' => array(),
          'ingest_methods' => array());
        $applies_to = $rules->item($i)->getElementsByTagName('applies_to');
        for ($j = 0; $j < $applies_to->length; $j++) {
          $rule['applies_to'][] = trim($applies_to->item($j)->nodeValue);
        }

        $methods = $rules->item($i)->getElementsByTagName('ingest_methods')->item(0)->getElementsByTagName('ingest_method');
        for ($j = 0; $j < $methods->length; $j++) {
          $method = array('module' => $methods->item($j)->getAttribute('module') == '' ? 'fedora_repository' : $methods->item($j)->getAttribute('module'),
            'file' => $methods->item($j)->getAttribute('file'),
            'class' => $methods->item($j)->getAttribute('class'),
            'method' => $methods->item($j)->getAttribute('method'),
            'dsid' => $methods->item($j)->getAttribute('dsid'),
            'modified_files_ext' => $methods->item($j)->getAttribute('modified_files_ext'),
            'parameters' => array());

          $params = $methods->item($j)->getElementsByTagName('parameters');
          if ($params->length > 0) {
            $params = $params->item(0)->getElementsByTagName('parameter');
            for ($k = 0; $k < $params->length; $k++) {
              $method['parameters'][$params->item($k)->getAttribute('name')] = $params->item($k)->nodeValue;
            }
          }

          $rule['ingest_methods'][] = $method;
        }

        $ret[] = $rule;
      }
    }
    return $ret;
  }

  /**
   * getIngestFormAttributes ??
   * @return type 
   */
  public function getIngestFormAttributes() {
    $ret = FALSE;
    if ($this->validate()) {
      $ingest_formEl = $this->xml->getElementsByTagName('ingest_form')->item(0);
      $ret = array('dsid' => $ingest_formEl->getAttribute('dsid'),
        'page' => $ingest_formEl->getAttribute('page'),
        'hide_file_chooser' => strtolower($ingest_formEl->getAttribute('hide_file_chooser')) == 'TRUE',
        'redirect' => strtolower($ingest_formEl->getAttribute('redirect')) == 'FALSE' ? FALSE : TRUE);
    }
    return $ret;
  }

  /**
   * editIngestFormAttributes ??
   * @param type $dsid
   * @param type $page
   * @param type $hide_file_chooser
   * @param type $redirect
   * @return boolean 
   */
  public function editIngestFormAttributes($dsid, $page, $hide_file_chooser = FALSE, $redirect = TRUE) {
    $ret = FALSE;
    if (self::valid_dsid($dsid) && $this->validate()) {
      $ingest_formEl = $this->xml->getElementsByTagName('ingest_form')->item(0);
      $ingest_formEl->setAttribute('dsid', $dsid);
      $ingest_formEl->setAttribute('page', $page);
      if (!$redirect) {
        $ingest_formEl->setAttribute('redirect', 'FALSE');
      }
      else {
        $ingest_formEl->removeAttribute('redirect');
      }
      if ($hide_file_chooser) {
        $ingest_formEl->setAttribute('hide_file_chooser', 'TRUE');
      }
      else {
        $ingest_formEl->removeAttribute('hide_file_chooser');
      }
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * getIngestFormBuilderMethod
   * @return type 
   */
  public function getIngestFormBuilderMethod() {
    $ret = FALSE;
    if ($this->validate()) {
      $method = $this->xml->getElementsByTagName('ingest_form')->item(0)->getElementsByTagName('form_builder_method')->item(0);
      $ret = array('module' => ($method->getAttribute('module') == '' ? 'fedora_repository' : $method->getAttribute('module')),
        'file' => $method->getAttribute('file'),
        'class' => $method->getAttribute('class'),
        'method' => $method->getAttribute('method'),
        'handler' => $method->getAttribute('handler'));
    }
    return $ret;
  }

  /**
   * editIngestFormBuilderMethod ??
   * @param type $module
   * @param type $file
   * @param type $class
   * @param type $method
   * @param type $handler
   * @return boolean 
   */
  public function editIngestFormBuilderMethod($module, $file, $class, $method, $handler) {
    $ret = FALSE;
    if ($this->validate()) {
      $methodEl = $this->xml->getElementsByTagName('ingest_form')->item(0)->getElementsByTagName('form_builder_method')->item(0);
      $methodEl->setAttribute('module', $module);
      $methodEl->setAttribute('file', $file);
      $methodEl->setAttribute('class', $class);
      $methodEl->setAttribute('method', $method);
      $methodEl->setAttribute('handler', $handler);
      $ret = TRUE;
    }
    return $ret;
  }

  /**
   * Find the form element with name $name.
   * 
   * @param string $name
   *   The name of the form element to find.
   * @return DOMElement
   *   The form element $name, if found FALSE otherwise.
   */
  public function getForm($name) {
    $result = $this->xpath->query("//cm:form[@name='$name']");
    return $result->length == 1 ? $result->item(0) : FALSE;
  }

  /**
   *
   * @return array
   *    An array of form names that exist in this content model.
   */
  public function getFormNames() {
    if (!$this->validate()) {
      return FALSE;
    }
    $names = FALSE;
    $result = $this->xpath->query('//cm:forms/cm:form/@name'); // Select the name attribute of all forms.
    for ($i = 0; $i < $result->length; $i++) {
      $attribute = $result->item($i);
      $name = $attribute->value;
      $names[$name] = $name;
    }
    return $names;
  }

  /**
   *
   * @return array
   *    An array of form names that exist in this content model.
   */
  public function getIngestFormNames() {
    if (!$this->validate()) {
      return FALSE;
    }
    $result = $this->xpath->query('//cm:forms/cm:form[@ingest_class and @ingest_file and @ingest_module]/@name'); // Select the name attribute of all forms.
    for ($i = 0; $i < $result->length; $i++) {
      $attribute = $result->item($i);
      $name = $attribute->value;
      $names[$name] = $name;
    }
    return $names;
  }

  /**
   *
   * @return array
   *    An array of form names that exist in this content model.
   */
  public function getEditFormNames() {
    if (!$this->validate()) {
      return FALSE;
    }
    $result = $this->xpath->query('//cm:forms/cm:form[@edit_class and @edit_file and @edit_module]/@name'); // Select the name attribute of all forms.
    for ($i = 0; $i < $result->length; $i++) {
      $attribute = $result->item($i);
      $name = $attribute->value;
      $names[$name] = $name;
    }
    return $names;
  }

  /**
   * Removes the named form.
   *
   * @param string $name
   *   Name of the form to remove.
   * 
   * @return boolean
   *   TRUE on success, FALSE otherwise.
   */
  public function removeForm($name) {
    $result = $this->xpath->query("//cm:form[@name='$name']");
    if ($result->length == 1) {
      $form = $result->item(0);
      $form->parentNode->removeChild($form);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Adds the named form.
   *
   * @param string $element
   *   Name of the form to add.
   *
   * @return boolean
   *   TRUE on success, FALSE otherwise.
   */
  public function addForm($element) {
    $result = $this->xpath->query("//cm:forms");
    if ($result->length == 0) { // Forms doesn't exist
      $forms = $this->xml->createElement('forms');
      $element = $this->xml->importNode($element);
      $forms->appendChild($element);
      $result = $this->xpath->query("//cm:content_model");
      $result->item(0)->appendChild($forms);
      return TRUE;
    }
    elseif ($result->length == 1) {
      $element = $this->xml->importNode($element);
      $result->item(0)->appendChild($element);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Edits a form element with attribute name='$name' from the 'forms' element.
   *
   * @param String $name
   */
  public function editForm($name, $element) {
    if (!$this->validate()) {
      return FALSE;
    }
    $result = $this->xpath->query("//cm:form[@name='$name']");
    if ($result->length == 1) {
      $form = $result->item(0);
      $result = $this->xpath->query("child::node()", $form);
      $element = $this->xml->importNode($element);
      for ($i = 0; $i < $result->length; $i++) {
        $child = $result->item($i);
        $element->appendChild($child);
      }
      $form->parentNode->replaceChild($element, $form);
      return TRUE;
    }
    return FALSE;
  }

  /**
   *
   * @param <type> $element_definition
   * @param <type> $parent_element
   * @return <type>
   */
  public function addElementToForm($element, $parent_element) {
    $element = $this->xml->importNode($element, TRUE);
    $parent_element->appendChild($element);
    return TRUE;
  }

  /**
   *
   * @param <type> $element_definition
   * @param <type> $parent_element
   */
  public function editFormElement($new_element, $edit_element) {
    $new_element = $this->xml->importNode($new_element, TRUE);
    $name = $new_element->tagName;
    $result = $new_element->getElementsByTagName('content');
    if ($result->length == 1) {
      $new_content = $result->item(0);
      $result = $this->xpath->query("child::cm:content/child::node()", $edit_element);
      for ($i = 0; $i < $result->length; $i++) {
        $child = $result->item($i);
        $new_content->appendChild($child);
      }
    }
    $edit_element->parentNode->replaceChild($new_element, $edit_element);
    return TRUE;
  }

  /**
   *
   * @param <type> $new_element
   * @param <type> $element
   */
  private function insertElementInPlace($new_element, $element) {
    $next = $element->nextSibling;
    if ($next) {
      $element->parentNode->insertBefore($new_element, $next);
    }
    else {
      $element->parentNode->appendChild($new_element);
    }
  }

  /**
   *
   * @param <type> $form_element
   * @return <type>
   */
  public function incFormElement($form_element) {
    $prev = $form_element;
    $name = $prev->getAttribute('name');
    while ($prev = $prev->previousSibling) {
      if (get_class($prev) == 'DOMElement') {
        $form_element->parentNode->insertBefore($form_element, $prev);
        break;
      }
    }
    return TRUE;
  }

  /**
   *
   * @param <type> $form_element
   * @return <type>
   */
  public function decFormElement($form_element) {
    $next = $form_element;
    while ($next = $next->nextSibling) {
      if (get_class($next) == 'DOMElement') {
        $this->insertElementInPlace($form_element, $next);
        break;
      }
    }
    return TRUE;
  }

}
