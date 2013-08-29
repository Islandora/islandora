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
  // XXX: Certain features of the Devel module rely on the use of "shutdown
  // handlers", such as query logging... The problem is that they might blindly
  // add additional output which will break things if what is actually being
  // output is anything but a webpage... like an image or video or audio or
  // whatever the datastream is here.
  $GLOBALS['devel_shutdown'] = FALSE;

  if ($version !== NULL) {
    if (isset($datastream[$version])) {
      $datastream = $datastream[$version];
    }
    else {
      return drupal_not_found();
    }
  }

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

  $cache_check = islandora_view_datastream_cache_check($datastream);
  if ($cache_check !== 200) {
    if ($cache_check === 304) {
      header('HTTP/1.1 304 Not Modified');
    }
    elseif ($cache_check === 412) {
      header('HTTP/1.0 412 Precondition Failed');
    }
  }
  islandora_view_datastream_set_cache_headers($datastream);

  drupal_page_is_cacheable(FALSE);
  // Try not to load the file into PHP memory!
  // Close and flush ALL the output buffers!
  while (@ob_end_flush()) {
  };

  // New content needed.
  if ($cache_check === 200) {
    $datastream->getContent('php://output');
  }
  exit();
}

/**
 * Parse "etags" from HTTP If-Match or If-None-Match headers.
 *
 * Parses from the CSV-like struture supported by HTTP headers into an array,
 * so `"asdf", "fdsa", W/"2132"` should become an array containing the strings:
 * - asdf
 * - fdsa
 * - 2132
 *
 * @see http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.24
 * @see http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.26
 *
 * @param string $header_value
 *   The value from the headers.
 *
 * @return array
 *   An array containing all the etags present.
 */
function islandora_parse_http_match_headers($header_value) {
  $matches = array();
  // Match the CSV-like structure supported by the HTTP headers.
  $count = preg_match_all('/(((W\/)?("?)(\*|.+?)\4)(, +)?)/', $header_value, $matches);
  // The fifth sub-expression/group is which will contain the etags.
  return $matches[5];
}

/**
 * Validate cache headers.
 *
 * @param AbstractDatastream $datastream
 *   The datastream for which to check the request headers against.
 *
 * @return int
 *   An integer representing the HTTP response code. One of:
 *   - 200: Proceed as normal. (Full download).
 *   - 304: Resource hasn't changed; pass cache validation.
 *   - 412: Resource has changed; fail cache validation.
 *
 * @see http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
 */
function islandora_view_datastream_cache_check(AbstractDatastream $datastream) {
  if (!variable_get('islandora_use_datastream_cache_headers', TRUE)) {
    return 200;
  }

  // Let's assume that if we get here, we'll be able to complete the request.
  $return = 200;

  if (isset($_SERVER['HTTP_IF_MODIFIED_SINCE'])) {
    $modified_since = DateTime::createFromFormat('D, d M Y H:i:s e', $_SERVER['HTTP_IF_MODIFIED_SINCE']);
    if ($datastream->createdDate->getTimestamp() - $modified_since->getTimestamp() > 0) {
      // Changed!
      return $return;
    }
    else {
      $return = 304;
    }
  }
  if ($return === 200 && isset($_SERVER['HTTP_IF_UNMODIFIED_SINCE'])) {
    $unmodified_since = DateTime::createFromFormat('D, d M Y H:i:s e', $_SERVER['HTTP_IF_UNMODIFIED_SINCE']);
    if ($datastream->createdDate->getTimestamp() !== $unmodified_since->getTimestamp()) {
      // Changed!
      $return = 412;
    }
    else {
      return $return;
    }
  }

  // Only consider Etags we have provided.
  if (isset($datastream->checksum)) {
    $tags = array();
    foreach ($datastream as $offset => $version) {
      if (isset($version->checksum)) {
        $tags[$offset] = $version->checksum;
      }
    }

    if ($return === 200 && isset($_SERVER['HTTP_IF_MATCH'])) {
      $request_tags = islandora_parse_http_match_headers($_SERVER['HTTP_IF_MATCH']);
      if (in_array('*', $request_tags) || count(array_intersect($tags, $request_tags)) > 0) {
        // There's a match... Let things go ahead.
        return $return;
      }
      else {
        $return = 412;
      }
    }
    if (in_array($return, array(200, 304), TRUE) && isset($_SERVER['HTTP_IF_NONE_MATCH'])) {
      $request_tags = islandora_parse_http_match_headers($_SERVER['HTTP_IF_NONE_MATCH']);
      if (in_array('*', $request_tags) || count(array_intersect($tags, $request_tags)) > 0) {
        $return = 304;
      }
      else {
        $return = 200;
      }
    }
  }

  return $return;
}

/**
 * Set various HTTP headers for caching.
 *
 * @param AbstractDatastream $datastream
 *   The datastream being viewed/downloaded.
 */
function islandora_view_datastream_set_cache_headers(AbstractDatastream $datastream) {
  if (variable_get('islandora_use_datastream_cache_headers', TRUE)) {
    // Force cache revalidation.
    header('Expires: Sun, 19 Nov 1978 05:00:00 GMT');
    $cache_control = array();
    if ($datastream->parent->repository->api->connection->username == 'anonymous') {
      $cache_control[] = 'public';
    }
    else {
      $cache_control[] = 'private';
    }
    $cache_control[] = 'must-revalidate';
    $cache_control[] = 'max-age=0';
    header('Cache-Control: ' . implode(', ', $cache_control));
    header('Last-Modified: ' . $datastream->createdDate->format('D, d M Y H:i:s \G\M\T'));
    if (isset($datastream->checksum)) {
      header("Etag: \"{$datastream->checksum}\"");
    }
  }
  else {
    header_remove('Cache-Control');
    header_remove('Expires');
  }
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
