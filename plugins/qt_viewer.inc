<?php



/**
 * @file
 * ShowQTStreamsInFieldSets class
 */

/**
 * Show QT Stream in Field Sets
 */
class ShowQtStreamsInFieldSets {

  private $pid = NULL;

  /**
   * Constructor
   * @param type $pid 
   */
  function ShowQtStreamsInFieldSets($pid) {
    $this->pid = $pid;
  }

  /**
   * Returna a new Fedora Object with the QT movie ???
   * @return fedora_item 
   */
  function fedoraObject() {
    return new fedora_item($this->pid);
  }

  /**
   * Tecnical metadata ??
   * @param type $defaults
   * @param type $dsid
   * @return type 
   */
  function technicalMetadata($defaults = array(), $dsid = 'OBJ_EXIFTOOL') {
    $data = $defaults;

    try {
      $src = ObjectHelper::getStream($this->pid, $dsid);

      $doc = new SimpleXMLElement($src);
      $doc->registerXPathNamespace('File', 'http://ns.exiftool.ca/File/1.0/');
      $doc->registerXPathNamespace('Composite', 'http://ns.exiftool.ca/Composite/1.0/');
      $mime = reset($doc->xpath('//File:MIMEType'));
      $data['mime'] = $mime;
      if (strpos($mime, 'audio/') !== FALSE) {
        $data['width'] = 300;
        $data['height'] = 0;
      }
      else {
        $size = reset($doc->xpath('//Composite:ImageSize/text()'));
        list($width, $height) = explode('x', $size);
        $data['width'] = $width;
        $data['height'] = $height;
      }

      $data['doc'] = $src;
    } catch (Exception $e) {
      $data = $defaults;
    }

    return $data;
  }

  /**
   * Get Poster Frame Datastream Information ??
   * @param type $dsid
   * @return type 
   */
  function getPosterFrameDatastreamInfo($dsid = 'FULL_SIZE') {
    $p = ObjectHelper::getDatastreamInfo($this->pid, $dsid);
    if (empty($p) || $p == ' ' || $p === FALSE) {
      return FALSE;
    }
    return $p;
  }

  /**
   * Get Media Datastream Information ??
   * @param type $dsid
   * @param type $alt
   * @return type 
   */
  function getMediaDatastreamInfo($dsid = 'OBJ', $alt = array('')) {
    $p = ObjectHelper::getDatastreamInfo($this->pid, $dsid);
    if (empty($p) || $p == ' ' || $p === FALSE) {
      if (!empty($alt)) {
        $ds = array_shift($alt);
        return $this->getMediaDatastreamInfo($ds, $alt);
      }
      return FALSE;
    }

    return $p;
  }

  /**
   * Is download enabled. It always returns FALSE. ??? 
   * @return FALSE 
   */
  function enableDownload() {
    return FALSE;
  }

  /**
   * Show the QT ???
   * @global type $base_url
   * @return type 
   */
  function showQt() {
    module_load_include('inc', 'fedora_repository', 'plugins/tagging_form');
    module_load_include('inc', 'fedora_repository', 'plugins/ShowStreamsInFieldSets');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');

    $info = $this->technicalMetadata(array('width' => 640, 'height' => 480));
    $width = $info['width'];
    $height = $info['height'];

    $pframe = $this->getPosterFrameDatastreamInfo();
    $media = $this->getMediaDatastreamInfo('PROXY', array('OBJ'));
    if ($media === FALSE) {
      return '';
    }
    global $base_url;
    $path = drupal_get_path('module', 'Fedora_Repository');
    $fullPath = base_path() . $path;
    $content = '';
    $pathTojs = drupal_get_path('module', 'Fedora_Repository') . '/js/AC_Quicktime.js';
    drupal_add_js($pathTojs);

    $divid = 'player' . md5($this->pid) . 'MOV';
    $content .= '<div  class="player"  id="' . $divid . '">';
    if ($pframe !== FALSE) {
      $content .= '<div class="poster" style="cursor: pointer; position: relative; width: ' . $width . 'px; min-height: ' . ($height) . 'px;">';
      $content .= '<img src="' . base_path() . 'fedora/repository/' . $this->pid . '/' . $pframe->ID . '/poster.jpg' . '" />';
      $content .= '<div class="play" style="font-size: 128px; color: white; position: absolute; top: 50%; left: 50%; margin-top: -0.085em; margin-left: -0.33em; opacity: 0.9; ">▶</div>';
      $content .= '</div>';
    }
    $content .= '</div>';
    if ($this->enableDownload()) {
      $url = base_path() . 'fedora/repository/' . $this->pid . '/OBJ/MOV.mov';
      $content .= '<a class="download" href="' . $url . '">Download Media File</a>';
    }
    $src = base_path() . 'fedora/repository/' . $this->pid . '/' . $media->ID . '/MOV.mov';
    $qtparams = '';
    $qtparams .= "'autostart', '" . ($pframe !== FALSE ? 'TRUE' : 'FALSE') . "', ";
    $init = <<<EOD
     $(function() { 
       src = "$src";
       if(src.substring(0,4) != 'http') {
       src = 'http://' + location.host + src;
       }
     	str = QT_GenerateOBJECTText_XHTML(src, "$width", ($height+15), '', 
		$qtparams
       		'postdomevents', 'TRUE',
      		'EnableJavaScript', 'TRUE',
       		'bgcolor', 'black',
       		'controller', 'TRUE',
       		'SCALE', 'aspect',
       		'LOOP', 'FALSE'
       	);		
	if($('.poster', '#$divid').length ==0) {
      	$('#$divid').append(str);
        } else {
         $('#$divid .poster').one('click', function() { $(this).hide(); $('#$divid').append(str); });
        }
    });
EOD;

    drupal_add_js($init, 'inline', 'footer');

    $collection_fieldset = array(
      '#title' => t('Quicktime'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#value' => $content);
    return theme('fieldset', $collection_fieldset);
  }

}
