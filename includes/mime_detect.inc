<?php

/**
 * @file
 * Class for determining MIME types and file extensions.
 *
 * This class inspired by Chris Jean's work, here:
 * http://chrisjean.com/2009/02/14/generating-mime-type-in-php-is-not-magic/
 *
 * It does some MIME trickery, inspired by the need to to deal with Openoffice
 * and MS Office 2007 file formats -- which are often mis-interpreted by
 * mime-magic, fileinfo, and the *nix `file` command.
 *
 * In Drupal 6, we also make use of file_get_mimetype. See:
 * http://api.drupal.org/api/function/file_get_mimetype/6
 * ... however this only provides a uni-directional lookup (ext->mime).
 * While I don't have a specific use case for a mime->extension lookup, I think
 * it's good to have in here.
 *
 * Drupal 7 will have better mime handlers.  See:
 * http://api.drupal.org/api/function/file_default_mimetype_mapping/7
 */

/**
 * Class for doing mapping to mimetypes.
 */
// @codingStandardsIgnoreLine
class MimeDetect {
  /**
   * This is a shortlist of mimetypes.
   *
   * It should catch most mimetype<-->extension lookups in the context of
   * Islandora collections.
   *
   * It has been cut from a much longer list.
   *
   * Two types of mimetypes should be put in this list:
   * 1) Special emerging formats which may not yet be expressed in the system
   * mime.types file.
   * 2) Heavily used mimetypes of particular importance to the Islandora
   * project, as lookups against this list will be quicker and less
   * resource intensive than other methods.
   *
   * Lookups are first checked against this short list.  If no results are
   * found, then the lookup function may move on to check other sources,
   * namely the system's mime.types file.
   *
   * In most cases though, this short list should suffice.
   *
   * If modifying this list, please note that for promiscuous mimetypes
   * (those which map to multiple extensions, such as text/plain)
   * The function get_extension will always return the *LAST* extension in
   * this list, so you should put your preferred extension *LAST*.
   *
   * e.g...
   * "jpeg"    => "image/jpeg",
   * "jpe"     => "image/jpeg",
   * "jpg"     => "image/jpeg",
   *
   * $this->get_extension('image/jpeg') will always return 'jpg'.
   *
   * @var array
   */
  protected $protectedMimeTypes;

  /**
   * This is a shortlist of file extensions.
   *
   * @var array
   */
  protected $protectedFileExtensions;

  /**
   * Extension exceptions.
   *
   * XXX: Deprecated... Only here due to old 'tif' => 'image/tif' mapping...
   * The correct MIMEtype is 'image/tiff'.
   *
   * @var array
   */
  protected $extensionExceptions = array(
    'image/tif' => 'tif',
  );

  /**
   * Stores the system list of mimetypes.
   *
   * @var array
   */
  protected $systemTypes;

  /**
   * Stores the system list of extensions.
   *
   * @var array
   */
  protected $systemExts;

  /**
   * System mime.types file location.
   *
   * @var string
   */
  protected $etcMimeTypes = '/etc/mime.types';

  /**
   * Construtor.
   */
  public function __construct() {
    module_load_include('inc', 'islandora', 'includes/mimetype.utils');
    $this->protectedMimeTypes = islandora_mime_mapping();
    // Populate the reverse shortlist:
    $this->protectedFileExtensions = array_flip($this->protectedMimeTypes);
    $this->protectedFileExtensions += $this->extensionExceptions;

    // Pick up a local mime.types file if it is available.
    if (is_readable('mime.types')) {
      $this->etcMimeTypes = 'mime.types';
    }
  }

  /**
   * Gets MIME type associated with the give file's extension.
   *
   * @param string $filename
   *   The filename.
   * @param bool $debug
   *   Returns a debug array.
   *
   * @return mixed
   *   string or an array
   */
  public function getMimetype($filename, $debug = FALSE) {

    $file_name_and_extension = explode('.', $filename);
    $ext = drupal_strtolower(array_pop($file_name_and_extension));

    if (!empty($this->protectedMimeTypes[$ext])) {
      if (TRUE === $debug) {
        return array('mime_type' => $this->protectedMimeTypes[$ext], 'method' => 'from_array');
      }
      return $this->protectedMimeTypes[$ext];
    }

    if (function_exists('file_get_mimetype')) {
      $drupal_mimetype = file_get_mimetype($filename);
      if ('application/octet-stream' != $drupal_mimetype) {
        if (TRUE == $debug) {
          return array('mime_type' => $drupal_mimetype, 'method' => 'file_get_mimetype');
        }
        return $drupal_mimetype;
      }
    }

    if (!isset($this->systemTypes)) {
      $this->systemTypes = $this->systemExtensionMimetypes();
    }
    if (isset($this->systemTypes[$ext])) {
      if (TRUE == $debug) {
        return array('mime_type' => $this->systemTypes[$ext], 'method' => 'mime.types');
      }
      return $this->systemTypes[$ext];
    }

    if (TRUE === $debug) {
      return array('mime_type' => 'application/octet-stream', 'method' => 'last_resort');
    }
    return 'application/octet-stream';
  }

