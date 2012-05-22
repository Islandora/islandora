<?php



/**
 * @file
 * ModsFormBuilder class
 */
module_load_include('inc', 'fedora_repository', 'plugins/FormBuilder');

/**
 * ModsFormBuilder class ???
 */
class ModsFormBuilder extends FormBuilder {

  static $MODS_NS = 'http://www.loc.gov/mods/v3';
  protected $cm;
  protected $item;
  protected $pid;

  /**
   * Constructor
   * @param type $pid 
   */
  function __construct($pid=NULL) {
    parent::__construct();
    if ($pid !== NULL) {
      module_load_include('inc', 'fedora_repository', 'ContentModel');
      module_load_include('inc', 'fedora_repository', 'api/fedora_item');

      $this->pid = $pid;
      $this->cm = ContentModel::loadFromObject($pid);
      $this->item = new fedora_item($pid);
    }
  }

  /**
   * Handle Edit Metadata Form ???
   * @param &$form_id
   * @param &$form_values
   * @param &$soap_client
   */
  function handleEditMetadataForm(&$form_id, &$form_values, &$soap_client) {
    $dom = new DomDocument("1.0", "UTF-8");
    $dom->formatOutput = TRUE;
    $mods = $this->modsFromForm($form_values, $dom);
    $dom->appendChild($mods);

    if ($this->item->modify_datastream_by_value($dom->saveXML(), 'MODS', "MODS Record", 'text/xml') !== NULL) {
      drupal_set_message(t('Successfully updated MODS datastream for object %pid', array('%pid' => $this->pid)));
    }
    drupal_goto('/fedora/repository/' . $this->pid);
  }

