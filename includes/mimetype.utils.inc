<?php

/**
 * @file
 * Mimetype specific utility functions.
 */

/**
 * Retrieve the correct file extension for a give mimetype.
 *
 * @param string $mimetype
 *   The mimetype whose extension is required.
 *
 * @return string
 *   The extension mapped to the given mimetype. Defaults to 'bin'.
 */
function islandora_get_extension_for_mimetype($mimetype) {
  // file.mimetypes.inc is a part of Drupal core, however is not
  // automatically loaded. Manually require it.
  require_once DRUPAL_ROOT . "/includes/file.mimetypes.inc";
  $extension = 'bin';
  $mimetype_mapping = file_mimetype_mapping();
  $extension_index = array_search($mimetype, $mimetype_mapping['mimetypes']);
  if ($extension_index !== FALSE) {
    // Reversing such that any altered values take precedence over the default
    // mappings.
    $mime_array_flipped = array_reverse($mimetype_mapping['extensions']);
    $extension = array_search($extension_index, $mime_array_flipped);
  }

  // We can only have one mapping in drupal for 'xml'.
  if ($mimetype == "text/xml") {
    return "xml";
  }
  return $extension;
}

/**
 * Retrieve all file extensions for a give mimetype.
 *
 * @param string $mimetype
 *   The mimetype whose extensions are required.
 *
 * @return array
 *   All known legal extensions.
 */
function islandora_get_extensions_for_mimetype($mimetype) {
  // file.mimetypes.inc is a part of Drupal core, however is not
  // automatically loaded. Manually require it.
  require_once DRUPAL_ROOT . "/includes/file.mimetypes.inc";
  $mimetype_mapping = file_mimetype_mapping();
  $index = array_search($mimetype, $mimetype_mapping['mimetypes']);
  $extensions = array();
  if ($index !== FALSE) {
    $extensions = array_keys($mimetype_mapping['extensions'], $index);
  }
  // We can only have one mapping in drupal for 'xml'.
  if ($mimetype == "text/xml") {
    $extensions[] = 'xml';
  }
  return $extensions;
}

/**
 * Extensions for the mimes accepted by a datastream.
 *
 * @param AbstractObject $object
 *   Object to check for extensions.
 * @param string $dsid
 *   Datastream ID to check for extensions.
 *
 * @return string[]
 *   Extensions for the mimes accepted by the datastream ID on the object.
 */
function islandora_get_extensions_for_datastream(AbstractObject $object, $dsid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $datastream_mime_map = islandora_get_datastreams_requirements_from_models($object->models);
  $mimes = isset($datastream_mime_map[$dsid]) ? $datastream_mime_map[$dsid]['mime'] : array();
  if (isset($object[$dsid])) {
    $current_mime = $object[$dsid]->mimetype;
    if (!in_array($current_mime, $mimes)) {
      $mimes[] = $current_mime;
    }
  }
  $extensions = array();
  foreach ($mimes as $mime) {
    $extensions = array_merge($extensions, islandora_get_extensions_for_mimetype($mime));
  }
  return array_unique($extensions);
}

/**
 * Retrieve custom Islandora mime mappings.
 *
 * Our mime needs are rather extreme and we need to modify/extend Drupal's map.
 */
function islandora_mime_mapping() {
  return array(
    // Openoffice:
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
    // Staroffice:
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
    // K-office:
    'kil' => 'application/x-killustrator',
    'kpt' => 'application/x-kpresenter',
    'kpr' => 'application/x-kpresenter',
    'ksp' => 'application/x-kspread',
    'kwt' => 'application/x-kword',
    'kwd' => 'application/x-kword',
    // Ms office 97:
    'doc' => 'application/msword',
    'xls' => 'application/vnd.ms-excel',
    'ppt' => 'application/vnd.ms-powerpoint',
    // Office2007:
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
    'xsl' => 'text/xsl',
    'xslt' => 'text/xsl',
    'csv' => 'text/csv',
    'tsv' => 'text/tab-separated-values',
    'txt' => 'text/plain',
    // images:
    "bmp" => "image/bmp",
    'dng' => 'image/x-adobe-dng',
    "gif" => "image/gif",
    "ief" => "image/ief",
    "jpeg" => "image/jpeg",
    "jpe" => "image/jpeg",
    "jpg" => "image/jpeg",
    "jp2" => "image/jp2",
    "png" => "image/png",
    "tiff" => "image/tiff",
    "tif" => "image/tiff",
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
    "mkv" => "video/x-matroska",
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
    // Audio:
    "mp3" => "audio/mpeg",
    "mp4a" => "audio/mp4",
    "m4a" => "audio/mp4",
    "oga" => "audio/ogg",
    "ogg" => "audio/ogg",
    "flac" => "audio/x-flac",
    "wav" => "audio/vnd.wave",
    // Chemical:
    // MDL Molfile.
    "mol" => "chemical/x-mdl-molfile",
    // XYZ format.
    "xyz" => "chemical/x-xyz",
    // PDB.
    "pdb" => "chemical/x-pdb",
    // ChemDraw CDX.
    'cdx' => 'chemical/x-cdx',
    // ChemDraw 3D.
    "c3d" => "chemical/x-chem3d",
    // ChemDraw file.
    "chm" => "chemical/x-chemdraw",
    // Crystallographic Information File.
    "cif" => "chemical/x-cif",
    // Chemical Markup Language.
    "cml" => "chemical/x-cml",
    // GAMESS Input.
    "inp" => "chemical/x-gamess-input",
    // GAMESS Output.
    "gam" => "chemical/x-gamess-output",
    // Gaussian Cube.
    "cub" => "chemical/x-gaussian-cube",
    // Gaussian 98/03 Cartesian Input.
    "gau" => "chemical/x-gaussian-input",
    // JCAMP Spectroscopic Data Exchange Format.
    "jdx" => "chemical/x-jcamp-dx",
    // OpenDX Grid.
    "dx" => "chemical/x-jcamp-dx",
    // MOPAC Cartesian.
    "mop" => "chemical/x-mopac-input",
    // Compressed formats:
    // (note: http://svn.cleancode.org/svn/email/trunk/mime.types)
    "tgz" => "application/x-gzip",
    "gz" => "application/x-gzip",
    "tar" => "application/x-tar",
    "gtar" => "application/x-gtar",
    "zip" => "application/x-zip",
    "dat" => "application/octet-stream",
    // others:
    'bin' => 'application/octet-stream',
    // Web Archives:
    "warc" => "application/warc",
    "json" => "application/json",
    // JSON-LD
    "jsonld" => "application/ld+json",
  );
}