  /**
   * Gets one valid file extension for a given MIME type.
   *
   * @param string $mime_type
   *   The MIME type.
   * @param bool $debug
   *   Generated debug information?
   *
   * @return string
   *   The file extensions associated with the given MIME type.
   */
  public function getExtension($mime_type, $debug = FALSE) {

    if (!empty($this->protectedFileExtensions[$mime_type])) {
      if (TRUE == $debug) {
        return array('extension' => $this->protectedFileExtensions[$mime_type], 'method' => 'from_array');
      }
      return $this->protectedFileExtensions[$mime_type];
    }

    if (!isset($this->systemExts)) {
      $this->systemExts = $this->systemMimetypeExtensions();
    }
    if (isset($this->systemExts[$mime_type])) {
      if (TRUE == $debug) {
        return array('extension' => $this->systemExts[$mime_type], 'method' => 'mime.types');
      }
      return $this->systemExts[$mime_type];
    }

    if (TRUE == $debug) {
      return array('extension' => 'bin', 'method' => 'last_resort');
    }
    return 'bin';
  }

  /**
   * Gets an associative array of MIME type and extension associations.
   *
   * Users the system mime.types file, or a local mime.types if one is found.
   *
   * @see MIMEDetect::__constuctor()
   *
   * @return array
   *   An associative array where the keys are MIME types and the values
   *   extensions.
   */
  protected function systemMimetypeExtensions() {
    $out = array();
    if (file_exists($this->etcMimeTypes)) {
      $file = fopen($this->etcMimeTypes, 'r');
      while (($line = fgets($file)) !== FALSE) {
        $line = trim(preg_replace('/#.*/', '', $line));
        if (!$line) {
          continue;
        }
        $parts = preg_split('/\s+/', $line);
        if (count($parts) == 1) {
          continue;
        }
        // A single part means a mimetype without extensions, which we ignore.
        $type = array_shift($parts);
        if (!isset($out[$type])) {
          $out[$type] = array_shift($parts);
        }
        // We take the first ext from the line if many are present.
      }
      fclose($file);
    }
    return $out;
  }

  /**
   * Gets a associative array of extensions and MIME types.
   *
   * Users the system mime.types file, or a local mime.types if one is found.
   *
   * @see MIMEDetect::__constuctor()
   *
   * @return array
   *   An associative array where the keys are extensions and the values
   *   MIME types.
   */
  protected function systemExtensionMimetypes() {
    $out = array();
    if (file_exists($this->etcMimeTypes)) {
      $file = fopen($this->etcMimeTypes, 'r');
      while (($line = fgets($file)) !== FALSE) {
        $line = trim(preg_replace('/#.*/', '', $line));
        if (!$line) {
          continue;
        }
        $parts = preg_split('/\s+/', $line);
        if (count($parts) == 1) {
          continue;
        }
        // A single part means a mimetype without extensions, which we ignore.
        $type = array_shift($parts);
        foreach ($parts as $part) {
          $out[$part] = $type;
        }
      }
      fclose($file);
    }
    return $out;
  }

  /**
   * Gets MIME type array.
   *
   * @return array
   *   Returns associative array with exts and mimetypes.
   */
  public function getMimeTypes() {
    return $this->protectedMimeTypes;
  }

  /**
   * Get all valid extensions for this MIME type.
   *
   * @param string $mimetype
   *   The MIME type we are searching for.
   *
   * @return array
   *   An array of valid extensions for this MIME type.
   */
  public function getValidExtensions($mimetype) {
    $filter = function ($mime) use ($mimetype) {
      return $mime == $mimetype;
    };
    return array_keys(array_filter($this->protectedMimeTypes, $filter));
  }

}
