<?php



/**
 * @file
 * FormBuilder class
 */

/**
 * implements methods from content model ingest form xml
 * builds a dc metadata form
 */
class FormBuilder {

  /**
   * Constructor
   */
  function FormBuilder() {
    module_load_include('inc', 'FormBuilder', '');
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  /**
   * Create QDC Stream ??
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createQDCStream($form_values, &$dom, &$rootElement) {
    $datastream = $dom->createElement("foxml:datastream");
    $datastream->setAttribute("ID", "DC");
    $datastream->setAttribute("STATE", "A");
    $datastream->setAttribute("CONTROL_GROUP", "X");
    $version = $dom->createElement("foxml:datastreamVersion");
    $version->setAttribute("ID", "DC.0");
    $version->setAttribute("MIMETYPE", "text/xml");
    $version->setAttribute("LABEL", "Dublin Core Record");
    $datastream->appendChild($version);
    $content = $dom->createElement("foxml:xmlContent");
    $version->appendChild($content);
    ///begin writing qdc
    $oai = $dom->createElement("oai_dc:dc");
    $oai->setAttribute('xmlns:oai_dc', "http://www.openarchives.org/OAI/2.0/oai_dc/");
    $oai->setAttribute('xmlns:dc', "http://purl.org/dc/elements/1.1/");
    $oai->setAttribute('xmlns:dcterms', "http://purl.org/dc/terms/");
    $oai->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
    $content->appendChild($oai);
    //dc elements
    $previousElement = NULL; //used in case we have to nest elements for qualified dublin core
    foreach ($form_values as $key => $value) {
      $key = str_replace('_', ':', $key);
      $index = strrpos($key, '-');
      if ($index > 01) {
        $key = substr($key, 0, $index);
      }
      $test = substr($key, 0, 2);

      if ($test == 'dc' || $test == 'ap') {
        //don't try to process other form values
        try {
          if (!strcmp(substr($key, 0, 4), 'app_')) {
            $key = substr($key, 4);
            $previousElement->appendChild($dom->createElement($key, $value));
          }
          else {
            $previousElement = $dom->createElement($key, $value);
            $oai->appendChild($previousElement);
          }
        } catch (exception $e) {
          drupal_set_message(t('@e', array('@e' => check_plain($e->getMessage()))), 'error');
          continue;
        }
      }
      $rootElement->appendChild($datastream);
    }
  }

  /**
   * Create the security policy
   * @param type $collectionPid
   * @param type $dom
   * @param type $rootElement
   * @return type 
   */
  function createPolicy($collectionPid, &$dom, &$rootElement) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $objectHelper = new ObjectHelper();

    $dsid = 'CHILD_SECURITY';
    $policyStreamDoc = $objectHelper->getStream($collectionPid, $dsid, FALSE);
    if (!isset($policyStreamDoc)) {
      return NULL; //there is no policy stream so object will not have a policy stream
    }
    try {
      $xml = new SimpleXMLElement($policyStreamDoc);
    } catch (Exception $e) {
      watchdog(t("fedora_repository"), "Problem getting security policy.", NULL, WATCHDOG_ERROR);
      drupal_set_message(t('Problem getting security policy: @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return FALSE;
    }
    $policyElement = $dom->createDocumentFragment();
    if (!$policyElement) {
      drupal_set_message(t('Error parsing security policy stream.'));
      watchdog(t("fedora_repository"), "Error parsing security policy stream, could not parse policy stream.", NULL, WATCHDOG_NOTICE);
      return FALSE;
    }
    $dom->importNode($policyElement, TRUE);
    $value = $policyElement->appendXML($policyStreamDoc);
    if (!$value) {
      drupal_set_message(t('Error creating security policy stream.'));
      watchdog(t("fedora_repository"), "Error creating security policy stream, could not parse collection policy template file.", NULL, WATCHDOG_NOTICE);
      return FALSE;
    }

    $ds1 = $dom->createElement("foxml:datastream");
    $rootElement->appendChild($ds1);
    $ds1->setAttribute("ID", "POLICY");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "X");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1->appendChild($ds1v);
    $ds1v->setAttribute("ID", "POLICY.0");
    $ds1v->setAttribute("MIMETYPE", "text/xml");
    $ds1v->setAttribute("LABEL", "POLICY");
    $content = $dom->createElement("foxml:xmlContent");
    $ds1v->appendChild($content);
    $content->appendChild($policyElement);
    return TRUE;
  }

  /**
   * Handle QDC Form ??
   * @param type $form_values
   * @return type 
   */
  function handleQDCForm($form_values) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'CollectionPolicy');

