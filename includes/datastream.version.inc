<?php

/**
 * @file
 * The form displayed that shows the datastream version history.
 */

/**
 * Function that returns markup for the datastream versions page.
 */
function islandora_datastream_version_table($datastream) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $parent = $datastream->parent;
  drupal_set_title(t("@dsid Previous Versions", array('@dsid' => $datastream->id)));
  $audit_values = islandora_get_audit_trail($parent->id, $datastream->id);

  $header = array();
  $header[] = array('data' => t('Created Date'));
  $header[] = array('data' => t('Size'));
  $header[] = array('data' => t('Label'));
  $header[] = array('data' => t('Responsibility'));
  $header[] = array('data' => t('Mime type'));
  $header[] = array('data' => t('Operations'), 'colspan' => '2');
  $rows = array();

  foreach ($datastream as $version => $datastream_version) {
    $row = array();
    $reponsibility = $parent->owner;
    foreach ($audit_values as $audit_value) {
      $internal = $datastream_version->createdDate;
      if ($audit_value['date'] == $datastream_version->createdDate) {
        $reponsibility = $audit_value['responsibility'];
      }
    }
    $user = user_load_by_name($reponsibility);
    if ($user) {
      $user_id = $user->uid;
      $user_val = l($reponsibility, "user/$user_id");
    }
    else {
      $user_val = $reponsibility;
    }
    $row[] = array(
      'class' => 'datastream-date',
      'data' => theme('islandora_datastream_view_link', array(
        'datastream' => $datastream,
        'label' => $datastream_version->createdDate->format(DATE_RFC850),
        'version' => $version,
      )),
    );
    $row[] = array(
      'class' => 'datastream-size',
      'data' => islandora_datastream_get_human_readable_size($datastream_version),
    );
    $row[] = array(
      'class' => 'datastream-label',
      'data' => $datastream_version->label,
    );
    $row[] = array(
      'class' => 'datastream-responsibility',
      'data' => $user_val,
    );
    $row[] = array(
      'class' => 'datastream-mime',
      'data' => $datastream_version->mimeType,
    );
    $row[] = array(
      'class' => 'datastream-delete',
      'data' => theme('islandora_datastream_delete_link', array(
        'datastream' => $datastream,
        'version' => $version,
      )),
    );
    $row[] = array(
      'class' => 'datastream-revert',
      'data' => theme('islandora_datastream_revert_link', array(
        'datastream' => $datastream,
        'version' => $version,
      )),
    );
    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * The admin delete datastream form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractDatastream $datastream
 *   The datastream to be deleted.
 * @param string $version
 *   The version number of the datastream we are trying to delete.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_delete_datastream_version_form(array $form, array &$form_state, AbstractDatastream $datastream, $version) {
  if (!isset($datastream[$version]) || count($datastream) < 2) {
    return drupal_not_found();
  }

  $form_state['datastream'] = $datastream;
  $form_state['version'] = $version;
  return confirm_form($form,
    t('Are you sure you want to delete version @version of the @dsid datastream?', array('@dsid' => $datastream->id, '@version' => $version)),
    "islandora/object/{$datastream->parent->id}",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for the delete datastream form.
 *
 * Purges/Delete's the given AbstractDatastream if possible.
 *
 * The ISLANDORA_PRE_PURGE_DATASTREAM_HOOK will query other modules as to
 * whether the given FedoraDatastream
 * should be: blocked from purging; state set to 'Deleted'; or purged.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_delete_datastream_version_form_submit(array $form, array &$form_state) {
  $datastream = $form_state['datastream'];
  $version = $form_state['version'];

  $datastream_id = $datastream->id;
  $object = $datastream->parent;

  try {
    unset($datastream[$version]);
  }
  catch (Exception $e) {
    drupal_set_message(t('Error deleting version %v of %s datastream from object %o %e', array(
          '%v' => $version,
          '%s' => $datastream_id,
          '%o' => $object->label,
          '%e' => $e->getMessage())), 'error');
  }

  drupal_set_message(t('%d datastream version successfully deleted from Islandora object %o', array(
        '%d' => $datastream_id,
        '%o' => $object->label)));

  $form_state['redirect'] = "islandora/object/{$object->id}/datastream/{$datastream->id}/version";
}

/**
 * The admin revert datastream form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractDatastream $datastream
 *   The datastream to be deleted.
 * @param string $version
 *   The version number of the datastream we are trying to revert to.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_revert_datastream_version_form(array $form, array &$form_state, AbstractDatastream $datastream, $version) {
  if (!isset($datastream[$version]) || count($datastream) < 2) {
    return drupal_not_found();
  }

  $form_state['dsid'] = $datastream->id;
  $form_state['object_id'] = $datastream->parent->id;
  $form_state['version'] = $version;

  return confirm_form($form,
    t('Are you sure you want to revert to version @version of the @dsid datastream?', array('@dsid' => $datastream->id, '@version' => $version)),
    "islandora/object/{$datastream->parent->id}",
    "",
    t('Revert'),
    t('Cancel')
  );
}

/**
 * Submit handler for the revert datastream form.
 *
 * Reverts the given AbstractDatastream if possible.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_revert_datastream_version_form_submit(array $form, array &$form_state) {
  $islandora_object = islandora_object_load($form_state['object_id']);

  $datastream_to_revert = $islandora_object[$form_state['dsid']];
  $version = $form_state['version'];

  // Create file holding specified datastream version, and set datastream to it.
  $datastream_to_revert_to = $datastream_to_revert[$version];
  if (in_array($datastream_to_revert->controlGroup, array('R', 'E'))) {
    $datastream_to_revert->url = $datastream_to_revert_to->url;
  }
  else {
    $filename = file_create_filename('datastream_temp_file', 'temporary://');
    $datastream_to_revert_to->getContent($filename);
    $datastream_to_revert->setContentFromFile($filename);
    file_unmanaged_delete($filename);
  }

  if ($datastream_to_revert->mimeType != $datastream_to_revert_to->mimeType) {
    $datastream_to_revert->mimeType = $datastream_to_revert_to->mimeType;
  }
  if ($datastream_to_revert->label != $datastream_to_revert_to->label) {
    $datastream_to_revert->label = $datastream_to_revert_to->label;
  }

  drupal_set_message(t('%d datastream successfully reverted to version %v for Islandora object %o', array(
        '%d' => $datastream_to_revert->id,
        '%v' => $version,
        '%o' => $islandora_object->label)));

  $form_state['redirect'] = "islandora/object/{$islandora_object->id}/datastream/{$datastream_to_revert->id}/version";
}

/**
 * Process available dsids, mime and extensions for a given object.
 *
 * @param AbstractObject $object
 *   The FedoraObject to process available extensions
 *
 * @return array()
 *   An associative array, merged from calls to
 *   islandora_get_datastreams_requirements_from_content_models()
 *   and an objects dsid's.
 */
function islandora_get_object_extensions(AbstractObject $object) {
  $extensions = islandora_get_datastreams_requirements_from_models($object->models);
  foreach ($object as $datastream) {
    // Could be a datastream not associated in a content model,
    // such as user added.
    if (!isset($extensions[$datastream->id])) {
      // Add the extensions manually.
      $extensions[$datastream->id] = array(
        'id' => $datastream->id,
        'optional' => TRUE,
        'mime' => array($datastream->mimeType),
      );
    }
  }
  return $extensions;
}

/**
 * The admin replace datastream form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractDatastream $datastream
 *   The datastream to be updated.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_datastream_version_replace_form($form, &$form_state, AbstractDatastream $datastream) {
  module_load_include('inc', 'islandora', 'includes/content_model');
  module_load_include('inc', 'islandora', 'includes/utilities');

  $object = islandora_object_load($datastream->parent->id);
  $form_state['object_id'] = $object->id;
  $form_state['dsid'] = $datastream->id;
  $form_state['object'] = $object;
  $datastream_mime_map = islandora_get_object_extensions($object);
  $mime_detect = new MimeDetect();
  $ext = array();
  if (isset($datastream_mime_map[$datastream->id])) {
    foreach ($datastream_mime_map[$datastream->id]['mime'] as $key => $value) {
      $extensions = $mime_detect->getValidExtensions($value);
      $ext = array_merge($ext, $extensions);
    }
  }
  $valid_extensions = implode(' ', $ext);
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  return array(
    'dsid_fieldset' => array(
      '#type' => 'fieldset',
      '#title' => t("Update datastream with latest version"),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      'dsid' => array(
        '#type' => 'markup',
        '#markup' => "<div>DSID: <b>$datastream->id</b></div>",
      ),
      'label' => array(
        '#type' => 'markup',
        '#markup' => "<div>Label: <b>$datastream->label</b></div>",
      ),
      'file' => array(
        '#type' => 'managed_file',
        '#required' => TRUE,
        '#title' => t('Upload Document'),
        '#size' => 64,
        '#description' => t('Select a file to upload.<br/>Files must be less than <strong>@size MB.</strong>', array('@size' => $upload_size)),
        '#upload_location' => 'temporary://',
        '#upload_validators' => array(
          'file_validate_extensions' => array($valid_extensions),
          // Assume its specified in MB.
          'file_validate_size' => array($upload_size * 1024 * 1024),
        ),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Add Contents'),
      ),
    ),
  );
}

/**
 * Submit handler for the replace datastream form.
 *
 * Adds a new datastream version as latest.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_datastream_version_replace_form_submit($form, &$form_state) {
  $object = islandora_object_load($form_state['object_id']);
  $form_state['redirect'] = "islandora/object/{$object->id}";
  $file = file_load($form_state['values']['file']);
  try {
    $ds = $object[$form_state['dsid']];
    $ds->mimetype = $file->filemime;
    $path = drupal_realpath($file->uri);
    $ds->setContentFromFile($path);
    file_delete($file);
  }
  catch (exception $e) {
    drupal_set_message(t('An error occurred during datastream updates. See watchlog for more information.'), 'error');
    watchdog('islandora',
      'Failed to add new versionable datastream.<br/>code: @code<br/>message: @msg',
      array(
        '@code' => $e->getCode(),
        '@msg' => $e->getMessage(),
      ),
      WATCHDOG_ERROR
    );
    file_delete($file);
    return;
  }
  drupal_set_message(t("Successfully Updated Datastream"));
}

/**
 * Gets Audit datastream values from foxml.
 *
 * @param string $pid
 *   PID of parent object
 *
 * @return array
 *   Array of audit values
 */
function islandora_get_audit_trail($pid, $dsid) {
  $url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
  $connection = islandora_get_tuque_connection(NULL, $url);
  $xml = $connection->api->m->getObjectXml($pid);
  $simple_xml = simplexml_load_string($xml);
  $fox_ns = "info:fedora/fedora-system:def/foxml#";
  $audit_ns = 'info:fedora/fedora-system:def/audit#';
  $foxml_nodes = $simple_xml->children($fox_ns);
  foreach ($foxml_nodes as $node) {
    if ($node->attributes()->ID == "AUDIT") {
      $content = $node->datastreamVersion->xmlContent;
      $audit_nodes = $content->children($audit_ns);
    }
  }
  $audit_values = array();
  if (isset($audit_nodes)) {
    foreach ($audit_nodes->auditTrail->record as $record) {
      if ($dsid == $record->componentID) {
        $values['responsibility'] = $record->responsibility;
        $values['date'] = $record->date;
        $audit_values[] = $values;
      }
    }
  }
  return $audit_values;
}
