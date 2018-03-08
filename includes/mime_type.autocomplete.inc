<?php

/**
 * @file
 * Autocomplete functionality for MIME types in Islandora.
 */

/**
 * Autocomplete the MIME type name.
 *
 * @param string $string
 *   A search string.
 *
 * @return string
 *   The rendered JSON results.
 */
function islandora_mime_type_autocomplete($string) {
  require_once DRUPAL_ROOT . "/includes/file.mimetypes.inc";
  $mime_types = file_mimetype_mapping();
  $output = array();
  foreach ($mime_types as $mime_type) {
    if (preg_match("/{$string}/i", $mime_type) !== 0) {
      $output[$mime_type] = $mime_type;
    }
  }
  return drupal_json_output($output);
}
