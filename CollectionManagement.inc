<?php

/**
 * collection creation form 
 * @param array $form_state
 * @param string $parent_collection_pid
 * @param string $content_models
 * @return array
 */
function collection_management_form(&$form_state, $this_collection_pid, $content_models) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  $restricted = FALSE;
  if (variable_get('fedora_namespace_restriction_enforced', true)) {
    $restricted = true;
    $allowed_string = variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora:');
    $namespaces = explode(':', $allowed_string);
    foreach ($namespaces as $namespace) {
      if ($namespace) {
        $allowed[trim($namespace)] = trim($namespace);
      }
    }
  }
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
  $item = new Fedora_Item($this_collection_pid);
  $collection_name = $item->objectProfile->objLabel;
  $new_content_models = get_content_models_as_option_array();
  $cm_options = array();
  $name_mappings = array();
  foreach ($content_models as $content_model) {
    if ($content_model->pid != "islandora:collectionCModel") {
      $item = new fedora_item($content_model->pid);
      $cm_name = $item->objectProfile->objLabel;
      $cm_options[$content_model->pid] = $cm_name;
    }
  }
  if (!empty($cm_options)) {
    $show_delete = true;
  }


  $content_models = get_content_models_as_option_array();
  $form['child_creation'] = array(
    '#title' => "Create Child Collection",
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['child_creation']['titlebox'] = array(
    '#type' => 'item',
    '#value' => t("Create New Child Collection within $this_collection_pid"),
  );

  $form['child_creation']['collection_name'] = array(
    '#title' => "Collection Name",
    '#type' => 'textfield',
    '#description' => t("Human readable name for this collection"),
  );

  $form['child_creation']['new_collection_pid'] = array(
    '#title' => "Collection PID",
    '#type' => 'textfield',
    '#size' => 15,
    '#description' => t("Unique PID for this collection.  <br />Pids take the general form of namespace:collection eg islandora:pamphlets"),
  );
  if (!$restricted) {
    $form['child_creation']['collection_namespace'] = array(
      '#title' => "Collection Namespace",
      '#type' => 'textfield',
      '#size' => 15,
      '#default_value' => 'default',
      '#description' => t("Namespace for objects in this collection."),
    );
  }
  else {
    $form['child_creation']['collection_namespace'] = array(
      '#title' => "Collection Namespace",
      '#type' => 'select',
      '#options' => $allowed,
      '#default_value' => 'default',
      '#description' => t("Namespace for objects in this collection."),
    );
  }
  $form['parent_collection'] = array(
    '#type' => 'hidden',
    '#value' => $this_collection_pid,
  );
  
  $form['collection_pid'] = array(
    '#type' => 'hidden',
    '#value' => $this_collection_pid,
  );
  $form['child_creation']['content_models'] = array(
    '#title' => "Choose allowable content models for this collection",
    '#type' => 'checkboxes',
    '#options' => $content_models,
    '#description' => t("Content models describe the behaviours of objects with which they are associated."),
  );


  $form['child_creation']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create Collection'),
    '#id' => 'create_class'
  );
  $form['manage_collection_policy'] = array(
    '#title' => "Manage Collection Policies",
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['manage_collection_policy']['add'] = array(
    '#title' => "Add Content Model",
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => $show_delete,
  );


  $form ['manage_collection_policy']['add']['content_model_to_add'] = array(
    '#title' => "Choose Content Model",
    '#type' => 'select',
    '#options' => array_diff_key($content_models, $cm_options),
    '#description' => t("Choose content model to add to this collection policy."),
  );

  $form ['manage_collection_policy']['add']['new_cp_namespace'] = array(
    '#title' => "Choose Namespace",
    '#type' => 'textfield',
    '#size' => 15,
    '#description' => t("Choose namespace for objects in this collection associated with this content model"),
  );
  $form['manage_collection_policy']['add']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Content Model to Collection Policy'),
    '#id' => 'add_cm'
  );

  if ($show_delete) {
    $form['manage_collection_policy']['remove'] = array(
      '#title' => "Delete Content Model",
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form ['manage_collection_policy']['remove']['content_models_to_remove'] = array(
      '#title' => "Choose Content Model to Remove",
      '#type' => 'checkboxes',
      '#options' => $cm_options,
      '#description' => t("Choose content models to remove from this collection policy."),
    );


    $form['manage_collection_policy']['remove']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Remove Content Collection Policy'),
      '#id' => 'remove_cm'
    );
  }
  $form['change_cmodel'] = array(
    '#title' => "Change Content Models",
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['change_cmodel']['titlebox'] = array(
    '#type' => 'item',
    '#value' => t("Change Content Models within $this_collection_pid"),
  );

  $form['change_cmodel']['current_content_model'] = array(
    '#title' => "Choose content model to be changed",
    '#type' => 'select',
    '#options' => $cm_options,
    '#description' => t("All objects in this collection with the selected content model will be changed."),
  );
  $form['change_cmodel']['new_content_model'] = array(
    '#title' => "Choose new content model",
    '#type' => 'select',
    '#options' => $new_content_models,
    '#description' => t("The new content model to be assigned to selected objects."),
  );
  $form['change_cmodel']['collection_pid'] = array(
    '#type' => 'hidden',
    '#value' => $this_collection_pid,
  );
  $form['change_cmodel']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Change Content Model Associations'),
    '#id' => 'change_model',
  );
  if (user_access('delete entire collections')) {
    $form['delete_collection'] = array(
      '#title' => "Permanently Delete $collection_name",
      '#type' => 'fieldset',
      '#description' => t("Clicking this button will delete all objects within $collection_name. <br /> <strong>This action cannot be undone.</strong>"),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['delete_collection']['confirm'] = array(
      '#title' => "Are you sure?",
      '#type' => 'fieldset',
      '#description' => t('<strong>Clicking the delete button will permanantly remove all objects from this collection. <br />  <strong>This action cannot be undone.</strong> '),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['delete_collection']['confirm']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete this collection'),
      '#id' => 'delete_collection',
    );
  }
  return($form);
}

