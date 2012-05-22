<?php



/**
 * @file
 * Refworks class
 */

module_load_include('inc', 'fedora_repository', 'SecurityClass');

/**
 * Refworks class ???
 */
class Refworks {

  private $romeoUrlString = "";
  private $referenceList;
  private $securityHelper;
  private $collectionPolicyStream;
  private $issn = '';

  /**
   * Constructor
   */
  function Refworks() {
    $this->romeoUrlString = "http://www.sherpa.ac.uk/romeo/api24.php?issn=";
  }

  /**
   * Build Form ??
   * @param type $form
   * @param type $ingest_form_definition
   * @param type $form_values
   * @return type 
   */
  function buildForm(&$form, $ingest_form_definition, &$form_values) {
    $form['indicator2'] = array(
      '#type' => 'fieldset',
      '#title' => t('Ingest digital object step #2'),
    );
    foreach ($ingest_form_definition->form_elements->element as $element) {
      $name = strip_tags($element->name->asXML());
      $title = strip_tags($element->label->asXML());
      $required = strip_tags($element->required->asXML());
      $prefix = strip_tags($element->prefix);
      $required = strtolower($required);
      if ($required != 'TRUE') {
        $required = '0';
      }

      $description = strip_tags($element->description->asXML());
      $type = strip_tags($element->type->asXML());

      $form['indicator2']["$name"] = array(
        '#title' => $title,
        '#required' => $required,
        '#description' => $description,
        '#prefix' => $prefix,
        '#type' => $type
      );
    }

    return $form;
  }

  /**
   * Read the input file and generate a list of Reference items.
   *
   * @param array $form_values
   * @param SimpleXMLElement $dom
   * @return SimpleXMLElement
   */
  function parse_refworks_item(&$form_values) {

    $file = $form_values['ingest-file-location'];
    try {
      $dom = new DOMDocument('1.0', 'UTF-8');
      $dom->substituteEntities = FALSE;
      $dom->loadXML(trim(file_get_contents($file)));
      $xml = simplexml_import_dom($dom);
      //$xml=simplexml_load_string(trim(file_get_contents($file),NULL,TRUE));
      //$dom = dom_import_simplexml($xml);//test to see if it behaves better
      //$xml = new SimpleXMLElement(trim(file_get_contents($file)));
    } catch (Exception $e) {
      drupal_set_message(t('Error processing Refworks file: @message', array('@message' => $e->getMessage())));
      return FALSE;
    }
    $this->referenceList = array();
    foreach ($xml->reference as $reference) {
      array_push($this->referenceList, $reference);
    }

    return $this->referenceList;
  }