    $dom = new DomDocument("1.0", "UTF-8");
    $dom->formatOutput = TRUE;
    $pid = $form_values['pid'];
    $rootElement = $dom->createElement("foxml:digitalObject");
    $rootElement->setAttribute('VERSION', '1.1');
    $rootElement->setAttribute('PID', "$pid");
    $rootElement->setAttribute('xmlns:foxml', "info:fedora/fedora-system:def/foxml#");
    $rootElement->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
    $rootElement->setAttribute('xsi:schemaLocation', "info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd");
    $dom->appendChild($rootElement);
    // Create standard fedora stuff
    $this->createStandardFedoraStuff($form_values, $dom, $rootElement);
    // Create relationships
    $this->createRelationShips($form_values, $dom, $rootElement);
    $collectionPid = $form_values['collection_pid'];

    if (($cp = CollectionPolicy::LoadFromCollection($collectionPid)) !== FALSE) {
      $collectionName = trim($cp->getName());
      if (trim($collectionName) != '') {
        $form_values['dc_relation'] = $collectionName;
      }
    }
    // Create dublin core
    $this->createQDCStream($form_values, $dom, $rootElement);

    if (!empty($form_values['ingest-file-location'])) {
      $this->createFedoraDataStreams($form_values, $dom, $rootElement);
    }
    $this->createPolicy($collectionPid, &$dom, &$rootElement);