/**
 * collection creation form validate 
 * @param array $form
 * @param array $form_state
 */
function collection_management_form_validate($form, &$form_state) {
  if ($form_state['clicked_button']['#id'] == 'create_class') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    $pid = $form_state['values']['new_collection_pid'];
    $item = new fedora_item($pid);
    if ($item->exists()) {
      form_set_error('new_collection_pid', t("$pid already exists within your repository. the PID must be unique"));
      return;
    }
    if (!valid_pid($pid)) {
      form_set_error('new_collection_pid', t("$pid is not a valid identifier"));
      return;
    }
  }
  if ($form_state['clicked_button']['#id'] == 'add_cm') {
    if (!valid_pid($form_state['values']['new_cp_namespace'])) {
      form_set_error('new_cp_namespace', t("Namespace must be a valid PID"));
      return;
    }
  }
}

/**
 * collection creation form submit
 * @global type $user
 * @param type $form
 * @param type $form_state
 */
function collection_management_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_collection');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/dublin_core');
  global $user;
  $collection_pid = $form_state['values']['parent_collection'];
  $policy = CollectionPolicy::loadFromCollection($collection_pid, TRUE);
  if ($form_state['clicked_button']['#id'] == 'create_class') {
    $module_path = drupal_get_path('module', 'fedora_repository');
    $thumbnail = drupal_get_path('module', 'Fedora_Repository') . '/images/Crystal_Clear_filesystem_folder_grey.png';
    $new_collection_pid = $form_state['values']['new_collection_pid'];
    $new_collection_label = $form_state['values']['collection_name'];
    $pid_namespace = $form_state['values']['collection_namespace'];
    $all_cModels = get_content_models_as_option_array();
    $collection_policy = '<?xml version="1.0" encoding="UTF-8"?>
<collection_policy xmlns="http://www.islandora.ca" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" name="" xsi:schemaLocation="http://www.islandora.ca http://syn.lib.umanitoba.ca/collection_policy.xsd">
    <content_models>
    </content_models>
    <search_terms>
    </search_terms>
    <staging_area></staging_area>
    <relationship>isMemberOfCollection</relationship>
</collection_policy>';
    $content_models = $form_state['values']['content_models'];
    $collection_policy_xml = simplexml_load_string($collection_policy);
    foreach ($content_models as $content_model) {
      if ($content_model) {
        $node = $collection_policy_xml->content_models->addChild('content_model');
        $node->addAttribute('dsid', 'ISLANDORACM');
        $node->addAttribute('name', $all_cModels[$content_model]);
        $node->addAttribute('namespace', $pid_namespace . ':1');
        $node->addAttribute('pid', $content_model);
      }
    }
    $item = fedora_item::ingest_new_item($new_collection_pid, 'A', $new_collection_label, $user->name);
    $item->add_relationship('isMemberOfCollection', $collection_pid, RELS_EXT_URI);
    $item->add_relationship('hasModel', 'islandora:collectionCModel', FEDORA_MODEL_URI);
    $item->add_datastream_from_string($collection_policy_xml->saveXML(), 'COLLECTION_POLICY');
    $item->add_datastream_from_file($thumbnail, 'TN');
    drupal_goto("/fedora/repository/$new_collection_pid");
  }

  if ($form_state['clicked_button']['#id'] == 'add_cm') {

    $cp_namespace = $form_state['values']['new_cp_namespace'];
    $cp_content_model = $form_state['values']['content_model_to_add'];
    $policy->addModel(ContentModel::loadFromModel($cp_content_model), $cp_namespace);
    $policy->saveToFedora();
    drupal_set_message("Collection model successfully added");
  }

  if ($form_state['clicked_button']['#id'] == 'remove_cm') {
    $candidates = $form_state['values']['content_models_to_remove'];
    $count = 0;
    foreach ($candidates as $candidate) {
      if (is_string($candidate)) {
        $policy->removeModel(ContentModel::loadFromModel($candidate));
        $count++;
      }
    }
    if ($count > 0) {
      $policy->saveToFedora();
      if ($count > 1) {
        $s = 's';
      }
      drupal_set_message("$count collection model$s removed");
    }
  }

  if ($form_state['clicked_button']['#id'] == 'change_model') {
    $current_content_model = $form_state['values']['current_content_model'];
    $new_content_model = $form_state['values']['new_content_model'];

    $add_to_policy = TRUE;
    $policy_cms = $policy->getContentModels();
    foreach ($policy_cms as $policy_cm) {
      if ($policy_cm->pid == $current_content_model) {
        $namespace = $policy_cm->pid_namespace;
      }
      if ($policy_cm->pid == $new_content_model) {
        $add_to_policy = FALSE;
      }
    }
    if ($add_to_policy) {
      $policy->addModel(ContentModel::loadFromModel($new_content_model), $namespace);
      $policy->saveToFedora();
    }
    $query = "select \$object  from <#ri>
    where (\$object <info:fedora/fedora-system:def/model#hasModel> <info:fedora/$current_content_model>
      and \$object <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> <info:fedora/$collection_pid>
      and \$object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)";
    $query = htmlentities(urlencode($query));
    $content = '';

    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    $url .= "?type=tuples&flush=TRUE&format=csv&limit=$limit&offset=$offset&lang=itql&stream=on&query=" . $query;
    $content .= do_curl($url);
    $results = explode("\n", $content);
    $object_pids = preg_replace('/^info:fedora\/|"object"/', '', $results);
    $count = 0;
    foreach ($object_pids as $object_pid) {
      if (!$object_pid) {
        continue;
      }
      $item = new fedora_item($object_pid);
      $item->purge_relationship('hasModel', $current_content_model);
      $item->add_relationship('hasModel', $new_content_model, FEDORA_MODEL_URI);
      $count++;
    }
    drupal_set_message("$current_content_model changed to $new_content_model on $count objects");
  }

  if ($form_state['clicked_button']['#id'] == 'delete_collection') {
    $collection_name = $form_state['values']['collection_name'];
    $pids = get_related_items_as_array($collection_pid, 'isMemberOfCollection');

    $batch = array(
      'title' => "Deleting Objects from  $name",
      'operations' => array(),
      'file' => drupal_get_path('module', 'fedora_repository') . '/CollectionManagement.inc',
    );


    foreach ($pids as $pid) {

      $batch['operations'][] = array('delete_objects_as_batch', array($pid));
    }

    batch_set($batch);
    batch_process('/fedora/repository');
  }
}

/**
 * deletes PID if pid is not collection
 * @param <type> $pid
 */
function delete_objects_as_batch($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $name = $user->name;
  $item_to_delete = new Fedora_Item($pid);
  $models = $item_to_delete->get_models();
  foreach ($models as $model) {
    if ($model['object'] != 'islandora:collectionCModel') {
      $item_to_delete->purge("$object purged by $name");
    }
  }
}