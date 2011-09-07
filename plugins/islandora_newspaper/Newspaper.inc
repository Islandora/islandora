<?php

/* @file
 * News Paper Plugin for Fedora Repository module.
 */

class Newspaper {

  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {

      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  public function buildDrupalForm($form = array(), $ingest_form = array(), &$form_state = array()) {

    // Give the user an option to enter a custom PID
    $form['custom_pid'] = array(
      '#type' => 'textfield',
      '#title' => 'Custom PID',
      '#description' => 'If you want to manually specify the PID for the new object, enter it here. ' .
      'Leave it blank for an automatically-generated PID.',
    );

    $form['mods'] = array(
      '#tree' => TRUE,
      '#prefix' => '<div id="mods-wrapper">',
      '#suffix' => '</div>',
    );
    $form['mods']['mods_record'] = array(
      '#type' => 'textarea',
      '#title' => 'MODS Record to Import',
      '#rows' => 20,
    );

    return $form;
  }

  public function handleIngestForm($form_values, &$form_state) {
    /*
     * process the metadata form
     * Create fedora object
     * Add the datastreams
     */
    module_load_include('inc', 'fedora_repository', 'MimeClass');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_ilives', 'book');
    $mods_list_doc = new DomDocument();
    $mods_list_doc->loadXML($form_values['mods']['mods_record']);
    $mods_item_doc = new DomDocument();
    $mods_item = $mods_list_doc->getElementsByTagNameNS('http://www.loc.gov/mods/v3', 'mods')->item(0);
    $new_mods_item = $mods_item_doc->importNode($mods_item, TRUE);
    $mods_item_doc->appendChild($new_mods_item);

    $title_info = $mods_item_doc->getElementsByTagNameNS('http://www.loc.gov/mods/v3', 'titleInfo')->item(0);
    $title = '';
    foreach (array('nonSort', 'title') as $title_field) {
      $title .= $title_info->getElementsByTagNameNS('http://www.loc.gov/mods/v3', $title_field)->item(0)->nodeValue;
    }

    $mods_text = $mods_item_doc->saveXML();
    global $user;
    $mimetype = new MimeClass();

    $new_item = Fedora_Item::ingest_new_item(!empty($form_values['custom_pid']) ? $form_values['custom_pid'] : $form_values['pid'], 'A', $title,
        $user->name);

    $new_item->add_datastream_from_string($mods_text, 'MODS',
      'MODS Metadata', 'text/xml', 'X');

    $dc = transform_mods_to_dc($mods_text);
    if ($dc) {
      // Add the PID to a dc:identifier field.
      $dc_doc = simplexml_load_string($dc);
      $dc_doc->registerXPathNamespace('oai_dc', 'http://www.openarchives.org/OAI/2.0/oai_dc/');
      $dc_item = $dc_doc->xpath('//oai_dc:dc');
      foreach ($dc_item as $node) {
        $node->addChild('dc:identifier', $new_item->pid, 'http://purl.org/dc/elements/1.1/');
      }
      $new_item->modify_datastream_by_value($dc_doc->saveXML(), 'DC', 'Dublin Core XML Metadata', 'text/xml');
    }
    $new_item->add_relationship('hasModel', $form_values['content_model_pid'], FEDORA_MODEL_URI);
    $new_item->add_relationship(!empty($form_values['relationship']) ? $form_values['relationship'] : 'isMemberOfCollection', $form_values['collection_pid']);
    drupal_set_message(t("Item !pid created successfully.", array('!pid' => l($new_item->pid, 'fedora/repository/' . $new_item->pid))), "status");
  }

  public function showFieldSets() {
    module_load_include('inc', 'fedora_ilives', 'book');
    global $base_url;

    $tabset = array();

    $tabset = array(
      '#type' => 'tabset',
    );

    global $user;
    $qs = '';
    if ($user->uid != 0) {
      $qs = '?uid=' . base64_encode($user->name . ':' . $user->pass);
    }

    $viewer_url = variable_get('fedora_base_url', '') . '/get/' . $this->pid . '/ilives:viewerSdef/getViewer' . $qs;
    $html = '<iframe src="' . $viewer_url . '"  scrolling="no" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';

    $tabset['read'] = array(
      '#type' => 'tabpage',
      '#title' => t('Read'),
      '#weight' => -1,
      '#content' => $html
    );

    $item = new Fedora_Item($this->pid);

    $tabset['description'] = array(
      '#type' => 'tabpage',
      '#title' => 'Description',
      '#content' => $item->get_dissemination('islandora:mods2htmlSdef', 'mods2html'),
    );
    $tabset['add_pages'] = array(
      '#type' => 'tabpage',
      '#title' => t('Add pages'),
      '#content' => drupal_get_form('book_add_pages_form', $this->pid, 'newspapers:pageCModel', 'isPartOf'),
    );
    return $tabset;
  }

  public function showPageFieldSets() {

    global $base_url;

    $tabset = array();

    $tabset = array(
      '#type' => 'tabset',
    );

    global $user;
    $qs = '';
    if ($user->uid != 0) {
      $qs = '?uid=' . base64_encode($user->name . ':' . $user->pass);
    }

    $viewer_url = variable_get('fedora_base_url', '') . '/get/' . $this->pid . '/ilives:viewerSdef/getViewer' . $qs;
    $html = '<iframe src="' . $viewer_url . '" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Read'),
      '#content' => $html
    );

    $item = new Fedora_Item($this->pid);

    // Get this page's parent item to show the issue's metadata.
    $rels = $item->get_relationships();
    $parent_pid = '';
    foreach ($rels as $rel) {
      if (strstr($rel['predicate'], 'isPartOf')) {
        $parent_pid = $rel['object'];
        break;
      }
    }
    $parent_item = new Fedora_Item($parent_pid);
    $tabset['second_tab'] = array(
      '#type' => 'tabpage',
      '#title' => 'Description',
      '#content' => $parent_item->get_dissemination('islandora:mods2htmlSdef', 'mods2html'),
    );

    return $tabset;
  }

}
