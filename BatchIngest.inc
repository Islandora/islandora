<?php


/**
 * batch creation form submit
 * @global type $user
 * @param array $form
 * @param array $form_state
 * @param array $content_models
*/
function batch_creation_form(&$form_state, $collection_pid, $content_models) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
  $cm_options = array();
  $name_mappings = array();
  foreach ($content_models as $content_model) {
    if ($content_model->pid != "islandora:collectionCModel") {
      $cm_options[$content_model->pid] = $content_model->name;
      $name_mappings[] = $content_model->pid . '^' . $content_model->pid_namespace;
    }
  }
  $mappings = implode('~~~', $name_mappings);
  $form['#attributes']['enctype'] = 'multipart/form-data';

  $form['titlebox'] = array(
    '#type' => 'item',
    '#value' => t("Batch ingest into $collection_pid"),
  );

  $form['collection_pid'] = array(
    '#type' => 'hidden',
    '#value' => $collection_pid,
  );
  $form['namespace_mappings'] = array(
    '#type' => 'hidden',
    '#value' => $mappings,
  );
  $form['metadata_type'] = array(
    '#title' => "Choose Metadata Type",
    '#type' => 'radios',
    '#options' => array('MODS' => 'MODS', "DC" => "DUBLIN CORE"),
    '#required' => true,
    '#description' => t("Select primary metadata schema"),
  );
  $form['content_model'] = array(
    '#title' => "Choose content model to be associated with objects ingested",
    '#type' => 'select',
    '#options' => $cm_options,
    '#required' => true,
    '#description' => t("Content models describe the behaviours of objects with which they are associated."),
  );
  $form['indicator']['file-location'] = array(
    '#type' => 'file',
    '#title' => t('Upload zipped folder'),
    '#size' => 48,
    '#description' => t('The zipped folder should contain all files necessary to build objects.<br .>
      Related files must have the same filename, but with differing extensions to indicate mimetypes.<br />
      ie. <em>myFile.xml</em> and <em>myFile.jpg</em>'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create Objects ')
  );


  return($form);
}

/**
 * @param array $form
 * @param array $form_state
*/

function batch_creation_form_validate($form, &$form_state) {

  $fieldName = 'file-location';
  if (isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])) {
    $file = file_save_upload($fieldName);
    if ($file->filemime != 'application/zip') {
      form_set_error($fieldName, 'Input file must be a .zip file');
      return;
    }
    if (!$file) {
      form_set_error($fieldName, 'Error uploading file.');
      return;
    }
    $form_state['values']['file'] = $file;
  }
  else {
    // set error
    form_set_error($fieldName, 'Error uploading file.');
    return;
  }
}

function batch_creation_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'ContentModel');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  global $user;
  $namespace_mappings = array();
  $content_model = $form_state['values']['content_model'];
  $metadata = $form_state['values']['metadata_type'];
  $collection_pid = $form_state['values']['collection_pid'];
  $namespace_process = explode('~~~', $form_state['values']['namespace_mappings']);
  foreach ($namespace_process as $line) {
    $line_parts = explode('^', $line);
    $namespace_mappings[$line_parts[0]] = $line_parts[1];
  }
  $namespace = $namespace_mappings[$content_model];
  $namespace = preg_replace('/:.*/', '', $namespace);
  $dirName = "temp" . $user->uid;
  $tmpDir = file_directory_path() . '/' . $dirName . '/';
  mkdir($tmpDir);
  $file = $form_state['values']['file'];
  $fileName = $file->filepath;
  $file_list = array();
  $cmdString = "unzip -q -o -d $tmpDir \"$fileName\"";
  system($cmdString, $retVal);
  $dirs = array();
  $doNotAdd = array('.', '..', '__MACOSX');
  array_push($dirs, $tmpDir);
  $files = scandir($tmpDir);
  foreach ($files as $file) {
    if (is_dir("$tmpDir/$file") & !in_array($file, $doNotAdd)) {
      array_push($dirs, $tmpDir . $file);
    }
  }
  foreach ($dirs as $directory) {
    if ($inputs = opendir($directory)) {
      while (FALSE !== ($file_name = readdir($inputs))) {
        if (!in_array($file_name, $doNotAdd) && is_dir($file_name) == FALSE) {
          $ext = strrchr($file_name, '.');
          $base = preg_replace("/$ext$/", '', $file_name);
          $ext = substr($ext, 1);
          if ($ext) {
            $file_list[$base][$ext] = "$directory/" . $file_name;
          }
        }
      }
      closedir($inputs);
    }
  }

  if (($cm = ContentModel::loadFromModel($content_model, 'ISLANDORACM')) === FALSE) {
    drupal_set_message("$content_model not found", "error");
    return;
  }

  $batch = array(
    'title' => 'Ingesting Objects',
    'operations' => array(),
    'file' => drupal_get_path('module', 'fedora_repository') . '/BatchIngest.inc',
  );


  foreach ($file_list as $label => $object_files) {
    if ($object_files['xml']) {
      $batch['operations'][] = array('create_batch_objects', array($label, $content_model, $object_files, $collection_pid, $namespace, $metadata));
    }
  }
  $batch['operations'][] = array('recursive_directory_delete', array($tmpDir));
  batch_set($batch);
  batch_process();
}

