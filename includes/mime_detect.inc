<?php
/**
 * @file
 * Replacement for the original islandora MimeClass.
 * Does essentially the same thing, just using the built in drupal 7
 * mime system.
 *
 * A nice added bonus is that this new class will put a note in watchdog when
 * it encounters a filetype it doesnt know.
 */

class MimeDetect {

  const DEFAULT_MIMETYPE = 'application/octet-stream';
  const DEFAULT_EXTENSION = 'bin';

  protected $mimeTypes;
  protected $extTypes;

  /**
   * Construtor.
   */
  public function __construct() {
    // Mimetype list returned as a double assoc array with two entries:
    // Mimetypes: Integer array of mimetypes
    // Extensions: Assoc array of extensions
    //
    // Extension array can be used as a key to get mimetypes
    // Example if $extensions['png'] == 5, then
    // $mimetypes[5] = 'image/png'
    // We want to combine this into 1 array for easy access.
    $temp = file_mimetype_mapping();

    $temp_mime_types = $temp['mimetypes'];
    $temp_mime_ext = $temp['extensions'];

    // Free the memory in temp since we don't need it anymore.
    unset($temp);

    // Combine the two arrays into one.
    foreach ($temp_mime_ext as $key => $val) {
      $this->mime_types[$key] = $temp_mime_types[$val];
    }

    // Flip the mime_type array to make the ext array.
    $this->ext_types = array_flip($this->mime_types);
  }

  /**
   * For backwards-compatibility with our old mimetype class.
   *
   * @param string $filename
   *   Filename to convert to mimetype.
   *
   * @return data
   *   Returns a string with the mimetype for the file.
   */
  public function getType($filename) {
    return $this->getMimetype($filename);
  }

  /**
   * Returns a mimetype associated with the file extension of $filename.
   *
   * @param string $filename
   *   The filename we want the mime type of.
   *
   * @param bool $debug
   *   Enable or disable debug output mode.
   *  
   * @return string
   *   Mimetype associated with the file extension of $filename.
   */
  public function getMimetype($filename, $debug = FALSE) {
    // The file extension will be the last index in the exploded string.
    $ext = @end(explode('.', $filename));
    $ext = strtolower($ext);

    // Check the array for a predetermined mime type.
    // If type not found, add a warning to watchdog.
    if (!empty($this->mime_types[$ext])) {
      if (TRUE === $debug) {
        $return_type = $this->mime_types[$ext];
        return array('mime_type' => $return_type, 'method' => 'from_array');
      }

      return $this->mime_types[$ext];
    }
    else {
      // This file was not in the drupal mime system.
      // Add a line to the watchdog informing us of this incident.
      $err_msg = 'Found a file extension not covered in the drupal mime func:';
      watchdog('MimeClass', $err_msg . $ext);
    }

    // By default return application octet stream.
    if (TRUE == $debug) {
      return array('mime_type' => 'application/octet-stream', 'method' => 'last_resort');
    }

    return 'application/octet-stream';
  }

  /**
   * Returns *one* valid file extension for a given $mime_type.
   *
   * @param string $type
   *   String containing the mimetype
   *
   * @param bool $debug
   *   Toggle debug mode on/off
   *
   * @return string
   *   Return the file extension
   */
  public function getExtension($type, $debug = FALSE) {

    if (!empty($this->ext_types[$type])) {
      if (TRUE == $debug) {
        return array('extension' => $this->ext_types[$type], 'method' => 'from_array');
      }
      return $this->ext_types[$type];
    }
    else {
      // Return bin by default. This could cause errors.
      // Function should return false or -1 or something to be handled.
      // Client side.
      if (TRUE == $debug) {
        return array('extension' => 'bin', 'method' => 'last_resort');
      }
      return 'bin';
    }
  }

}
