<?php



/**
 * @file
 * Ffmpeg wrapper class
 */

/**
 * FFMpeg wrapper class for generating movie thumbnails
 * 
 * This Class implements the methods defined in the STANDARD_QT content model
 */
class Ffmpeg {

  /**
   * Default constructor
   */
  function Ffmpeg() {
    
  }

  /**
   * Extract a thumbnail from the movie
   * @param type $parameterArray
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return type 
   */
  function extract_thumbnail($parameterArray, $dsid, $file, $file_ext) {
    $defaults = array('ss' => '00:00:10', 's' => NULL);
    $params = array_merge($defaults, $parameterArray);
    $system = getenv('System');
    $file_suffix = '_' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $output = array();
    $size = '';

    if ($params['s'] != NULL) {
      $size = ' -s ' . escapeshellarg($params['s']);
    }
    exec('ffmpeg -i ' . escapeshellarg($file) . ' -r 1 -ss ' . escapeshellarg($params['ss']) . ' ' . $size . ' -t 1 ' . escapeshellarg($file . $file_suffix));

    if (!file_exists($file . $file_suffix)) {
      return FALSE;
    }

    $_SESSION['fedora_ingest_files']["$dsid"] = $file . $file_suffix;
    return TRUE;
  }

}