  /**
   * Create a DC stream with ID of DC ???
   * @param type $dom
   * @param type $rootElement
   * @param type $reference
   * @return type 
   */
  function createQDCStream(&$dom, &$rootElement, $reference) {
    $datastream = $dom->createElement("foxml:datastream");
    $datastream->setAttribute("ID", "DC");
    $datastream->setAttribute("STATE", "A");
    $datastream->setAttribute("CONTROL_GROUP", "X");
    $version = $dom->createElement("foxml:datastreamVersion");
    $version->setAttribute("ID", "DC.0");
    $version->setAttribute("MIMETYPE", "text/xml");
    $version->setAttribute("LABEL", "DC Dublin Core Record");
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

    foreach ($reference->sr as $value) {
      $element = $dom->createElement('dc:type', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->id as $value) {
      $identifier = $value;
    }
    foreach ($reference->a1 as $value) {
      $element = $dom->createElement('dc:creator', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->t1 as $value) {
      // $form_values['dc:title'] = $value;
      $element = $dom->createElement('dc:title', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->jf as $value) {
      $source = $value;
    }
    foreach ($reference->yr as $value) {
      $element = $dom->createElement('dc:date', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->vo as $value) {
      $source .= ' Volume: ' . $value;
    }
    foreach ($reference->is as $value) {
      $source .= ' Issue: ' . $value;
    }
    foreach ($reference->sp as $value) {
      $source .= ' Start Page: ' . $value;
    }
    foreach ($reference->op as $value) {
      $source .= ' Other Pages: ' . $value;
    }
    foreach ($reference->ul as $value) {
      $source .= ' URL: ' . $value;
    }
    foreach ($reference->k1 as $value) {
      $element = $dom->createElement('dc:subject', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->a2 as $value) {
      $element = $dom->createElement('dc:contributor', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->a3 as $value) {
      $element = $dom->createElement('dc:contributor', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->a4 as $value) {
      $element = $dom->createElement('dc:contributor', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->a5 as $value) {
      $element = $dom->createElement('dc:contributor', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->la as $value) {
      $element = $dom->createElement('dc:language', htmlspecialchars($value, ENT_NOQUOTES, 'UTF-8'));
      $oai->appendChild($element);
    }
    foreach ($reference->pb as $value) {
      $source = 'Publisher: ' . $value;
    }
    foreach ($reference->pp as $value) {
      $source .= ' Place of publication: ' . $value;
    }
    foreach ($reference->sn as $value) {
      $identifier .= ' ISSN/ISBN: ' . $value;
      //$this->romeoUrlString = $this->romeoUrlString . $value;
      if (!$this->issn == '') {
        $this->issn = $value;
      }
      else {
        $this->issn .= ',' . $value;
      }
    }
    foreach ($reference->ab as $value) {
      $description = ' abstract: ' . $value;
    }
    foreach ($reference->cr as $value) {
      $description .= ' Cited reference: ' . $value;
    }
    $element = $dom->createElement('dc:description', htmlspecialchars($description, ENT_NOQUOTES, 'UTF-8'));
    $oai->appendChild($element);
    $element = $dom->createElement('dc:source', htmlspecialchars($source, ENT_NOQUOTES, 'UTF-8'));
    $oai->appendChild($element);
    $element = $dom->createElement('dc:identifier', htmlspecialchars($identifier, ENT_NOQUOTES, 'UTF-8'));
    $oai->appendChild($element);
    //$rootElement->appendChild($datastream);
    return $datastream;
  }

  /**
   * Handle Form ??
   * @param type $form_values
   * @return type 
   */
  function handleForm(&$form_values) {
    $errorMessage = NULL;
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    module_load_include('inc', 'fedora_repository', 'ContentModel');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $contentModelPid = ContentModel::getPidFromIdentifier($form_values['models']);
    $contentModelDsid = ContentModel::getDSIDFromIdentifier($form_values['models']);
    $collectionHelper = new CollectionClass();
    $startTime = time();
    $collection_pid = $form_values['collection_pid'];

    $this->parse_refworks_item($form_values);

    $this->securityHelper = new SecurityClass();

    $collection_item = new Fedora_Item($collection_pid);
    $this->collectionPolicyStream = $collection_item->get_datastream_dissemination('CHILD_SECURITY');
    if (empty($this->collectionPolicyStream)) {
      $this->collectionPolicyStream = file_get_contents(drupal_get_path('module', 'fedora_repository') . '/policies/noObjectEditPolicy.xml');
    }
    $success = 0;
    $errors = 0;
    foreach ($this->referenceList as $reference) {
      $dom = new DomDocument("1.0", "UTF-8");
      $dom->substituteEntities = FALSE;
      $dom->formatOutput = TRUE;
      $pid = $collectionHelper->getNextPid($collection_pid, $contentModelDsid);

      $rootElement = $dom->createElement("foxml:digitalObject");
      $rootElement->setAttribute('VERSION', '1.1');
      $rootElement->setAttribute('PID', "$pid");
      $rootElement->setAttribute('xmlns:foxml', "info:fedora/fedora-system:def/foxml#");
      $rootElement->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
      $rootElement->setAttribute('xsi:schemaLocation', "info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd");
      $dom->appendChild($rootElement);

      //create standard fedora stuff
      $qdc_element = $this->createQDCStream($dom, $rootElement, $reference);
      if (!$qdc_element) {
        drupal_set_message(t('Error creating DC for Refworks'), 'error');
        continue;
      }
      $item_title = '';
      foreach ($reference->t1 as $value) {
        $item_title .= ' --- ' . $value;
      }
      $this->createStandardFedoraStuff($form_values, $dom, $rootElement, $reference);
      $rootElement->appendChild($qdc_element);
      //create relationships
      $this->createRelationShips($form_values, $dom, $rootElement, $pid);
      //create dublin core

      $this->createFedoraDataStreams($form_values, $dom, $rootElement, $reference);

      if (!empty($this->collectionPolicyStream)) {
        $this->create_security_policies($dom, $rootElement, $reference);
      }

      $params = array(
        'objectXML' => $dom->saveXML(),
        'format' => 'info:fedora/fedora-system:FOXML-1.1',
        'logMessage' => "Fedora Object Ingested",
      );

      try {
        $soapHelper = new ConnectionHelper();
        $client = $soapHelper->getSoapClient(variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/wsdl?api=API-M'));

        if ($client == NULL) {
          drupal_set_message(t('Error getting SOAP client.'), 'error');
          watchdog(t("fedora_repository"), "Error getting SOAP client: @e", array('@e' => $e), NULL, WATCHDOG_ERROR);
          return;
        }
        $object = $client->__soapCall('ingest', array(
              $params
            ));
        watchdog(t("fedora_repository"), "Successfully added repository item " . $pid . " - ". $item_title, NULL, WATCHDOG_INFO);
        $deleteFiles = $form_values['delete_file']; //remove files from drupal file system

        if ($deleteFiles > 0) {
          unlink($form_values['fullpath']);
        }
      } catch (exception $e) {
        $errors++;
        $errorMessage = 'yes';
        watchdog(t("fedora_repository"), t("Error during ingest !it @e", array('!it' => $item_title, '@e' => $e)), NULL, WATCHDOG_ERROR);
      }
      $success++;
    }
    if (isset($errorMessage)) {
      drupal_set_message(t('Error ingesting one or more records! Check Drupal watchdog logs for more info'), 'error');
    }
    $endTime = time();
    drupal_set_message(t('Successfull ingest of %success records.  %errors records failed.  Ingest took %seconds seconds', array('%success' => $success - $errors, '%errors' => $errors, '%seconds' => $endTime - $startTime)), 'info');
  }

  /**
   * Creates the RELS-EXT for the foxml
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement
   * @param type $pid 
   */
  function createRelationShips($form_values, &$dom, &$rootElement, $pid = NULL) {
    $drdf = $dom->createElement("foxml:datastream");
    $drdf->setAttribute("ID", "RELS-EXT");
    $drdf->setAttribute("CONTROL_GROUP", "X");
    $dvrdf = $dom->createElement("foxml:datastreamVersion");
    $dvrdf->setAttribute("ID", "RELS-EXT.0");
    $dvrdf->setAttribute("MIMETYPE", "text/xml");
    $dvrdf->setAttribute("LABEL", "Fedora Object-to-Object Relationship Metadata");
    $dvcontent = $dom->createElement("foxml:xmlContent");
    $rdf = $dom->createElement("rdf:RDF");
    $rdf->setAttribute("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    $rdf->setAttribute("xmlns:rdfs", "http://www.w3.org/2000/01/rdf-schema#");
    $rdf->setAttribute("xmlns:fedora", "info:fedora/fedora-system:def/relations-external#");
    $rdf->setAttribute("xmlns:fedora-model", "info:fedora/fedora-system:def/model#");
    $rdfdesc = $dom->createElement("rdf:description");
    $rdfdesc->setAttribute("rdf:about", "info:fedora/$pid");
    $member = $dom->createElement("fedora:isMemberOfCollection");
    $membr = $form_values['collection_pid'];
    $member->setAttribute("rdf:resource", "info:fedora/$membr");
    $model = $dom->createElement("fedora-model:hasModel");
    $model->setAttribute("rdf:resource", "info:fedora/islandora:refworksCModel");

    $drdf->appendChild($dvrdf);
    $dvrdf->appendChild($dvcontent);
    $dvcontent->appendChild($rdf);
    $rdf->appendChild($rdfdesc);
    $rdfdesc->appendChild($member);
    $rdfdesc->appendChild($model);
    $rootElement->appendChild($drdf);
  }

  /**
   * Create Romeo Datastream
   * @param type $dom
   * @param type $rootElement 
   */
  function createRomeoDataStream(&$dom, &$rootElement) {
    $ds1 = $dom->createElement("foxml:datastream");
    $ds1->setAttribute("ID", "ROMEO");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "E");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1v->setAttribute("ID", "ROMEO.0");
    $ds1v->setAttribute("MIMETYPE", "text/xml");
    $ds1v->setAttribute("LABEL", "ROMEO");
    $ds1content = $dom->createElement('foxml:contentLocation');
    $url = $this->romeoUrlString . $this->issn;
    $this->issn = ''; //clear the issn's for next ingest in case we are doing batch
    $ds1content->setAttribute("REF", "$url");
    $ds1content->setAttribute("TYPE", "URL");
    $ds1->appendChild($ds1v);
    $ds1v->appendChild($ds1content);
    $rootElement->appendChild($ds1);
  }

  /**
   * Create Fedora Datastream
   * @global type $base_url
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement
   * @param type $reference 
   */
  function createFedoraDataStreams($form_values, &$dom, &$rootElement, $reference) {
    global $base_url;
    module_load_include('inc', 'fedora_repository', 'MimeClass');
    $mimetype = new MimeClass();
    $server = NULL;
    $this->createRomeoDataStream($dom, $rootElement);
    $ds1 = $dom->createElement("foxml:datastream");
    $ds1->setAttribute("ID", "refworks");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "X");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1v->setAttribute("ID", "refworks.0");
    $ds1v->setAttribute("MIMETYPE", "text/xml");
    $ds1v->setAttribute("LABEL", "Refworks datastream");
    $ds1content = $dom->createElement("foxml:xmlContent");

    $ds1content->appendChild($dom->importNode(dom_import_simplexml($reference), TRUE));
    $ds1->appendChild($ds1v);
    $ds1v->appendChild($ds1content);
    $rootElement->appendChild($ds1);

    $count = 0;
  }

  /**
   * Creates the standard foxml properties
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement
   * @param type $reference 
   */
  function createStandardFedoraStuff($form_values, &$dom, &$rootElement, $reference) {
    // Foxml object properties section
    $objproperties = $dom->createElement("foxml:objectProperties");
    $prop2 = $dom->createElement("foxml:property");
    $prop2->setAttribute("NAME", "info:fedora/fedora-system:def/model#state");
    $prop2->setAttribute("VALUE", "A");
    $prop3 = $dom->createElement("foxml:property");
    $prop3->setAttribute("NAME", "info:fedora/fedora-system:def/model#label");
    $label = $reference->t1;
    if (strlen($label) > 254) {
      $label = substr($label, 0, 245);
    }
    $prop3->setAttribute("VALUE", $label);
    $prop5 = $dom->createElement("foxml:property");
    $prop5->setAttribute("NAME", "info:fedora/fedora-system:def/model#ownerId");
    $prop5->setAttribute("VALUE", $form_values['user_id']);
    $objproperties->appendChild($prop2);
    $objproperties->appendChild($prop3);
    $objproperties->appendChild($prop5);
    $rootElement->appendChild($objproperties);
  }

  /**
   * Read the list of Users from the U1 field and Roles from the U2 field and add elements
   *  to the security policy record for this item, then add the record as the security policy datastream.
   *
   * @param array $form_values
   * @param DOMDocument $dom
   * @param  $rootElement
   * @param SimpleXMLElement $reference
   */
  function create_security_policies($dom, &$rootElement, $reference) {
    global $user;
    $ds1 = $dom->createElement("foxml:datastream");
    $ds1->setAttribute("ID", "POLICY");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "X");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1v->setAttribute("ID", "POLICY.0");
    $ds1v->setAttribute("MIMETYPE", "text/xml");
    $ds1v->setAttribute("LABEL", "POLICY Record");
    $ds1content = $dom->createElement("foxml:xmlContent");

    $custom_policy = $this->collectionPolicyStream;
    $allowed_users_and_roles = array();
    $allowed_users_and_roles['users'] = array();
    $allowed_users_and_roles['roles'] = array();
    foreach ($reference->u1 as $namelist) {
      foreach (explode(';', strip_tags($namelist->asXML())) as $name) {
        array_push($allowed_users_and_roles['users'], $name);
      }
    }
    if (empty($reference->u1)) {
      // If no "u1" value exists, add the currently logged-in user to the item's security policy.
      array_push($allowed_users_and_roles['users'], $user->name);
    }

    foreach ($reference->u2 as $rolelist) {
      foreach (explode(';', strip_tags($rolelist->asXML())) as $role) {
        array_push($allowed_users_and_roles['roles'], $role);
      }
    }
    $custom_policy = $this->securityHelper->set_allowed_users_and_roles($custom_policy, $allowed_users_and_roles);
    $custom_policy_sxe = new SimpleXMLElement($custom_policy);
    $ds1->appendChild($ds1v);
    $ds1v->appendChild($ds1content);

    $rootElement->appendChild($ds1);
    $ds1content->appendChild($dom->importNode(dom_import_simplexml($custom_policy_sxe), TRUE));
  }

}
