<?php



/**
 * @file
 * PersonalCollectionClass class
 */

/**
 * Personal Collection Class
 */
class PersonalCollectionClass {

  /**
   * Constructor
   */
  function PersonalCollectionClass() {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  /**
   * Create collection
   * @param type $theUser
   * @param type $pid
   * @param type $soapClient
   * @return type 
   */
  function createCollection($theUser, $pid, $soapClient) {
    $dom = new DomDocument("1.0", "UTF-8");
    $dom->formatOutput = TRUE;

    $rootElement = $dom->createElement("foxml:digitalObject");
    $rootElement->setAttribute('PID', "$pid");
    $rootElement->setAttribute('xmlns:foxml', "info:fedora/fedora-system:def/foxml#");
    $rootElement->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
    $rootElement->setAttribute('xsi:schemaLocation', "info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-0.xsd");
    $dom->appendChild($rootElement);
    //create standard fedora stuff
    $this->createStandardFedoraStuff($theUser, $dom, $rootElement);
    //create dublin core
    $this->createDCStream($theUser, $dom, $rootElement);
    $value = $this->createPolicyStream($theUser, $dom, $rootElement);

    if (!$value) {
      return FALSE; //error should already be logged.
    }
    $this->createCollectionPolicyStream($theUser, $dom, $rootElement);
    try {
      $params = array(
        'objectXML' => $dom->saveXML(),
        'format' => "foxml1.0",
        'logMessage' => "Fedora object ingested",
      );

      $object = $soapClient->__soapCall('ingest', array(
          $params
        ));
    } catch (exception $e) {
      drupal_set_message(t('Error ingesting personal collection object: @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Create Collection Policy Stream ??
   * @param type $user
   * @param type $dom
   * @param type $rootElement
   * @return type 
   */
  function createCollectionPolicyStream($user, $dom, $rootElement) {
    $collectionTemplate = file_get_contents(drupal_get_path('module', 'fedora_repository') . '/collection_policies/PERSONAL-COLLECTION-POLICY.xml');
    try {
      $xml = new SimpleXMLElement($collectionTemplate);
    } catch (Exception $e) {
      watchdog(t("fedora_repository"), "Problem creating personal collection policy, could not parse collection policy stream.", NULL, WATCHDOG_ERROR);
      drupal_set_message(t('Problem creating personal collection policy, could not parse collection policy stream: @e', array('@e' => check_plain($e->getMessage()))), 'error');
      return FALSE;
    }
    $policyElement = $dom->createDocumentFragment();

    if (!$policyElement) {
      drupal_set_message(t('Error parsing policy stream.'));
      watchdog(t("fedora_repository"), "Error parsing policy stream, could not parse policy stream.", NULL, WATCHDOG_NOTICE);
      return FALSE;
    }
    $dom->importNode($policyElement, TRUE);

    $value = $policyElement->appendXML($collectionTemplate);
    if (!$value) {
      drupal_set_message(t('Error creating collection policy stream.'));
      watchdog(t("fedora_repository"), "Error creating collection policy stream, could not parse collection policy template file.", NULL, WATCHDOG_NOTICE);
      return FALSE;
    }

    $ds1 = $dom->createElement("foxml:datastream");
    $rootElement->appendChild($ds1);
    $ds1->setAttribute("ID", "COLLECTION_POLICY");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "X");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1->appendChild($ds1v);
    $ds1v->setAttribute("ID", "COLLECTION_POLICY.0");
    $ds1v->setAttribute("MIMETYPE", "text/xml");
    $ds1v->setAttribute("LABEL", "COLLECTION_POLICY");
    //$ds1v->setAttribute("FORMAT_URI","info:fedora/fedora-system:format/xml.fedora.audit");
    $content = $dom->createElement("foxml:xmlContent");
    $ds1v->appendChild($content);
    $content->appendChild($policyElement);
    return TRUE;
  }

  /**
   * Create Policy Stream ??
   * @param type $user
   * @param type $dom
   * @param type $rootElement
   * @return type 
   */
  function createPolicyStream($user, $dom, $rootElement) {
    module_load_include('inc', 'fedora_repository', 'SecurityClass');

    $securityClass = new SecurityClass();
    $policyStreamDoc = $securityClass->createPersonalPolicy($user);
    $policyNodes = $policyStreamDoc->getElementsByTagName('Policy');
    $policyStream = $policyNodes->item(0);
    $policyStream = $dom->importNode($policyStream, TRUE);

    $ds1 = $dom->createElement("foxml:datastream");
    $rootElement->appendChild($ds1);
    $ds1->setAttribute("ID", "POLICY");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "X");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1->appendChild($ds1v);
    $ds1v->setAttribute("ID", "POLICY.0");
    $ds1v->setAttribute("MIMETYPE", "text/xml");
    $ds1v->setAttribute("LABEL", "SECURITY_POLICY");
    //$ds1v->setAttribute("FORMAT_URI","info:fedora/fedora-system:format/xml.fedora.audit");
    $content = $dom->createElement("foxml:xmlContent");
    $ds1v->appendChild($content);
    $content->appendChild($policyStream);
    return $this->createChildPolicyStream($dom, $rootElement, $policyStream->cloneNode(TRUE));
  }

  /**
   * right now this is the same as the policy stream for this object, may change
   * objects in this collection will reference this datastream as their own POLICY stream ???
   * @param type $dom
   * @param type $rootElement
   * @param type $policyStream
   * @return type 
   */
  function createChildPolicyStream($dom, $rootElement, $policyStream) {
    $ds1 = $dom->createElement("foxml:datastream");

    $rootElement->appendChild($ds1);
    $ds1->setAttribute("ID", "CHILD_SECURITY");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "X");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1->appendChild($ds1v);
    $ds1v->setAttribute("ID", "CHILD_SECURITY.0");
    $ds1v->setAttribute("MIMETYPE", "text/xml");
    $ds1v->setAttribute("LABEL", "SECURITY_POLICY");
    //$ds1v->setAttribute("FORMAT_URI","info:fedora/fedora-system:format/xml.fedora.audit");
    $content = $dom->createElement("foxml:xmlContent");
    $ds1v->appendChild($content);
    $content->appendChild($policyStream);
    return TRUE;
  }

  /**
   * Create standard fedora stuff ??????????????????
   * @param type $user
   * @param type $dom
   * @param type $rootElement 
   */
  function createStandardFedoraStuff($user, & $dom, & $rootElement) {
    /* foxml object properties section */
    $objproperties = $dom->createElement("foxml:objectProperties");
    $prop1 = $dom->createElement("foxml:property");
    $prop1->setAttribute("NAME", "http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
    $prop1->setAttribute("VALUE", "FedoraObject");
    $prop2 = $dom->createElement("foxml:property");
    $prop2->setAttribute("NAME", "info:fedora/fedora-system:def/model#state");
    $prop2->setAttribute("VALUE", "A");
    $prop3 = $dom->createElement("foxml:property");
    $prop3->setAttribute("NAME", "info:fedora/fedora-system:def/model#label");
    $prop3->setAttribute("VALUE", $user->name);
    $prop4 = $dom->createElement("foxml:property");
    $prop4->setAttribute("NAME", "info:fedora/fedora-system:def/model#contentModel");
    $prop4->setAttribute("VALUE", 'Collection');
    $prop5 = $dom->createElement("foxml:property");
    $prop5->setAttribute("NAME", "info:fedora/fedora-system:def/model#ownerId");
    $prop5->setAttribute("VALUE", $user->name);
    $objproperties->appendChild($prop1);
    $objproperties->appendChild($prop2);
    $objproperties->appendChild($prop3);
    $objproperties->appendChild($prop4);
    $objproperties->appendChild($prop5);
    $rootElement->appendChild($objproperties);
  }

  /**
   * Create DC Stream ???
   * @global type $user
   * @param type $theUser
   * @param type $dom
   * @param type $rootElement 
   */
  function createDCStream($theUser, & $dom, & $rootElement) {
    global $user;
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
    ///begin writing dc
    $oai = $dom->createElement("oai_dc:dc");
    $oai->setAttribute('xmlns:oai_dc', "http://www.openarchives.org/OAI/2.0/oai_dc/");
    $oai->setAttribute('xmlns:dc', "http://purl.org/dc/elements/1.1/");
    $oai->setAttribute('xmlns:dcterms', "http://purl.org/dc/terms/");
    $oai->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
    $content->appendChild($oai);
    //dc elements
    $element = $dom->createElement('dc:type', 'Collection');
    $oai->appendChild($element);
    $element = $dom->createElement('dc:title', $theUser->name . ' Personal Collection');
    $oai->appendChild($element);
    $element = $dom->createElement('dc:date', date(DATE_RFC822));
    $oai->appendChild($element);
    $element = $dom->createElement('dc:subject', $theUser->name);
    $oai->appendChild($element);
    $element = $dom->createElement('dc:contributor', '');
    $oai->appendChild($element);
    $element = $dom->createElement('dc:creator', $user->name);
    $oai->appendChild($element);

    $element = $dom->createElement('dc:language', '');
    $oai->appendChild($element);

    $rootElement->appendChild($datastream);
  }

}