  /**
   * Build Edit Metadata Form
   * @return array  
   */
  function buildEditMetadataForm() {
    $form['#multistep'] = TRUE; // used so that it triggers a form rebuild every time.
    $form['indicator2'] = array(
      '#type' => 'fieldset',
      '#title' => t('Edit metadata'),
    );

    if ($this->cm !== FALSE && $this->item != NULL) {
      $form['pid'] = array('#type' => 'hidden', '#value' => $this->pid);

      $elements = $this->cm->getIngestFormElements();
      $content = $this->item->get_datastream_dissemination('MODS');

      if (trim($content) != '') {
        $dom = DOMDocument::loadXML($content);
        $xpath = new DOMXPath($dom);
        // Register the php: namespace (required)
        $xpath->registerNamespace("php", "http://php.net/xpath");

        // Register PHP functions (no restrictions)
        $xpath->registerPHPFunctions();

        foreach ($elements as $element) {

          $el = array(
            '#title' => $element['label'],
            '#required' => ($element['required'] ? 1 : 0),
            '#description' => $element['description'],
            '#type' => $element['type']
          );

          $includeEl = TRUE;
          $elname = explode('][', $element['name']);
          $elLocation = &$form['indicator2'];
          while (isset($elLocation[$name[0]]) && ($partial = array_shift($elname)) != NULL) {
            $elLocation = &$elLocation[$partial];
          }

          foreach ($element['parameters'] as $key => $val) {
            switch ($key) {
              case '#autocomplete_path':
                $val .= '/' . $form_values['storage']['collection_pid'];
                break;

              case '#exclude_from_edit_metadata':
                $includeEl = FALSE;
                break;


              case '#edit_metadata_xpath':
                $nodeList = $xpath->evaluate($val);
//  		echo $val. ' '.$nodeList->length.' ';
// 		echo $nodeList->item(0)->nodeValue.' ';
// 		echo '<br/>';

                if (is_string($nodeList)) {
                  $el['#default_value'] = $nodeList;
                }
                elseif ($nodeList->length > 1) {
                  $el['#default_value'] = array();
                  foreach ($nodeList as $node) {
                    $el['#default_value'][] = $node->nodeValue;
                  }
                }
                elseif ($nodeList->length > 0) {
                  if ($el['#type'] == 'list') {
                    $values = array();
                    for ($i = 0; $i < $nodeList->length; $i++) {
                      $values[] = $nodeList->item($i)->nodeValue;
                    }
                    $el['#default_value'] = join('; ', $values);
                  }
                  else {
                    $el['#default_value'] = $nodeList->item(0)->nodeValue;
                  }
                }
                break;
            }

            if ($key != '#sticky') {
              $el[$key] = $val;
            }
          }

          if ($element['type'] == 'people') {

            $names = $xpath->evaluate('/mods:mods/mods:name');
            $people = array();
            foreach ($names as $mname) {

              $type = $mname->getAttribute('type');
              $role = $mname->getElementsByTagName('roleTerm')->item(0)->nodeValue;

              $nameParts = $mname->getElementsByTagName('namePart');
              foreach ($nameParts as $namePart) {
                switch ($namePart->getAttribute('type')) {
                  case 'given': $given = $namePart->nodeValue;
                    break;
                  case 'family': $family = $namePart->nodeValue;
                    break;
                  case 'termsOfAddress': $title = $namePart->nodeValue;
                    break;
                  case 'date': $date = $namePart->nodeValue;
                    break;
                  default: $name = $namePart->nodeValue;
                    break;
                }
              }

              $person = array('role' => $role);
              switch ($type) {
                case 'personal':
                  if (isset($given) && isset($family) && !isset($name)) {
                    $name = (isset($title) ? $title . ' ' : '') . $family . ', ' . $family;
                  }
                  $person['name'] = $name;
                  $person['date'] = $date;
                  break;
                case 'organization':
                  $person['organization'] = $name;
                  break;
                case 'conference':
                  $person['conference'] = $name;
                  $person['date'] = $date;
                  break;
              }
              $people[] = $person;
            }

            $names = $xpath->evaluate('/mods:mods/mods:subject/mods:name');
            foreach ($names as $mname) {

              $type = $mname->getAttribute('type');

              $nameParts = $mname->getElementsByTagName('namePart');
              foreach ($nameParts as $namePart) {
                switch ($namePart->getAttribute('type')) {
                  case 'given': $given = $namePart->nodeValue;
                    break;
                  case 'family': $family = $namePart->nodeValue;
                    break;
                  case 'termsOfAddress': $title = $namePart->nodeValue;
                    break;
                  case 'date': $date = $namePart->nodeValue;
                    break;
                  default: $name = $namePart->nodeValue;
                    break;
                }
              }

              $person = array('subject' => 1);
              switch ($type) {
                case 'personal':
                  if (isset($given) && isset($family) && !isset($name)) {
                    $name = (isset($title) ? $title . ' ' : '') . $family . ', ' . $family;
                  }
                  $person['name'] = $name;
                  $person['date'] = $date;
                  break;
                case 'organization':
                  $person['organization'] = $name;
                  break;
                case 'conference':
                  $person['conference'] = $name;
                  $person['date'] = $date;
                  break;
              }
              $people[] = $person;
            }

            $el['#default_value'] = $people;
          }


          if ($element['type'] == 'select' || $element['type'] == 'other_select') {
            $el['#options'] = isset($element['authoritative_list']) ? $element['authoritative_list'] : array();
          }

          if ($includeEl) {
            $elLocation[join('][', $elname)] = $el;
          }
        }

        $form['submit'] = array(
          '#type' => 'submit',
          '#submit' => array('fedora_repository_edit_qdc_form_submit'),
          '#value' => 'Save Metadata'
        );

        return $form;
      }
    }
  }

  /**
   * Handle Mods Form
   * @param &$form_values
   * @param &$form_state
   */
  function handleModsForm(&$form_values, &$form_state) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'CollectionPolicy');

    $form_state['storage']['people'] = NULL; //clears out old entities for the next run of the formbuilder.

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
    $form_values['dc:title'] = $form_values['mods_title'];
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
    // Create MODS
    $this->createModsStream($form_values, $dom, $rootElement);
    $this->createCollectionPolicy($form_values, $dom, $rootElement);
    $this->createWorkflowStream($form_values, $dom, $rootElement);

    if (!empty($form_values['ingest-file-location'])) {
      $this->createFedoraDataStreams($form_values, $dom, $rootElement);
    }
    $this->createPolicy($collectionPid, &$dom, &$rootElement);

