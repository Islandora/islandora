<?php



/**
 * @file
 * ShowStreamsInFieldSets class
 */

/**
 * Show Streams In Field Sets ??
 */
class ShowStreamsInFieldSets {

  private $pid = NULL;

  /**
   * Constructor
   * @param type $pid 
   */
  function ShowStreamsInFieldSets($pid) {
    $this->pid = $pid;
  }

  /**
   * Show the FLV ??
   * @return type 
   */
  function showFlv() {
    //FLV is the datastream id
    $path = drupal_get_path('module', 'fedora_repository');
    $fullPath = url($path);
    $content = "";
    $pathTojs = drupal_get_path('module', 'fedora_repository') . '/js/swfobject.js';
    drupal_add_js("$pathTojs");
    $div_id = "player' . $this->pid . 'FLV";
    $content .= <<<EOH
<div id="$div_id"><a href="http://www.macromedia.com/go/getflashplayer">Get the Flash Player</a> to see this player.</div>
EOH;
    drupal_add_js(<<<EOJS
var s1 = new SWFObject("$fullPath/flash/flvplayer.swf","single","320","240","7");
s1.addParam("allowfullscreen","TRUE");
s1.addVariable("file", "$fullPath/fedora/repository/{$this->pid}/FLV/FLV.flv");
s1.write("$div_id");
EOJS
, 'inline', 'footer');
    $collection_fieldset = array(
      '#title' => t('Flash Video'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#value' => $content);
    return theme('fieldset', $collection_fieldset);
  }

  /**
   * Show the TN ??
   * @return type 
   */
  function showTN() {
    $collection_fieldset = array(
      '#title' => '',
      '#attributes' => array(),
      '#collapsible' => FALSE,
      '#value' => l(theme('image', "fedora/repository/{$this->pid}/TN/TN", '', '', NULL, FALSE), "fedora/repository/{$this->pid}/OBJ", array('html' => TRUE)),
    );
    return theme('fieldset', $collection_fieldset);
  }

  /**
   * Same as showTN but artinventory stores the image in a dsid of IMAGE instead of OBJ
   * @return type 
   */
  function showArtInventoryTN() {
    $collection_fieldset = array(
      '#collapsible' => FALSE,
      '#value' => l(theme('image', "fedora/repository/{$this->pid}/TN/TN", '', '', NULL, FALSE), "fedora/repository/{$this->pid}/IMAGE/image.jpg", array('html' => TRUE)),
    );
    return theme('fieldset', $collection_fieldset);
  }

  /**
   * Embed Google Docs' PDF viewer into the page.
   * @global type $base_url
   * @global type $base_path
   * @global type $user
   * @return type 
   */
  function showPDFPreview() {
    global $base_url;
    global $base_path;
    global $user;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');

    $tabset = array();

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Description'),
    );

    $tabset['first_tab']['tabs'] = array(
      '#type' => 'tabset',
    );
    $objectHelper = new ObjectHelper();
    $item = new Fedora_Item($this->pid);
    if (key_exists('TN', $item->datastreams)) {
      $tn_url = "fedora/repository/{$item->pid}/TN";
    }
    else {
      $tn_url = drupal_get_path('module', 'fedora_repository') . '/images/Crystal_Clear_app_download_manager.png';
    }
    $tn_url = url($tn_url);
    
    $dc_html = $objectHelper->getFormattedDC($item);

    $dl_link = l('<div  style="float:left; padding: 10px">' . theme('image', $tn_url, '', '', NULL, FALSE) . '<br />' . t('View Document') .'</div>', "fedora/repository/{$this->pid}/OBJ", array('html' => TRUE));

    $tabset['first_tab']['tabs']['view'] = array(
      '#type' => 'tabpage',
      '#title' => t('View'),
      '#content' => $dl_link . $dc_html,
    );

    if (fedora_repository_access(OBJECTHELPER :: $EDIT_FEDORA_METADATA, $this->pid, $user)) {
      $editform = drupal_get_form('fedora_repository_edit_qdc_form', $this->pid, 'DC');
      $tabset['first_tab']['tabs']['edit'] = array(
        '#type' => 'tabpage',
        '#title' => t('Edit'),
        '#content' => $editform,
      );
    }

    $viewer_url = 'http://docs.google.com/viewer?url=' . url("fedora/repository/{$this->pid}/OBJ/preview.pdf", array('absolute' => TRUE)) . '&embedded=TRUE';
    $tabset['second_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Read Online'),
      '#content' => <<<EOM
<iframe src="$viewer_url" style="width:600px; height:500px;" frameborder="0"></iframe>"
EOM
    );

    // Render the tabset.
    return $tabset;
  }

  /**
   * Show QDC ??
   * @return type 
   */
  function showQdc() {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $objectHelper = new ObjectHelper();
    $content = $objectHelper->getQDC($this->pid);
    $collection_fieldset = array(
      '#title' => t('Description'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#value' => $content,
    );
    return theme('fieldset', $collection_fieldset);
  }

  /**
   * Show Object Link ??
   * @global type $base_url
   * @return type 
   */
  function showOBJLink() {
    global $base_url;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($this->pid);
    $streams = $item->get_datastreams_list_as_array();
    return l($streams['OBJ']['label'], "fedora/repository/{$this->pid}/OBJ");
  }

  /**
   * Show REF works ??
   * @return type 
   */
  function showRefworks() {
    $path = drupal_get_path('module', 'fedora_repository');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "refworks");
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
    $xsl->load($path . '/xsl/refworks.xsl');
    $input = new DomDocument();
    $input->loadXML(trim($xmlstr));
    $xsl = $proc->importStylesheet($xsl);
    $newdom = $proc->transformToDoc($input);
    $content = $newdom->saveXML();

    $collection_fieldset = array(
      '#title' => t('Refworks'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#value' => $content);
    return theme('fieldset', $collection_fieldset);
  }

  /**
   * Show JP2000
   * @param type $collapsed
   * @return type 
   */
  function showJP2($collapsed = FALSE) {
    $viewer_url = variable_get('fedora_base_url', '') . '/get/' . $this->pid . '/ilives:viewerSdef/getViewer';
    $html = '<iframe src="' . $viewer_url . '" frameborder="0" style="width: 100%; height: 400px;">Errors: unable to load viewer</iframe>';
    $fieldset = array(
      '#title' => t('Viewer'),
      '#collapsible' => TRUE,
      '#collapsed' => $collapsed,
      '#value' => $html
    );
    return theme('fieldset', $fieldset);
  }

  /**
   * Show Romeo ??
   * @param type $collapsed
   * @return type 
   */
  function showRomeo($collapsed = FALSE) {
    $path = drupal_get_path('module', 'fedora_repository');
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "ROMEO", 0);

    if ($xmlstr == NULL || strlen($xmlstr) < 5) {
      return " ";
    }

    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('@e', array('@e' => check_plain($e->getMessage()))), 'error');
      return;
    }
    $xsl = new DomDocument();
    $xsl->load($path . '/xsl/romeo.xsl');
    $input = new DomDocument();
    $input->loadXML(trim($xmlstr));
    $xsl = $proc->importStylesheet($xsl);
    $newdom = $proc->transformToDoc($input);
    $content = $newdom->saveXML();

    $collection_fieldset = array(
      '#title' => t('Romeo'),
      '#collapsible' => TRUE,
      '#collapsed' => $collapsed,
      '#value' => $content
    );
    return theme('fieldset', $collection_fieldset);
  }

}
