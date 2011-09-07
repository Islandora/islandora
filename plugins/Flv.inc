<?php

// $Id$

/**
 * @file
 * Form Builder class
 */

/**
 * implements method from content model ingest form xml
 */
class FormBuilder {

  /**
   * Constructor
   */
  function FormBuilder() {
    module_load_include('inc', 'Flv', '');
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  /**
   * Create QDC Stream ???
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createQDCStream($form_values, &$dom, &$rootElement) {
    $datastream = $dom->createElement("foxml:datastream");
    $datastream->setAttribute("ID", "QDC");
    $datastream->setAttribute("STATE", "A");
    $datastream->setAttribute("CONTROL_GROUP", "X");
    $version = $dom->createElement("foxml:datastreamVersion");
    $version->setAttribute("ID", "QDC.0");
    $version->setAttribute("MIMETYPE", "text/xml");
    $version->setAttribute("LABEL", "QDC Dublin Core Record");
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
      $index = strrpos($key, '-');
      
      if ($index > 01) {
        $key = substr($key, 0, $index);
      }

      $test = substr($key, 0, 2);

      if ($test == 'dc' || $test == 'ap') {//don't try to process other form values
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
          drupal_set_message(t("!e", array('!e' => $e->getMessage())), 'error');
          continue;
        }
      }
      $rootElement->appendChild($datastream);
    }
  }

  /**
   * Handle QDC Form ???
   * @param type $form_values
   * @return type 
   */
  function handleQDCForm($form_values) {
    $dom = new DomDocument("1.0", "UTF-8");
    $dom->formatOutput = TRUE;
    $pid = $form_values['pid'];
    $rootElement = $dom->createElement("foxml:digitalObject");
    $rootElement->setAttribute('PID', "$pid");
    $rootElement->setAttribute('xmlns:foxml', "info:fedora/fedora-system:def/foxml#");
    $rootElement->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
    $rootElement->setAttribute('xsi:schemaLocation', "info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-0.xsd");
    $dom->appendChild($rootElement);
    
    //create standard fedora stuff
    $this->createStandardFedoraStuff($form_values, $dom, $rootElement);
    
    //create relationships
    $this->createRelationShips($form_values, $dom, $rootElement);
    
    //create dublin core
    $this->createQDCStream($form_values, $dom, $rootElement);
    $this->createFedoraDataStreams($form_values, $dom, $rootElement);
    $params = array(
      'objectXML' => $dom->saveXML(),
      'format' => "foxml1.0",
      'logMessage' => "Fedora Object Ingested",
    );

    try {
      $soapHelper = new ConnectionHelper();
      $client = $soapHelper->getSoapClient(variable_get('fedora_soap_manage_url', 'http://localhost:8080/fedora/services/management?wsdl'));

      if ($client == NULL) {
        drupal_set_message(t('Error getting SOAP client.'), 'error');
        return;
      }
      $object = $client->__soapCall('ingest', array($params));
      $deleteFiles = $form_values['delete_file']; //remove files from drupal file system

      if ($deleteFiles > 0) {
        foreach ($_SESSION['fedora_ingest_files'] as $dsid => $createdFile) {
          unlink($createdFile);
        }
        unlink($form_values['fullpath']);
      }
    } catch (exception $e) {
      drupal_set_message(t('Error ingesting object: !e', array('!e' => $e->getMessage())), 'error');
      return;
    }
  }

  /**
   * Create Fedora DataStream
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createFedoraDataStreams($form_values, &$dom, &$rootElement) {
    module_load_include('inc', 'fedora_repository', 'MimeClass');
    $mimetype = new MimeClass();
    $server = NULL;
    $file = $form_values['ingest-file-location'];
    $dformat = $mimetype->getType($file);
    $fileUrl = 'http://' . $_SERVER['HTTP_HOST'] . $file;
    $beginIndex = strrpos($fileUrl, '/');
    $dtitle = substr($fileUrl, $beginIndex + 1);
    $dtitle = substr($dtitle, 0, strpos($dtitle, "."));
    $ds1 = $dom->createElement("foxml:datastream");
    $ds1->setAttribute("ID", "OBJ");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "M");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1v->setAttribute("ID", "OBJ.0");
    $ds1v->setAttribute("MIMETYPE", "$dformat");
    $ds1v->setAttribute("LABEL", "$dtitle");
    $ds1content = $dom->createElement('foxml:contentLocation');
    $ds1content->setAttribute("REF", "$fileUrl");
    $ds1content->setAttribute("TYPE", "URL");
    $ds1->appendChild($ds1v);
    $ds1v->appendChild($ds1content);
    $rootElement->appendChild($ds1);

    foreach ($_SESSION['fedora_ingest_files'] as $dsid => $createdFile) {
      $createdFile = strstr($createdFile, $file);
      $dformat = $mimetype->getType($createdFile);
      $fileUrl = 'http://' . $_SERVER['HTTP_HOST'] . $createdFile;
      $beginIndex = strrpos($fileUrl, '/');
      $dtitle = substr($fileUrl, $beginIndex + 1);
      $dtitle = substr($dtitle, 0, strpos($dtitle, "."));
      $dtitle = $dtitle . '_' . $dsid;
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

  /**
   * creates the RELS-EXT for the foxml
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createRelationShips($form_values, &$dom, &$rootElement) {
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
    $rdf->setAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    $rdf->setAttribute("xmlns:oai_dc", "http://www.openarchives.org/OAI/2.0/oai_dc/");
    $rdfdesc = $dom->createElement("rdf:description");
    $pid = $form_values['pid'];
    $rdfdesc->setAttribute("rdf:about", "info:fedora/$pid");
    $member = $dom->createElement("fedora:isMemberOfCollection");
    $membr = $form_values['collection_pid'];
    $member->setAttribute("rdf:resource", "info:fedora/$membr");

    $drdf->appendChild($dvrdf);
    $dvrdf->appendChild($dvcontent);
    $dvcontent->appendChild($rdf);
    $rdf->appendChild($rdfdesc);
    $rdfdesc->appendChild($member);
    $rootElement->appendChild($drdf);
  }

  /**
   * creates the standard foxml properties
   * @param type $form_values
   * @param type $dom
   * @param type $rootElement 
   */
  function createStandardFedoraStuff($form_values, &$dom, &$rootElement) {
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
    $prop3->setAttribute("VALUE", $form_values['dc:title']);
    $prop4 = $dom->createElement("foxml:property");
    $prop4->setAttribute("NAME", "info:fedora/fedora-system:def/model#contentModel");
    $prop4->setAttribute("VALUE", $form_values['content_model_name']);
    $prop5 = $dom->createElement("foxml:property");
    $prop5->setAttribute("NAME", "info:fedora/fedora-system:def/model#ownerId");
    $prop5->setAttribute("VALUE", $form_values['user_id']);
    $objproperties->appendChild($prop1);
    $objproperties->appendChild($prop2);
    $objproperties->appendChild($prop3);
    $objproperties->appendChild($prop4);
    $objproperties->appendChild($prop5);
    $rootElement->appendChild($objproperties);
  }

  /**
   * Build QDC Form
   * @param type $form
   * @param type $ingest_form_definition
   * @param type $form_values
   * @return type 
   */
  function buildQDCForm(&$form, $ingest_form_definition, &$form_values) {
    $form['indicator2'] = array(
      '#type' => 'fieldset',
      '#title' => t('Ingest Digital Object Step #2')
    );
    
    foreach ($ingest_form_definition->form_elements->element as $element) {
      $name = strip_tags($element->name->asXML());
      $title = strip_tags($element->label->asXML());
      $required = strip_tags($element->required->asXML());
      $required = strtolower($required);
      
      if ($required != 'TRUE') {
        $required = '0';
      }

      $description = strip_tags($element->description->asXML());
      $type = strip_tags($element->type->asXML());
      $options = array();
      
      if ($element->type == 'select') {
        foreach ($element->authoritative_list->item as $item) {
          $field = strip_tags($item->field->asXML());
          $value = strip_tags($item->value->asXML());
          $options["$field"] = $value;
        }
        $form['indicator2']["$name"] = array(
          '#title' => $title,
          '#required' => $required,
          '#description' => $description,
          '#type' => $type,
          '#options' => $options,
        );
      }
      else {
        $form['indicator2']["$name"] = array(
          '#title' => $title,
          '#required' => $required,
          '#description' => $description,
          '#type' => $type
        );
      }
    }

    return $form;
  }

}

