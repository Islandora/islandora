<?php

/**
 * @file
 * Contains the admin form and callback functions for datastream manipulations.
 */

/**
 * Callback to download the given datastream to the users computer.
 *
 * @param AbstractDatastream $datastream
 *   The datastream to download.
 */
function islandora_download_datastream(AbstractDatastream $datastream) {
  islandora_view_datastream($datastream, TRUE);
}

/**
 * Callback function to view or download a datastream.
 *
 * @note
 *  This function calls exit().
 *
 * @param AbstractDatastream $datastream
 *   The datastream to view/download.
 * @param bool $download
 *   If TRUE the file is download to the user computer for viewing otherwise it
 *   will attempt to display in the browser natively.
 * @param int $version
 *   The version of the datastream to display
 */
function islandora_view_datastream(AbstractDatastream $datastream, $download = FALSE, $version = NULL) {
  if ($version !== NULL) {
    if (isset($datastream[$version])) {
      $datastream = $datastream[$version];
    }
    else {
      return drupal_not_found();
    }
  }

  header_remove('Cache-Control');
  header_remove('Expires');
  header('Content-type: ' . $datastream->mimetype);
  if ($datastream->controlGroup == 'M' || $datastream->controlGroup == 'X') {
    header('Content-length: ' . $datastream->size);
  }
  if ($download) {
    // Browsers will not append all extensions.
    $mime_detect = new MimeDetect();
    $extension = $mime_detect->getExtension($datastream->mimetype);
    $filename = $datastream->label . '.' . $extension;
    header("Content-Disposition: attachment; filename=\"$filename\"");
  }
  drupal_page_is_cacheable(FALSE);
  // Try not to load the file into PHP memory!
  ob_end_flush();
  $datastream->getContent('php://output');
  exit();
}

/**
 * Get the human readable size of the given datastream.
 *
 * @param AbstractDatastream $datastream
 *   The datastream to check.
 *
 * @return string
 *   A human readable size of the given datastream, or '-' if the size could not
 *   be determined.
 */
function islandora_datastream_get_human_readable_size(AbstractDatastream $datastream) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $size_is_calculatable = $datastream->controlGroup == 'M' || $datastream->controlGroup == 'X';
  return $size_is_calculatable ? islandora_convert_bytes_to_human_readable($datastream->size) : '-';
}

/**
 * Get either the 'view' or 'download' url for the given datastream if possible.
 *
 * @param AbstractDatastream $datastream
 *   The datastream to generated the url to.
 * @param string $type
 *   One of:
 *   - download
 *   - view
 * @param int $version
 *   (Optional) The version of the datastream to get a URL for.
 *
 * @return string
 *   either the 'view' or 'download' url for the given datastream.
 */
function islandora_datastream_get_url(AbstractDatastream $datastream, $type = 'download', $version = NULL) {
  if ($version === NULL) {
    $link = "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/$type";
  }
  else {
    $link = "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/version/$version/$type";
    $datastream = $datastream[$version];
  }

  if ($datastream->controlGroup == 'R') {
    return $datastream->url;
  }
  else {
    return $link;
  }
}

/**
 * Gets the delete link.
 *
 * @param AbstractDatastream $datastream
 *   The datastream to generated the url to.
 *
 * @return string
 *   Markup containing the link to the confirm form to delete the datastream.
 */
function islandora_datastream_get_delete_link(AbstractDatastream $datastream) {
  $message = islandora_deprecated('7.x-1.2', 'Use the "islandora_datastream_delete_link" theme implementation.');
  trigger_error(filter_xss($message), E_USER_DEPRECATED);

  return theme('islandora_datastream_delete_link', array(
    'datastream' => $datastream,
  ));
}

/**
 * Gets the edit link.
 *
 * @param AbstractDatastream $datastream
 *   The datastream to generated the url to.
 *
 * @return string
 *   Markup containing the link to edit the datastream.
 */
function islandora_datastream_edit_get_link(AbstractDatastream $datastream) {
  $message = islandora_deprecated('7.x-1.2', 'Use the "islandora_datastream_edit_link" theme implementation.');
  trigger_error(filter_xss($message), E_USER_DEPRECATED);

  return theme('islandora_datastream_edit_link', array(
    'datastream' => $datastream,
  ));
}

/**
 * Display the edit datastream page.
 *
 * @param AbstractDatastream $datastream
 *   The datastream to edit.
 */
function islandora_edit_datastream(AbstractDatastream $datastream) {
  $edit_registry = module_invoke_all('islandora_edit_datastream_registry', $datastream->parent, $datastream);
  $edit_count = count($edit_registry);
  switch ($edit_count) {
    case 0:
      // No edit implementations.
      drupal_set_message(t('There are no edit methods specified for this datastream.'));
      drupal_goto("islandora/object/{$object->id}/manage/datastreams");
      break;

    case 1:
      // One registry implementation, go there.
      drupal_goto($edit_registry[0]['url']);
      break;

    default:
      // Multiple edit routes registered.
      return islandora_edit_datastream_registry_render($edit_registry);
  }
}

/**
 * Displays links to all the edit datastream registry items.
 *
 * @param array $edit_registry
 *   A list of 'islandora_edit_datastream_registry' values.
 *
 * @return array
 *   A Drupal renderable array containing the "edit" markup.
 */
function islandora_edit_datastream_registry_render(array $edit_registry) {
  $markup = '';
  foreach ($edit_registry as $edit_route) {
    $markup .= l($edit_route['name'], $edit_route['url']) . '<br/>';
  }
  return array(
    '#type' => 'markup',
    '#markup' => $markup,
  );
}

/**
 * Get markup for a download link.
 *
 * @param AbstractDatastream $datastream
 *   The datastream for which to generate a link.
 *
 * @return string
 *   Either the link markup if the user has access or an empty string if the
 *   user is not allowed to see the given datastream.
 */
function islandora_datastream_get_download_link(AbstractDatastream $datastream) {
  $message = islandora_deprecated('7.x-1.2', 'Use the "islandora_datastream_download_link" theme implementation.');
  trigger_error(filter_xss($message), E_USER_DEPRECATED);

  return theme('islandora_datastream_download_link', array(
    'datastream' => $datastream,
  ));
}

/**
 * Get markup for a view link.
 *
 * @param AbstractDatastream $datastream
 *   The datastream for which to generate a link.
 *
 * @return string
 *   Either the link markup if the user has access or a string containing the
 *   datastream ID if the user is not allowed to see the given datastream.
 */
function islandora_datastream_get_view_link(AbstractDatastream $datastream) {
  $message = islandora_deprecated('7.x-1.2', 'Use the "islandora_datastream_view_link" theme implementation.');
  trigger_error(filter_xss($message), E_USER_DEPRECATED);

  return theme('islandora_datastream_view_link', array(
    'datastream' => $datastream,
  ));
}
