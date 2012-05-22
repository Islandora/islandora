<?php

/* @file



*/

/**
 * @file
 * Darwin Core class
 */

/**
 * Darwin Core ???
 */
class DarwinCore {

  /**
   * Constructor
   * @param type $item 
   */
  function __construct($item = NULL) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($item)) {
      $this->owner = $item;
      if (array_key_exists('DARWIN_CORE', $item->get_datastreams_list_as_array())) {
        $dwc = $item->get_datastream_dissemination('DARWIN_CORE');
        if (!empty($dwc)) {
          $this->darwinCoreXML = $dwc;
        }
      }
    }
  }

  /**
   * Build Drupal Form
   * @param type $form
   * @return int 
   */
  public function buildDrupalForm($form = array()) {

    $dwc_xml = $this->darwinCoreXML;

    $dwc = DOMDocument::loadXML($dwc_xml);

    $form['dc:type'] = array(
      '#title' => 'dc:type',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://purl.org/dc/terms/', 'type')->item(0)->nodeValue,
      '#description' => '',
    );

    $form['dc:language'] = array(
      '#title' => 'dc:language',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://purl.org/dc/terms/', 'language')->item(0)->nodeValue,
      '#description' => '',
    );

    $form['dwc:basisOfRecord'] = array(
      '#title' => 'Basis of Record',
      '#type' => 'select',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'basisOfRecord')->item(0)->nodeValue,
      '#options' => $this->vocabulary['basisOfRecord'],
      '#description' => '',
    );
    $form['dwc:scientificName'] = array(
      '#title' => 'dwc:scientificName',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'scientificName')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:specificEpithet'] = array(
      '#title' => 'dwc:specificEpithet',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'specificEpithet')->item(0)->nodeValue,
      '#description' => '',
    );

    $form['dwc:family'] = array(
      '#title' => 'dwc:family',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'family')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:genus'] = array(
      '#title' => 'dwc:genus',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'genus')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:vernacularName'] = array(
      '#title' => 'dwc:vernacularName',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'vernacularName')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:continent'] = array(
      '#title' => 'dwc:continent',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'continent')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:country'] = array(
      '#title' => 'dwc:country',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'country')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:stateProvince'] = array(
      '#title' => 'dwc:stateProvince',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'stateProvince')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:county'] = array(
      '#title' => 'dwc:county',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'county')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:locality'] = array(
      '#title' => 'dwc:locality',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'locality')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:habitat'] = array(
      '#title' => 'dwc:habitat',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'habitat')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:occurrenceRemarks'] = array(
      '#title' => 'dwc:occurrenceRemarks',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'occurrenceRemarks')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:occurrenceID'] = array(
      '#title' => 'dwc:occurrenceID',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'occurrenceID')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:institutionCode'] = array(
      '#title' => 'dwc:institutionCode',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'institutionCode')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:collectionCode'] = array(
      '#title' => 'dwc:collectionCode',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'collectionCode')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:collectionCode'] = array(
      '#title' => 'dwc:collectionCode',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'collectionCode')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:catalogNumber'] = array(
      '#title' => 'dwc:catalogNumber',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'catalogNumber')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:catalogNumber'] = array(
      '#title' => 'dwc:catalogNumber',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'catalogNumber')->item(0)->nodeValue,
      '#description' => '',
    );
    $form['dwc:recordedBy'] = array(
      '#title' => 'dwc:recordedBy',
      '#type' => 'textfield',
      '#default_value' => $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'recordedBy')->item(0)->nodeValue,
      '#description' => '',
    );
    $date = $dwc->getElementsByTagNameNS('http://rs.tdwg.org/dwc/terms/', 'eventDate')->item(0)->nodeValue;
    $format = 'Y-m-d H:i:s';
    $form['dwceventDate'] = array(
      '#type' => 'date_popup', // types 'date_text' and 'date_timezone' are also supported. See .inc file.
      '#title' => 'select a date',
      '#default_value' => $date,
      '#date_format' => $format,
      '#date_label_position' => 'within', // See other available attributes and what they do in date_api_elements.inc
      '#date_increment' => 15, // Optional, used by the date_select and date_popup elements to increment minutes and seconds.
      '#description' => '',
    );
    return $form;
  }

  /**
   * Handle Form ??
   * @global type $user
   * @param type $form_values 
   */
  public function handleForm($form_values) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    global $user;
    //$new_item = Fedora_Item::ingest_new_item($form_values['pid'], 'A', $form_values['dwc:institutionCode'].':'.$form_values['dwc:collectionCode'].':'.$form_values['dwc:catalogNumber'], $user->name);
    //$new_item->add_datastream_from_string($this->darwinCoreXML, 'DARWIN_CORE', 'Darwin Core Metadata', 'text/xml', 'X');
    // For each form element, find and replace that element's value in the XML stream.
    $dwc = new SimpleXMLElement($this->darwinCoreXML);
    $dc_elements = $dwc->SimpleDarwinRecord->children('http://purl.org/dc/terms/');
    $dc_elements->type = $form_values['dc:type'];
    $dc_elements->language = $form_values['dc:language'];

    $dwc_elements = $dwc->SimpleDarwinRecord->children('http://rs.tdwg.org/dwc/terms/');
    $dwc_elements->basisOfRecord = $form_values['dwc:basisOfRecord'];
    $dwc_elements->scientificName = $form_values['dwc:scientificName'];
    $dwc_elements->specificName = $form_values['dwc:specificName'];
    $dwc_elements->specificName = $form_values['dwc:family'];
    $dwc_elements->genus = $form_values['dwc:genus'];
    $dwc_elements->vernacularName = $form_values['dwc:vernacularName'];
    $dwc_elements->continent = $form_values['dwc:continent'];
    $dwc_elements->country = $form_values['dwc:country'];
    $dwc_elements->countryCode = $form_values['dwc:countryCode'];
    $dwc_elements->stateProvince = $form_values['dwc:stateProvince'];
    $dwc_elements->locality = $form_values['dwc:locality'];
    $dwc_elements->habitat = $form_values['dwc:habitat'];
    $dwc_elements->occurrenceRemarks = $form_values['dwc:occurrenceRemarks'];
    $dwc_elements->occurrenceID = $form_values['dwc:occurrenceID'];
    $dwc_elements->institutionCode = $form_values['dwc:institutionCode'];
    $dwc_elements->collectionCode = $form_values['dwc:collectionCode'];
    $dwc_elements->catalogNumber = $form_values['dwc:catalogNumber'];
    $dwc_elements->recordedBy = $form_values['dwc:recordedBy'];
    $dwc_elements->eventDate = $form_values['dwceventDate'];

    $this->darwinCoreXML = $dwc->saveXML();
  }

  /**
   * asXML ??
   * @return type 
   */
  public function asXML() {
    return $this->darwinCoreXML;
  }

  /**
   * asHTML ??
   * @return type 
   */
  public function asHTML() {
    $path = drupal_get_path('module', 'fedora_repository');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'CollectionClass');

    $xmlstr = $this->darwinCoreXML;
    html_entity_decode($xmlstr);

    if ($xmlstr == NULL || strlen($xmlstr) < 5) {
      return " ";
    }

    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('@e', array('@e' => check_plain($e->getMessage()))), 'error');
      return " ";
    }

    $xsl = new DomDocument();
    $xsl->load($path . '/xsl/specdwc.xsl');
    $input = new DomDocument();
    $input->loadXML(trim($xmlstr));
    $xsl = $proc->importStylesheet($xsl);
    $newdom = $proc->transformToDoc($input);
    $content = $newdom->saveXML();

    return $content;
  }

  public $vocabulary = array(
    'basisOfRecord' => array(
      'Occurrence' => 'Occurrence',
      'Event' => 'Event',
      'Location' => 'Location',
      'Taxon' => 'Taxon',
      'PreservedSpecimen' => 'PreservedSpecimen',
      'FossilSpecimen' => 'FossilSpecimen',
      'LivingSpecimen' => 'LivingSpecimen',
      'HumanObservation' => 'HumanObservation',
      'MachineObservation' => 'MachineObservation',
      'NomenclaturalChecklist' => 'NomenclaturalChecklist',
    ),
  );
  public $dwcFields = array(
    'dc:type',
    'dc:language',
    'dwc:basisofrecord',
    'dwc:scientificName',
    'dwc:specificName',
    'dwc:family',
    'dwc:genus',
    'dwc:vernacularName',
    'dwc:continent',
    'dwc:country',
    'dwc:countryCode',
    'dwc:stateProvince',
    'dwc:locality',
    'dwc:habitat',
    'dwc:occurrenceRemarks',
    'dwc:occurrenceID',
    'dwc:institutionCode',
    'dwc:collectionCode',
    'dwc:catalogNumber',
    'dwc:recordedBy',
    'dwc:eventDate',
    'dwc:eventTime',
  );
  public $darwinCoreXML = '