    try {

      $object = Fedora_Item::ingest_from_FOXML($dom);
      if (!empty($object->pid)) {
        // drupal_set_message("Item ". l($object->pid, 'fedora/repository/'. $object->pid) . " created successfully.", "status");
        drupal_set_message(t("Item !pid created successfully.", array('!pid' => l(check_plain($object->pid), 'fedora/repository/' . check_plain($object->pid)))), "status");
      }
      if (!empty($_SESSION['fedora_ingest_files'])) {
        foreach ($_SESSION['fedora_ingest_files'] as $dsid => $createdFile) {
          file_delete($createdFile);
        }
      }
      file_delete($form_values['ingest-file-location']);
    } catch (exception $e) {
      drupal_set_message(t('Error ingesting object: @e', array('@e' => check_plain($e->getMessage()))), 'error');
      watchdog(t("fedora_repository"), "Error ingesting object: @e", array('@e' => check_plain($e->getMessage())), NULL, WATCHDOG_ERROR);
      return;
    }
  }

  /**
   * Create Fedora Data stream
   * @global type $base_url
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createFedoraDataStreams($form_values, &$dom, &$rootElement) {
    module_load_include('inc', 'fedora_repository', 'MimeClass');
    global $base_url;
    $mimetype = new MimeClass();
    $server = NULL;
    $file = $form_values['ingest-file-location'];

    if (!empty($file)) {
      $dformat = $mimetype->getType($file);

      $parts = explode('/', $file);
      foreach ($parts as $n => $part) {
        $parts[$n] = rawurlencode($part);
      }
      $path = implode('/', $parts);
      $fileUrl = $base_url . '/' . $path;

      $beginIndex = strrpos($fileUrl, '/');
      $dtitle = substr($fileUrl, $beginIndex + 1);
      $dtitle = urldecode($dtitle);
      $ds1 = $dom->createElement("foxml:datastream");
      $ds1->setAttribute("ID", "OBJ");
      $ds1->setAttribute("STATE", "A");
      $ds1->setAttribute("CONTROL_GROUP", "M");
      $ds1v = $dom->createElement("foxml:datastreamVersion");
      $rootElement->appendChild($ds1);

      $ds1v->setAttribute("ID", "OBJ.0");
      $ds1v->setAttribute("MIMETYPE", "$dformat");
      $ds1v->setAttribute("LABEL", "$dtitle");
      $ds1content = $dom->createElement('foxml:contentLocation');
      $ds1content->setAttribute("REF", "$fileUrl");
      $ds1content->setAttribute("TYPE", "URL");
      $ds1->appendChild($ds1v);
      $ds1v->appendChild($ds1content);
    }
    if (!empty($_SESSION['fedora_ingest_files'])) {

      foreach ($_SESSION['fedora_ingest_files'] as $dsid => $createdFile) {

        if (!empty($file)) {
          $found = strstr($createdFile, $file);
          if ($found !== FALSE) {
            $createdFile = $found;
          }
        }

        $dformat = $mimetype->getType($createdFile);
        $parts = explode('/', $createdFile);
        foreach ($parts as $n => $part) {
          $parts[$n] = rawurlencode($part);
        }
        $path = implode('/', $parts);
        $fileUrl = $base_url . '/' . $path;

        $beginIndex = strrpos($fileUrl, '/');
        $dtitle = substr($fileUrl, $beginIndex + 1);
        $dtitle = urldecode($dtitle);
        $dtitle = $dtitle;
        $ds1 = $dom->createElement("foxml:datastream");
        $ds1->setAttribute("ID", "$dsid");
        $ds1->setAttribute("STATE", "A");
        $ds1->setAttribute("CONTROL_GROUP", "M");
        $ds1v = $dom->createElement("foxml:datastreamVersion");
        $ds1v->setAttribute("ID", "$dsid.0");
        $ds1v->setAttribute("MIMETYPE", "$dformat");
        $ds1v->setAttribute("LABEL", "$dtitle");
        $ds1content = $dom->createElement('foxml:contentLocation');
        $ds1content->setAttribute("REF", "$fileUrl");
        $ds1content->setAttribute("TYPE", "URL");
        $ds1->appendChild($ds1v);
        $ds1v->appendChild($ds1content);
        $rootElement->appendChild($ds1);
      }
    }
  }

  /**
   * Creates the RELS-EXT for the foxml
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createRelationShips($form_values, &$dom, &$rootElement) {
    $drdf = $dom->createElement("foxml:datastream");
    $drdf->setAttribute("ID", "RELS-EXT");
    $drdf->setAttribute("CONTROL_GROUP", "X");
    $dvrdf = $dom->createElement("foxml:datastreamVersion");
    $dvrdf->setAttribute("FORMAT_URI", "info:fedora/fedora-system:FedoraRELSExt-1.0");
    $dvrdf->setAttribute("ID", "RELS-EXT.0");
    $dvrdf->setAttribute("MIMETYPE", "application/rdf+xml");
    $dvrdf->setAttribute("LABEL", "RDF Statements about this Object");
    $dvcontent = $dom->createElement("foxml:xmlContent");
    $rdf = $dom->createElement("rdf:RDF");
    $rdf->setAttribute("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    $rdf->setAttribute("xmlns:rdfs", "http://www.w3.org/2000/01/rdf-schema#");
    $rdf->setAttribute("xmlns:fedora", "info:fedora/fedora-system:def/relations-external#");
    $rdf->setAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    $rdf->setAttribute("xmlns:oai_dc", "http://www.openarchives.org/OAI/2.0/oai_dc/");
    $rdf->setAttribute("xmlns:fedora-model", "info:fedora/fedora-system:def/model#");
    $rdfdesc = $dom->createElement("rdf:Description");
    $pid = $form_values['pid'];
    $rdfdesc->setAttribute("rdf:about", "info:fedora/$pid");
    $relationship = $form_values['relationship'];
    if (!isset($relationship)) {
      $relationship = 'isMemberOfCollection';
    }
    $member = $dom->createElement("fedora:" . $relationship);
    $membr = $form_values['collection_pid'];
    $member->setAttribute("rdf:resource", "info:fedora/$membr");
    $rdfHasModel = $dom->createElement("fedora-model:hasModel");
    $contentModelPid = $form_values['content_model_pid'];
    $rdfHasModel->setAttribute("rdf:resource", "info:fedora/$contentModelPid");
    $drdf->appendChild($dvrdf);
    $dvrdf->appendChild($dvcontent);
    $dvcontent->appendChild($rdf);
    $rdf->appendChild($rdfdesc);
    $rdfdesc->appendChild($member);
    $rdfdesc->appendChild($rdfHasModel);
    $rootElement->appendChild($drdf);
  }

  /**
   * Creates the standard foxml properties
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createStandardFedoraStuff($form_values, &$dom, &$rootElement) {
    // Foxml object properties section
    $objproperties = $dom->createElement("foxml:objectProperties");
    $prop2 = $dom->createElement("foxml:property");
    $prop2->setAttribute("NAME", "info:fedora/fedora-system:def/model#state");
    $prop2->setAttribute("VALUE", "A");
    $prop3 = $dom->createElement("foxml:property");
    $prop3->setAttribute("NAME", "info:fedora/fedora-system:def/model#label");
    $prop3->setAttribute("VALUE", $form_values['dc:title']);
    $prop5 = $dom->createElement("foxml:property");
    $prop5->setAttribute("NAME", "info:fedora/fedora-system:def/model#ownerId");
    $prop5->setAttribute("VALUE", $form_values['user_id']);
    $objproperties->appendChild($prop2);
    $objproperties->appendChild($prop3);
    $objproperties->appendChild($prop5);
    $rootElement->appendChild($objproperties);
  }

  /**
   * Build QDC Form
   * @param type $form
   * @param type $elements
   * @param type $form_values
   * @return string 
   */
  function buildQDCForm(&$form, $elements, &$form_values) {
    $form['#multistep'] = TRUE; // used so that it triggers a form rebuild every time.
    $form['indicator2'] = array(
      '#type' => 'fieldset',
      '#title' => t('Ingest digital object step #2'),
    );

    foreach ($elements as $element) {

      if ($element['type'] == 'markup') {
        $el = array('#value' => $element['description']);
      }
      else {
        $el = array(
          '#title' => $element['label'],
          '#required' => ($element['required'] ? 1 : 0),
          '#description' => $element['description'],
          '#type' => $element['type']
        );
      }

      $name = explode('][', $element['name']);
      $elLocation = &$form['indicator2'];
      while (isset($elLocation[$name[0]]) && ($partial = array_shift($name)) != NULL) {
        $elLocation = &$elLocation[$partial];
      }

      $autocomplete_path = FALSE;
      $autocomplete_omit_collection = FALSE;
      foreach ($element['parameters'] as $key => $val) {
        if ($key == '#autocomplete_path') {
          $autocomplete_path = $val;
        }
        elseif ($key == '#autocomplete_omit_collection') {
          $autocomplete_omit_collection = TRUE;
        }
        else {
          $el[$key] = $val;
        }
      }

      if ($autocomplete_path !== FALSE) {
        $el['#autocomplete_path'] = $autocomplete_path . (!$autocomplete_omit_collection ? '/' . $form_values['storage']['collection_pid'] : '/');
      }

      if ($element['type'] == 'select' || $element['type'] == 'other_select') {
        $el['#options'] = isset($element['authoritative_list']) ? $element['authoritative_list'] : array();
      }

      $elLocation[join('][', $name)] = $el;
    }

    return $form;
  }

}