//         header('Content-type: application/xml');
//         echo $dom->saveXML();  exit();

    try {
      $object = Fedora_Item::ingest_from_FOXML($dom);
      //for some reason, ingest_from_FOXML does not generate a JMS message
      //I just modify the workflow DS and it sends a JMS message.
      $item = new Fedora_Item($object->pid);
      $item->modify_datastream_by_value($item->get_datastream_dissemination('WORKFLOW'), 'WORKFLOW', "Workflow Record", 'text/xml');

      if (!empty($object->pid)) {
        drupal_set_message(t("Item !pid created successfully.", array('!pid' => l($object->pid, 'fedora/repository/' . $object->pid))), "status");
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
   * Create Collection Policy ??
   * @param $form_values
   * @param &$dom
   * @param &$rootElement
   */
  function createCollectionPolicy($form_values, &$dom, &$rootElement) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $model = new fedora_item($form_values['content_model_pid']);
    $ds_list = $model->get_datastreams_list_as_array();
    if (isset($ds_list['COLLECTION_POLICY_TMPL'])) {
      $cp = $model->get_datastream_dissemination('COLLECTION_POLICY_TMPL');
      $cpDom = DOMDocument::loadXML($cp);
      $cpRootEl = $cpDom->getElementsByTagName('collection_policy');
      if ($cpRootEl->length > 0) {
        $cpRootEl = $cpRootEl->item(0);
        $newNode = $dom->importNode($cpRootEl, TRUE);

        $datastream = $dom->createElement("foxml:datastream");
        $datastream->setAttribute("ID", "COLLECTION_POLICY");
        $datastream->setAttribute("STATE", "A");
        $datastream->setAttribute("CONTROL_GROUP", "X");
        $version = $dom->createElement("foxml:datastreamVersion");
        $version->setAttribute("ID", "COLLECTION_POLICY.0");
        $version->setAttribute("MIMETYPE", "text/xml");
        $version->setAttribute("LABEL", "Collection Policy");
        $datastream->appendChild($version);
        $content = $dom->createElement("foxml:xmlContent");
        $version->appendChild($content);
        $content->appendChild($newNode);
        $rootElement->appendChild($datastream);
      }
    }
  }

  /**
   * Create Workflow Stream ??
   * @param $form_values
   * @param &$dom
   * @param &$rootElement
   */
  function createWorkflowStream($form_values, &$dom, &$rootElement) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $model = new fedora_item($form_values['content_model_pid']);
    $ds_list = $model->get_datastreams_list_as_array();
    if (isset($ds_list['WORKFLOW_TMPL'])) {
      $workflow = $model->get_datastream_dissemination('WORKFLOW_TMPL');
      $workflowDom = DOMDocument::loadXML($workflow);
      $workflowRootEl = $workflowDom->getElementsByTagName('workflow');
      if ($workflowRootEl->length > 0) {
        $workflowRootEl = $workflowRootEl->item(0);
        $newNode = $dom->importNode($workflowRootEl, TRUE);

        $datastream = $dom->createElement("foxml:datastream");
        $datastream->setAttribute("ID", "WORKFLOW");
        $datastream->setAttribute("STATE", "A");
        $datastream->setAttribute("CONTROL_GROUP", "X");
        $version = $dom->createElement("foxml:datastreamVersion");
        $version->setAttribute("ID", "WORKFLOW.0");
        $version->setAttribute("MIMETYPE", "text/xml");
        $version->setAttribute("LABEL", "Workflow Record");
        $datastream->appendChild($version);
        $content = $dom->createElement("foxml:xmlContent");
        $version->appendChild($content);
        $content->appendChild($newNode);
        $rootElement->appendChild($datastream);
      }
    }
  }

  /**
   * Create mods Stream ??
   * @param $form_values
   * @param &$dom
   * @param &$rootElement
   */
  function createModsStream($form_values, &$dom, &$rootElement) {

    $datastream = $dom->createElement("foxml:datastream");
    $datastream->setAttribute("ID", "MODS");
    $datastream->setAttribute("STATE", "A");
    $datastream->setAttribute("CONTROL_GROUP", "X");
    $version = $dom->createElement("foxml:datastreamVersion");
    $version->setAttribute("ID", "MODS.0");
    $version->setAttribute("MIMETYPE", "text/xml");
    $version->setAttribute("LABEL", "MODS Record");
    $datastream->appendChild($version);
    $content = $dom->createElement("foxml:xmlContent");
    $version->appendChild($content);

    $mods = $this->modsFromForm($form_values, $dom);
    $content->appendChild($mods);

    $rootElement->appendChild($datastream);
  }

  /**
   * Mods From From ?????
   * @param type $form_values
   * @param type $dom
   * @return type 
   */
  function modsFromForm(&$form_values, &$dom) {

    ///begin writing MODS
    $mods = $dom->createElement("mods:mods");
    $mods->setAttribute('version', '3.4');
    $mods->setAttribute('xmlns:xlink', "http://www.w3.org/1999/xlink");
    $mods->setAttribute('xmlns:mods', "http://www.loc.gov/mods/v3");
    $mods->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
    $mods->setAttribute('xsi:schemaLocation', "http://www.loc.gov/mods/v3 http://www.loc.gov/standards/mods/v3/mods-3-0.xsd");


    if (isset($form_values['mods_title']) && trim($form_values['mods_title']) != '') {
      $titleinfo = $dom->createElement('mods:titleInfo');
      $title = $dom->createElement('mods:title', htmlspecialchars($form_values['mods_title']));
      $titleinfo->appendChild($title);
      $mods->appendChild($titleinfo);
    }

    if (isset($form_values['mods_alternative_titles']) && trim($form_values['mods_alternative_titles']) != '') {
      $titles = preg_split('/\s+\;\s+/', trim($form_values['mods_alternative_titles']));
      foreach ($titles as $t) {
        $titleinfo = $dom->createElement('mods:titleInfo');
        $titleinfo->setAttribute('type', 'alternative');
        $title = $dom->createElement('mods:title', $t);
        $titleinfo->appendChild($title);
        $mods->appendChild($titleinfo);
      }
    }

    if (isset($form_values['mods_alternative_title']) && trim($form_values['mods_alternative_title']) != '') {
      $titleinfo = $dom->createElement('mods:titleInfo');
      $titleinfo->setAttribute('type', 'alternative');
      $title = $dom->createElement('mods:title', trim($form_values['mods_alternative_title']));
      $titleinfo->appendChild($title);
      $mods->appendChild($titleinfo);
    }

    if (isset($form_values['mods_description']) && trim($form_values['mods_description']) != '') {
      $abstract = $dom->createElement('mods:abstract', htmlspecialchars(trim($form_values['mods_description'])));
      $mods->appendChild($abstract);
    }

    if (isset($form_values['pid']) && trim($form_values['pid']) != '') {
      $identifier = $dom->createElement('mods:identifier', htmlspecialchars(trim(preg_replace('/\:/', '\/', $form_values['pid']))));
      $identifier->setAttribute('type', 'hdl');
      $mods->appendChild($identifier);
    }

    if (isset($form_values['collection_pid']) && trim($form_values['collection_pid']) != '') {
      $relatedItem = $dom->createElement('mods:relatedItem');
      $relatedItem->setAttribute('type', 'isMemberOfCollection');
      $identifier = $dom->createElement('mods:identifier', htmlspecialchars(trim($form_values['collection_pid'])));
      $relatedItem->appendChild($identifier);
      $mods->appendChild($relatedItem);
    }

    if (isset($form_values['mods_identifier']) && trim($form_values['mods_identifier']) != '') {
      $identifier = $dom->createElement('mods:identifier', htmlspecialchars(trim($form_values['mods_identifier'])));
      $identifier->setAttribute('type', 'local');
      $mods->appendChild($identifier);
    }

    if (isset($form_values['mods_physicalLocation']) && trim($form_values['mods_physicalLocation']) != '') {
      $location = $dom->createElement('mods:location');
      $physLocation = $dom->createElement('mods:physicalLocation', htmlspecialchars(trim($form_values['mods_physicalLocation'])));
      $location->appendChild($physLocation);
      if (isset($form_values['mods_shelfLocator']) && trim($form_values['mods_shelfLocator']) != '') {
        $shelfLocator = $dom->createElement('mods:shelfLocator', htmlspecialchars(trim($form_values['mods_shelfLocator'])));
        $location->appendChild($shelfLocator);
      }
      $mods->appendChild($location);
    }

    $originInfo = $dom->createElement('mods:originInfo');
    $addOriginInfo = FALSE;
    if (isset($form_values['mods_pubinfo_place']) && trim($form_values['mods_pubinfo_place']) != '') {
      $place = $dom->createElement('mods:place');
      $placeTerm = $dom->createElement('mods:placeTerm', htmlspecialchars(trim($form_values['mods_pubinfo_place'])));
      $placeTerm->setAttribute('type', 'text');
      $place->appendChild($placeTerm);
      $originInfo->appendChild($place);
      $addOriginInfo = TRUE;
    }

    if (isset($form_values['mods_pubinfo_publisher']) && trim($form_values['mods_pubinfo_publisher']) != '') {
      $publisher = $dom->createElement('mods:publisher', htmlspecialchars(trim($form_values['mods_pubinfo_publisher'])));
      $originInfo->appendChild($publisher);
      $addOriginInfo = TRUE;
    }

    if (isset($form_values['mods_pubinfo_edition']) && trim($form_values['mods_pubinfo_edition']) != '') {
      $edition = $dom->createElement('mods:edition', htmlspecialchars(trim($form_values['mods_pubinfo_edition'])));
      $originInfo->appendChild($edition);
      $addOriginInfo = TRUE;
    }


    if (isset($form_values['mods_pubinfo_date']) && trim($form_values['mods_pubinfo_date']) != '' &&
      isset($form_values['mods_pubinfo_dateType']) && trim($form_values['mods_pubinfo_dateType']) != '') {
      if (in_array($form_values['mods_pubinfo_dateType'], array('issued', 'created', 'copyright', 'captured'))) {
        $date = $dom->createElement('mods:' . trim($form_values['mods_pubinfo_dateType']) . 'Date', htmlspecialchars(trim($form_values['mods_pubinfo_date'])));
      }
      else {
        //how to handle other types?  otherDate?
        $date = $dom->createElement('mods:otherDate', htmlspecialchars(trim($form_values['mods_pubinfo_date'])));
        $date->setAttribute('type', htmlspecialchars(trim($form_values['mods_pubinfo_dateType'])));
      }
      $originInfo->appendChild($date);
      $addOriginInfo = TRUE;
    }
    else {
      if (isset($form_values['mods_createdDate'])) {
        $date = $dom->createElement('mods:createdDate', htmlspecialchars(trim($form_values['mods_createdDate'])));
        $originInfo->appendChild($date);
        $addOriginInfo = TRUE;
      }

      if (isset($form_values['mods_issuedDate'])) {
        $date = $dom->createElement('mods:issuedDate', htmlspecialchars(trim($form_values['mods_issuedDate'])));
        $originInfo->appendChild($date);
        $addOriginInfo = TRUE;
      }

      if (isset($form_values['mods_copyrightDate'])) {
        $date = $dom->createElement('mods:copyrightDate', htmlspecialchars(trim($form_values['mods_copyrightDate'])));
        $originInfo->appendChild($date);
        $addOriginInfo = TRUE;
      }

      if (isset($form_values['mods_capturedDate'])) {
        $date = $dom->createElement('mods:capturedDate', htmlspecialchars(trim($form_values['mods_capturedDate'])));
        $originInfo->appendChild($date);
        $addOriginInfo = TRUE;
      }
    }

    if (isset($form_values['mods_pubinfo_journalFreq']) && trim($form_values['mods_pubinfo_journalFreq']) != '') {
      $frequency = $dom->createElement('mods:frequency', htmlspecialchars(trim($form_values['mods_pubinfo_journalFreq'])));
      $originInfo->appendChild($frequency);
      $issuance = $dom->createElement('mods:issuance', 'journal');
      $originInfo->appendChild($issuance);
      $addOriginInfo = TRUE;
    }
    elseif (isset($form_values['mods_pubinfo_journalFreq'])) {
      $issuance = $dom->createElement('mods:issuance', 'monographic');
      $originInfo->appendChild($issuance);
    }


    if ($addOriginInfo) {
      $mods->appendChild($originInfo);
    }

    if (isset($form_values['mods_note']) && trim($form_values['mods_note']) != '') {
      $note = $dom->createElement('mods:note', htmlspecialchars(trim($form_values['mods_note'])));
      $mods->appendChild($note);
    }

    if (isset($form_values['mods_caption']) && trim($form_values['mods_caption']) != '') {
      $note = $dom->createElement('mods:note', htmlspecialchars(trim($form_values['mods_caption'])));
      $note->setAttribute('type', 'caption');
      $mods->appendChild($note);
    }

    if (isset($form_values['mods_format']) && trim($form_values['mods_format']) != '') {
      $typeOfResource = $dom->createElement('mods:typeOfResource', htmlspecialchars($form_values['mods_format']));
      $mods->appendChild($typeOfResource);
    }


    if (isset($form_values['mods_language']) && trim($form_values['mods_language']) != '') {
      $languageList = explode(';', htmlspecialchars($form_values['mods_language']));
      foreach ($languageList as $lang) {
        $language = $dom->createElement('mods:language');
        $langTerm = $dom->createElement('mods:languageTerm', htmlspecialchars($lang));
        $langTerm->setAttribute('type', 'text');
        $language->appendChild($langTerm);
        $mods->appendChild($language);
      }
    }


    $hasSubject = FALSE;
    $subject = $dom->createElement('mods:subject');


    // Hierarchical Geographic Subject
    if (isset($form_values['mods_country']) && trim($form_values['mods_country']) != '') {
      $hasSubject = TRUE;
      $geographic = $dom->createElement('mods:hierarchicalGeographic');

      $country = $dom->createElement('mods:country', htmlspecialchars($form_values['mods_country']));
      $geographic->appendChild($country);

      if (isset($form_values['mods_province']) && trim($form_values['mods_province']) != '') {
        $province = $dom->createElement('mods:province', htmlspecialchars($form_values['mods_province']));
        $geographic->appendChild($province);
      }

      if (isset($form_values['mods_state']) && trim($form_values['mods_state']) != '') {
        $state = $dom->createElement('mods:state', htmlspecialchars($form_values['mods_state']));
        $geographic->appendChild($state);
      }

      if (isset($form_values['mods_city']) && trim($form_values['mods_city']) != '') {
        $city = $dom->createElement('mods:city', htmlspecialchars($form_values['mods_city']));
        $geographic->appendChild($city);
      }

      if (isset($form_values['mods_area']) && trim($form_values['mods_area']) != '') {
        $state = $dom->createElement('mods:area', htmlspecialchars($form_values['mods_area']));
        $geographic->appendChild($state);
      }


      $subject->appendChild($geographic);
    }

    if (isset($form_values['mods_date']) && trim($form_values['mods_date']) != '') {
      $hasSubject = TRUE;
      $temporal = $dom->createElement('mods:temporal', htmlspecialchars($form_values['mods_date']));
      $subject->appendChild($temporal);
    }

    if (isset($form_values['mods_subjtitle']) && trim($form_values['mods_subjtitle']) != '') {
      $hasSubject = TRUE;
      $titleInfo = $dom->createElement('mods:titleInfo');
      $title = $dom->createElement('mods:title', htmlspecialchars($form_values['mods_subjtitle']));
      $titleInfo->appendChild($title);
      $subject->appendChild($titleInfo);
    }


    if (isset($form_values['mods_topics']) && trim($form_values['mods_topics']) != '') {
      $hasSubject = TRUE;
      $topicList = explode(';', htmlspecialchars($form_values['mods_topics']));
      $authority = 'unknown';
      if (isset($form_values['mods_topicAuthority']) && trim($form_values['mods_topicAuthority']) != '') {
        $authority = htmlspecialchars($form_values['mods_topicAuthority']);
      }

      foreach ($topicList as $t) {
        $topic = $dom->createElement('mods:topic', $t);
        $topic->setAttribute('authority', $authority);
        $subject->appendChild($topic);
      }
    }


    if (isset($form_values['mods_cc']['cc']) && $form_values['mods_cc']['cc']['cc_enable']) {

      $commercial = trim($form_values['mods_cc']['cc']['cc_commercial']);
      $modifications = trim($form_values['mods_cc']['cc']['cc_modifications']);
      $jurisdiction = trim($form_values['mods_cc']['cc']['cc_jurisdiction']);

      module_load_include('inc', 'islandora_form_elements', 'includes/creative_commons.inc');

      if (!isset(CreativeCommons::$cc_jurisdiction_vals[$jurisdiction]))
        $jurisdiction = '';
      $version = CreativeCommons::$cc_versions[$jurisdiction];

      $license = 'by' . ($commercial != '' ? '-' . $commercial : '') . ($modifications != '' ? '-' . $modifications : '') . '/' . $version . '/' . ($jurisdiction != '' ? $jurisdiction . '/' : '');

      $accessCondition = $dom->createElement('mods:accessCondition', htmlspecialchars($license));
      $accessCondition->setAttribute('type', 'Creative Commons License');
      $mods->appendChild($accessCondition);
    }

    if (isset($form_values['mods_rights']) && trim($form_values['mods_rights']) != '') {
      $accessCondition = $dom->createElement('mods:accessCondition', htmlspecialchars($form_values['mods_rights']));
      $accessCondition->setAttribute('type', 'restriction on access; use and reproduction');
      $mods->appendChild($accessCondition);
    }

    if (isset($form_values['mods_people']) && isset($form_values['mods_people']['people']) && is_array($form_values['mods_people']['people'])) {
      foreach ($form_values['mods_people']['people'] as $key => $val) {
        $name = $dom->createElement('mods:name');
        $appendName = FALSE;
        if (isset($val['role'])) {
          $role = $dom->createElement('mods:role');
          $roleTerm = $dom->createElement('mods:roleTerm', htmlspecialchars(trim($val['role'])));
          $roleTerm->setAttribute('type', 'text');
          $roleTerm->setAttribute('authority', 'marcrelator');
          $role->appendChild($roleTerm);
          $name->appendChild($role);
        }

        if (isset($val['organization'])) {
          $name->setAttribute('type', 'organization');
          if (trim($val['organization']) != '') {
            $namePart = $dom->createElement('mods:namePart', htmlspecialchars(trim($val['organization'])));
            $name->appendChild($namePart);
            $appendName = TRUE;
          }
        }
        elseif (isset($val['conference'])) {
          $name->setAttribute('type', 'conference');
          if (trim($val['conference']) != '') {
            $namePart = $dom->createElement('mods:namePart', htmlspecialchars(trim($val['conference'])));
            $name->appendChild($namePart);
            $appendName = TRUE;
          }
        }
        else {
          $name->setAttribute('type', 'personal');
          if (trim($val['name']) != '') {
            $namePart = $dom->createElement('mods:namePart', htmlspecialchars(trim($val['name'])));
            $name->appendChild($namePart);
            $appendName = TRUE;
          }
        }

        if (isset($val['date'])) {
          $namePart = $dom->createElement('mods:namePart', htmlspecialchars(trim($val['date'])));
          $namePart->setAttribute('type', 'date');
          $name->appendChild($namePart);
        }

        if ($appendName) {
          if (isset($val['subject'])) {
            $subject->appendChild($name);
            $hasSubject = TRUE;
          }
          else {
            $mods->appendChild($name);
          }
        }
      }
    }

    if ($hasSubject) {
      $mods->appendChild($subject);
    }

    return $mods;
  }

}