<SimpleDarwinRecordSet xmlns="http://rs.tdwg.org/dwc/xsd/simpledarwincore/" xmlns:dc="http://purl.org/dc/terms/" xmlns:dwc="http://rs.tdwg.org/dwc/terms/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://rs.tdwg.org/dwc/xsd/simpledarwincore/ http://rs.tdwg.org/dwc/xsd/tdwg_dwc_simple.xsd">
  <SimpleDarwinRecord>
    <dc:type></dc:type>
    <dc:language>en</dc:language>
    <dwc:basisOfRecord></dwc:basisOfRecord>
    <dwc:scientificName></dwc:scientificName>
    <dwc:specificEpithet></dwc:specificEpithet>
    <dwc:family></dwc:family>
    <dwc:genus></dwc:genus>
    <dwc:vernacularName></dwc:vernacularName>
    <dwc:continent></dwc:continent>
    <dwc:country></dwc:country>
    <dwc:countryCode></dwc:countryCode>
    <dwc:stateProvince></dwc:stateProvince>
    <dwc:county></dwc:county>
    <dwc:locality></dwc:locality>
    <dwc:habitat></dwc:habitat>
    <dwc:occurrenceRemarks></dwc:occurrenceRemarks>
    <dwc:occurrenceID></dwc:occurrenceID>
    <dwc:institutionCode></dwc:institutionCode>
    <dwc:collectionCode></dwc:collectionCode>
    <dwc:catalogNumber></dwc:catalogNumber>
    <dwc:recordedBy></dwc:recordedBy>
    <dwc:eventDate></dwc:eventDate>
    <dwc:eventTime></dwc:eventTime>
  </SimpleDarwinRecord>
</SimpleDarwinRecordSet>';

}