/**
 *
 * @param <string> $label
 * @param <string> $content_model
 * @param <array> $object_files
 * @param <string> $collection_pid
 * @param <string> $namespace
 * @param <string> $metadata
 */


function create_batch_objects($label, $content_model, $object_files, $collection_pid, $namespace, $metadata) {
  module_load_include('inc', 'fedora_repository', 'ContentModel');
  module_load_include('inc', 'fedora_repository', 'MimeClass');
  $cm = ContentModel::loadFromModel($content_model, 'ISLANDORACM');
  $allowedMimeTypes = $cm->getMimetypes();
  $mime_helper = new MimeClass();
  $pid = fedora_item::get_next_PID_in_namespace($namespace);
  module_load_include('inc', 'fedora_reppository', 'api/fedora_item');
  $item = Fedora_item::ingest_new_item($pid, 'A', $label, $owner);
  $item->add_relationship('hasModel', $content_model, FEDORA_MODEL_URI);
  $item->add_relationship('isMemberOfCollection', $collection_pid);
  if ($metadata == 'DC') {
    $dc_xml = file_get_contents($object_files['xml']);
    $item->modify_datastream_by_value($dc_xml, 'DC', "Dublin Core", 'text/xml');
  }
  if ($metadata == 'MODS') {
    $mods_xml = file_get_contents($object_files['xml']);
    $item->add_datastream_from_string($mods_xml, 'MODS');
    $dc_xml = batch_create_dc_from_mods($mods_xml);
  }
  unset($object_files['xml']);
  $use_primary = TRUE;
  foreach ($object_files as $ext => $filename) {
    $file_mimetype = $mime_helper->get_mimetype($filename);
    if (in_array($file_mimetype, $allowedMimeTypes)) {
      $added = $cm->execIngestRules($filename, $file_mimetype);
    }
    else {
      $item->purge("$pid $label not ingested.  $file_mimetype not permitted in objects associated with $content_model");
      continue;
    }
    $ds_label = $use_primary ? $cm->getDatastreamNameDSID():$ext;
    $item->add_datastream_from_file($filename, $ds_label);
    $use_primary = FALSE;

    if (!empty($_SESSION['fedora_ingest_files'])) {
      foreach ($_SESSION['fedora_ingest_files'] as $dsid => $datastream_file) {
        $item->add_datastream_from_file($datastream_file, $dsid);
      }
    }
  }
}

/**
 * transforms mods to dc
 * @param <string> $mods_xml
 * @return <string>
 */



function batch_create_dc_from_mods($mods_xml) {
  $path = drupal_get_path('module', 'fedora_repository');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  module_load_include('inc', 'fedora_repository', 'CollectionClass');

  if ($xmlstr == NULL || strlen($xmlstr) < 5) {
    return " ";
  }

  try {
    $proc = new XsltProcessor();
  } catch (Exception $e) {
    drupal_set_message(t("!e", array('!e' => $e->getMessage())), 'error');
    return " ";
  }

  $xsl = new DomDocument();
  $xsl->load($path . '/xsl/mods_to_dc.xsl');
  $input = new DomDocument();
  $input->loadXML(trim($xmlstr));
  $xsl = $proc->importStylesheet($xsl);
  $newdom = $proc->transformToDoc($input);
  $dc_xml = $newdom->saveXML();

  return $dc_xml;
}

/**
 *
 * @param <string> $dir
 * @return <boolean>
 */
function recursive_directory_delete($dir) {
  if (!file_exists($dir))
    return true;
  if (!is_dir($dir))
    return unlink($dir);
  foreach (scandir($dir) as $item) {
    if ($item == '.' || $item == '..')
      continue;
    if (!recursive_directory_delete($dir . DIRECTORY_SEPARATOR . $item))
      return false;
  }
  return rmdir($dir);
}