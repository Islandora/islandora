<?php



/**
 * @file
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
 *
 */

class MimeClass {

  private $private_mime_types = array(
    /*
     * This is a shortlist of mimetypes which should catch most
     * mimetype<-->extension lookups in the context of Islandora collections.
     *
     * It has been cut from a much longer list.
     *
     * Two types of mimetypes should be put in this list:
     *  1) Special emerging formats which may not yet be expressed in the system
     *     mime.types file.
     *  2) Heavily used mimetypes of particular importance to the Islandora
     *     project, as lookups against this list will be quicker and less
     *     resource intensive than other methods.
     *
     * Lookups are first checked against this short list.
     * If no results are found, then the lookup function may move
     * on to check other sources, namely the system's mime.types file.
     *
     * In most cases though, this short list should suffice.
     *
     * If modifying this list, please note that for promiscuous mimetypes
     * (those which map to multiple extensions, such as text/plain)
     * The function get_extension will always return the *LAST* extension
     * in this list, so you should put your preferred extension *LAST*.
     *
     * e.g...
     * "jpeg"    => "image/jpeg",
     * "jpe"     => "image/jpeg",
     * "jpg"     => "image/jpeg",
     *
     * $this->get_extension('image/jpeg') will always return 'jpg'.
     *
     */
    // openoffice:
    'odb' => 'application/vnd.oasis.opendocument.database',
    'odc' => 'application/vnd.oasis.opendocument.chart',
    'odf' => 'application/vnd.oasis.opendocument.formula',
    'odg' => 'application/vnd.oasis.opendocument.graphics',
    'odi' => 'application/vnd.oasis.opendocument.image',
    'odm' => 'application/vnd.oasis.opendocument.text-master',
    'odp' => 'application/vnd.oasis.opendocument.presentation',
    'ods' => 'application/vnd.oasis.opendocument.spreadsheet',
    'odt' => 'application/vnd.oasis.opendocument.text',
    'otg' => 'application/vnd.oasis.opendocument.graphics-template',
    'oth' => 'application/vnd.oasis.opendocument.text-web',
    'otp' => 'application/vnd.oasis.opendocument.presentation-template',
    'ots' => 'application/vnd.oasis.opendocument.spreadsheet-template',
    'ott' => 'application/vnd.oasis.opendocument.text-template',
    // staroffice:
    'stc' => 'application/vnd.sun.xml.calc.template',
    'std' => 'application/vnd.sun.xml.draw.template',
    'sti' => 'application/vnd.sun.xml.impress.template',
    'stw' => 'application/vnd.sun.xml.writer.template',
    'sxc' => 'application/vnd.sun.xml.calc',
    'sxd' => 'application/vnd.sun.xml.draw',
    'sxg' => 'application/vnd.sun.xml.writer.global',
    'sxi' => 'application/vnd.sun.xml.impress',
    'sxm' => 'application/vnd.sun.xml.math',
    'sxw' => 'application/vnd.sun.xml.writer',
    // k-office:
    'kil' => 'application/x-killustrator',
    'kpt' => 'application/x-kpresenter',
    'kpr' => 'application/x-kpresenter',
    'ksp' => 'application/x-kspread',
    'kwt' => 'application/x-kword',
    'kwd' => 'application/x-kword',
    // MS office 97:
    'doc' => 'application/msword',
    'xls' => 'application/vnd.ms-excel',
    'ppt' => 'application/vnd.ms-powerpoint',
    // office2007:
    'docx' => 'application/vnd.openxmlformats-officedocument.wordprocessingml.document',
    'docm' => 'application/vnd.ms-word.document.macroEnabled.12',
    'dotx' => 'application/vnd.openxmlformats-officedocument.wordprocessingml.template',
    'dotm' => 'application/vnd.ms-word.template.macroEnabled.12',
    'xlsx' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
    'xlsm' => 'application/vnd.ms-excel.sheet.macroEnabled.12',
    'xltx' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.template',
    'xltm' => 'application/vnd.ms-excel.template.macroEnabled.12',
    'xlsb' => 'application/vnd.ms-excel.sheet.binary.macroEnabled.12',
    'xlam' => 'application/vnd.ms-excel.addin.macroEnabled.12',
    'pptx' => 'application/vnd.openxmlformats-officedocument.presentationml.presentation',
    'pptm' => 'application/vnd.ms-powerpoint.presentation.macroEnabled.12',
    'ppsx' => 'application/vnd.openxmlformats-officedocument.presentationml.slideshow',
    'ppsm' => 'application/vnd.ms-powerpoint.slideshow.macroEnabled.12',
    'potx' => 'application/vnd.openxmlformats-officedocument.presentationml.template',
    'potm' => 'application/vnd.ms-powerpoint.template.macroEnabled.12',
    'ppam' => 'application/vnd.ms-powerpoint.addin.macroEnabled.12',
    'sldx' => 'application/vnd.openxmlformats-officedocument.presentationml.slide',
    'sldm' => 'application/vnd.ms-powerpoint.slide.macroEnabled.12',
    // Wordperfect (who cares?):
    'wpd' => 'application/wordperfect',
    // Common and generic containers:
    'pdf' => 'application/pdf',
    'eps' => 'application/postscript',
    'ps' => 'application/postscript',
    'rtf' => 'text/rtf',
    'rtx' => 'text/richtext',
    'latex' => 'application/x-latex',
    'tex' => 'application/x-tex',
    'texi' => 'application/x-texinfo',
    'texinfo' => 'application/x-texinfo',
    // *ml:
    'css' => 'text/css',
    'htm' => 'text/html',
    'html' => 'text/html',
    'wbxml' => 'application/vnd.wap.wbxml',
    'xht' => 'application/xhtml+xml',
    'xhtml' => 'application/xhtml+xml',
    'xsl' => 'text/xml',
    'xslt' => 'text/xml',
    'xml' => 'text/xml',
    'csv' => 'text/csv',
    'tsv' => 'text/tab-separated-values',
    'txt' => 'text/plain',
    // images:
    "bmp" => "image/bmp",
    "gif" => "image/gif",
    "ief" => "image/ief",
    "jpeg" => "image/jpeg",
    "jpe" => "image/jpeg",
    "jpg" => "image/jpeg",
    "jp2" => "image/jp2",
    "png" => "image/png",
    "tiff" => "image/tiff",
    "tif" => "image/tif",
    "djvu" => "image/vnd.djvu",
    "djv" => "image/vnd.djvu",
    "wbmp" => "image/vnd.wap.wbmp",
    "ras" => "image/x-cmu-raster",
    "pnm" => "image/x-portable-anymap",
    "pbm" => "image/x-portable-bitmap",
    "pgm" => "image/x-portable-graymap",
    "ppm" => "image/x-portable-pixmap",
    "rgb" => "image/x-rgb",
    "xbm" => "image/x-xbitmap",
    "xpm" => "image/x-xpixmap",
    "xwd" => "image/x-windowdump",
    // videos:
    "mpeg" => "video/mpeg",
    "mpe" => "video/mpeg",
    "mpg" => "video/mpeg",
    "m4v" => "video/mp4",
    "mp4" => "video/mp4",
    "ogv" => "video/ogg",
    "qt" => "video/quicktime",
    "mov" => "video/quicktime",
    "mxu" => "video/vnd.mpegurl",
    "avi" => "video/x-msvideo",
    "movie" => "video/x-sgi-movie",
    "flv" => "video/x-flv",
    "swf" => "application/x-shockwave-flash",
    'mkv' => "video/x-matroska",
    'wmv' =>	'video/x-ms-wmv',
    // audio:
    "mp3" => "audio/mpeg",
    "mp4a" => "audio/mp4",
    "m4a" => "audio/mp4",
    "oga" => "audio/ogg",
    "ogg" => "audio/ogg",
    "flac" => "audio/x-flac",
    "wav" => "audio/vnd.wave",
    /* Compressed formats:
      (note: http://svn.cleancode.org/svn/email/trunk/mime.types).*/
    "tgz" => "application/x-gzip",
    "gz" => "application/x-gzip",
    "tar" => "application/x-tar",
    "gtar" => "application/x-gtar",
    "zip" => "application/x-zip",
    // others:
    'bin' => 'application/octet-stream',
    'json' => 'application/json',
  );

