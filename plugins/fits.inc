<?php

/**
 * @file
 * Fits
 * fits.sh must be in the apache user's path
 *
 * Download FITS from http://code.google.com/p/fits/
 * Installing: http://code.google.com/p/fits/wiki/installing
 * Edit line 5 in fits.sh (FITS_HOME='') and give it a home.
 * Make sure that home is in the apache user's path.
 *
 */

/**
 * This Class implements the methods defined in the STANDARD_IMAGE content model
 */
class fits {

  private $pid = NULL;
  private $item = NULL;

  /**
   * Constructor
   * @param type $pid
   */
  function __construct($pid = NULL) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {
      $this->pid = $pid;
      $this->item = new Fedora_Item($this->pid);
    }
  }

  /**
   * extract metadata
   * @param type $parameterArray
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return type
   */
  function extractFits($parameterArray, $dsid, $file, $file_ext) {
    if (variable_get('enable_fits', FALSE) == 1) {
      $file_name = '_' . $dsid . '.xml';
      $output = array();
      exec(variable_get('fits_path', '/usr/local/bin/fits.sh') .' -i ' . escapeshellarg($file) . '', $output);
      if ( !file_put_contents($file . $file_name, implode("\n", $output)) ) {
        //drupal_set_message(t("error writing fits file %s", array('%s' => "$file.$file_name")));
        return FALSE;
      }
      $_SESSION['fedora_ingest_files']["$dsid"] = $file . $file_name;
      return TRUE;
    }
    return TRUE; //this prevents getting the error following content model rules message when fits generation is turned off
  }

  /**
   * display metadata
   * @return type
   */
  function displayFits() {
    $output = '';
    $fits = $this->item->get_datastream_dissemination('TECHMD_FITS');
    if (trim($fits) != '') {
      $fitsDom = DOMDocument::loadXML($this->item->get_datastream_dissemination('FITS'));
      if ($fitsDom != NULL) {
        $description = $fitsDom->getElementsByTagName('fits');
        if ($description->length > 0) {
          $description = $description->item(0);
          $output .= '<div class="fedora_technical_metadata"><ul>';
          for ($i = 0; $i < $description->childNodes->length; $i++) {
            $name = $description->childNodes->item($i)->nodeName;
            $value = $description->childNodes->item($i)->nodeValue;
            if ($name != '#text' && !preg_match('/^System\:.*$/', $name) && trim($value) != '') {
              list($type, $name) = preg_split('/\:/', $name);
              $name = trim(preg_replace('/(?<!^)([A-Z][a-z]|(?<=[a-z])[A-Z])/', " $1", $name));
              $output .= '<li><b>' . $name . '</b>:  ' . $value . ' </li>';
            }
          }
          $output .= '</ul></div>';

          $fieldset = array(
            '#title' => t("!text", array('!text' => 'FITS Technical Metadata')),
            '#collapsible' => TRUE,
            '#collapsed' => TRUE,
            '#value' => $output
          );
          $output = theme('fieldset', $fieldset);
        }
      }
    }
    return $output;
  }

}
