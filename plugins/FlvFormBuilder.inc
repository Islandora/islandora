<?php



/**
 * @file
 * FLVFormBuilder
 */
module_load_include('inc', 'fedora_repository', 'plugins/FormBuilder');

/**
 * implements methods from content model ingest form xml
 * builds a dc metadata form
 */

class FlvFormBuilder extends FormBuilder {

  /**
   * Constructor
   */
  function FlvFormBuilder() {
    module_load_include('inc', 'fedora_repository', 'plugins/FormBuilder');
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  /**
   * method overrides method in FormBuilder.  We changed the dsid from OBJ to FLV and added the TN datastream
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
    $dformat = $mimetype->getType($file);
    //$fileUrl = 'http://'.$_SERVER['HTTP_HOST'].$file;
    $fileUrl = $base_url . '/' . drupal_urlencode($file);
    $beginIndex = strrpos($fileUrl, '/');
    $dtitle = substr($fileUrl, $beginIndex + 1);
    $dtitle = substr($dtitle, 0, strpos($dtitle, "."));
    $ds1 = $dom->createElement("foxml:datastream");
    $ds1->setAttribute("ID", "FLV");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "M");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1v->setAttribute("ID", "FLV.0");
    $ds1v->setAttribute("MIMETYPE", "$dformat");
    $ds1v->setAttribute("LABEL", "$dtitle");
    $ds1content = $dom->createElement('foxml:contentLocation');
    $ds1content->setAttribute("REF", "$fileUrl");
    $ds1content->setAttribute("TYPE", "URL");
    $ds1->appendChild($ds1v);
    $ds1v->appendChild($ds1content);
    $rootElement->appendChild($ds1);

    $createdFile = drupal_get_path('module', 'fedora_repository') . '/images/flashThumb.jpg';
    $fileUrl = $base_url . '/' . drupal_urlencode($createdFile); //'http://'.$_SERVER['HTTP_HOST'].'/'.$createdFile;
    $ds1 = $dom->createElement("foxml:datastream");
    $ds1->setAttribute("ID", "TN");
    $ds1->setAttribute("STATE", "A");
    $ds1->setAttribute("CONTROL_GROUP", "M");
    $ds1v = $dom->createElement("foxml:datastreamVersion");
    $ds1v->setAttribute("ID", "TN.0");
    $ds1v->setAttribute("MIMETYPE", "image/jpeg");
    $ds1v->setAttribute("LABEL", "Thumbnail");
    $ds1content = $dom->createElement('foxml:contentLocation');
    $ds1content->setAttribute("REF", "$fileUrl");
    $ds1content->setAttribute("TYPE", "URL");
    $ds1->appendChild($ds1v);
    $ds1v->appendChild($ds1content);
    $rootElement->appendChild($ds1);
  }

}