  private $private_file_extensions;
  private $system_types;
  private $system_exts;
  private $etc_mime_types = '/etc/mime.types';

  /**
   * Construtor
   */
  public function __construct() {

    // Populate the reverse shortlist:
    $this->private_file_extensions = array_flip($this->private_mime_types);

    // Pick up a local mime.types file if it is available.
    if (is_readable('mime.types')) {
      $this->etc_mime_types = 'mime.types';
    }
  }

  /**
   * function: getType
   * description: An alias to get_mimetype,
   * for backwards-compatibility with our old mimetype class.
   *
   * @param type $filename
   * @return type
   */
  public function getType($filename) {
    return $this->get_mimetype($filename);
  }

  /**
   * function: get_mimetype
   * description: returns a mimetype associated with the
   *   file extension of $filename
   *
   * @param type $filename
   * @param type $debug
   * @return string
   *   mimetype associated with the file extension of $filename
   */
  public function get_mimetype($filename, $debug = FALSE) {
    $ext = strtolower(substr($filename, strrpos($filename, '.') + 1));

    if (!empty($this->private_mime_types[$ext])) {
      if (TRUE === $debug) {
        return array('mime_type' => $this->private_mime_types[$ext], 'method' => 'from_array');
      }
      return $this->private_mime_types[$ext];
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

    if (!isset($this->system_types)) {
      $this->system_types = $this->system_extension_mime_types();
    }
    if (isset($this->system_types[$ext])) {
      if (TRUE == $debug) {
        return array('mime_type' => $this->system_types[$ext], 'method' => 'mime.types');
      }
      return $this->system_types[$ext];
    }

    if (TRUE === $debug) {
      return array('mime_type' => 'application/octet-stream', 'method' => 'last_resort');
    }
    return 'application/octet-stream';
  }

  /**
   * function: get_extension
   * description: returns *one* valid file extension for a given $mime_type
   *
   * @param type $mime_type
   * @param type $debug
   * @return type
   */
  public function get_extension($mime_type, $debug = FALSE) {

    if (!empty($this->private_file_extensions[$mime_type])) {
      if (TRUE == $debug) {
        return array('extension' => $this->private_file_extensions[$mime_type], 'method' => 'from_array');
      }
      return $this->private_file_extensions[$mime_type];
    }

    if (!isset($this->system_exts)) {
      $this->system_exts = $this->system_mime_type_extensions();
    }
    if (isset($this->system_exts[$mime_type])) {
      if (TRUE == $debug) {
        return array('extension' => $this->system_exts[$mime_type], 'method' => 'mime.types');
      }
      return $this->system_exts[$mime_type];
    }

    if (TRUE == $debug) {
      return array('extension' => 'bin', 'method' => 'last_resort');
    }
    return 'bin';
  }

  /**
   * function: system_mime_type_extensions
   * description: populates an internal array of mimetype/extension associations
   * from the system mime.types file, or a local mime.types if one is found (see
   * __constuctor).
   * return: array of mimetype => extension
   */
  private function system_mime_type_extensions() {
    $out = array();
    if (file_exists($this->etc_mime_types)) {
      $file = fopen($this->etc_mime_types, 'r');
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
   * function: system_mime_type_extensions
   * description: populates an internal array of mimetype/extension associations
   * from the system mime.types file, or a local mime.types if one is found (see
   * __constuctor).
   * return: array of extension => mimetype
   */
  private function system_extension_mime_types() {
    $out = array();
    if (file_exists($this->etc_mime_types)) {
      $file = fopen($this->etc_mime_types, 'r');
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

}
