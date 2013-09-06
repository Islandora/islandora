<?php

/**
 * @file
 * Handles the management of deleted objects.
 */

/**
 * The deletion management prep form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_deleted_objects_prep_form($form, $form_state, $serialized_chosen = NULL) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $chosen_contentmodels = array();
  if ($serialized_chosen) {
    $chosen_contentmodels = unserialize($serialized_chosen);
  }
  $contentmodels_with_deleted_members = islandora_get_contentmodels_with_deleted_members();
  $elegible_contentmodels = array_keys($contentmodels_with_deleted_members);
  if (empty($contentmodels_with_deleted_members)) {
    $form['message'] = array(
      '#type' => 'markup',
      '#markup' => t("There are no deleted objects in this repository."),
    );
    return $form;
  }
  $form['message'] = array(
    '#type' => 'markup',
    '#markup' => t("Select content models of deleted objects."),
  );
  $form['mapped_contentmodels'] = array(
    '#type' => 'hidden',
    '#value' => $contentmodels_with_deleted_members,
  );
  $table_element = islandora_content_model_select_table_form_element(NULL);

  foreach ($table_element['#options'] as $option) {
    if (!in_array($option['pid'], $elegible_contentmodels)) {
      unset($table_element['#options'][$option['pid']]);
    }
    if (array_key_exists($option['pid'], $chosen_contentmodels)) {
      $table_element['#default_value'][$option['pid']] = TRUE;
    }
  }

  $form['contentmodels'] = $table_element;
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );

  return $form;
}

/**
 * Submit handler for deletion management prep form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function islandora_deleted_objects_prep_form_submit($form, $form_state) {
  $content_models = $form_state['values']['contentmodels'];
  $chosen = function($element) {
    return $element;
  };
  $serialized_contentmodels = serialize(array_filter($content_models, $chosen));
  drupal_goto("admin/islandora/restore/manage/$serialized_contentmodels");
}

/**
 * The deletion management prep form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_deleted_objects_manage_form($form, $form_state, $serialized_chosen = NULL) {
  $form['previous'] = array(
    '#type' => 'submit',
    '#value' => t('Previous'),
    '#attributes' => array('source' => 'previous'),
  );

  $chosen_contentmodels = unserialize($serialized_chosen);
  $content_models_with_deleted = islandora_get_contentmodels_with_deleted_members();
  foreach ($chosen_contentmodels as $contentmodel) {
    if (!array_key_exists($contentmodel, $content_models_with_deleted)) {
      unset($chosen_contentmodels[$contentmodel]);
    }
  }

  if (empty($chosen_contentmodels)) {
    $form['message'] = array(
      '#type' => 'markup',
      '#markup' => t("There are no deleted objects with the selected content models in this repository."),
    );
    return $form;
  }

  if (is_array($chosen_contentmodels)) {
    foreach ($chosen_contentmodels as $key => $value) {
      if (in_array($key, $content_models_with_deleted)) {
        $chosen_contentmodels[$key] = $content_models_with_deleted[$key];
      }
    }
  }

  $tuque = islandora_get_tuque_connection();
  $repository = $tuque->repository;
  // Query brings back fedora-system:FedoraObject-3.0, doubling the results.
  $total = $repository->ri->countQuery(islandora_get_deleted_query($chosen_contentmodels), 'sparql') / 2;
  $limit = 25;
  if ($total < 28) {
    $limit = $total;
  }
  $current_page = pager_default_initialize($total, $limit);
  $query_limit = $limit * 2;
  $offset = $current_page * $query_limit;
  $options = islandora_get_deleted_objects($chosen_contentmodels, $query_limit, $offset);

  foreach ($options as &$option) {
    $option['content_model'] = $content_models_with_deleted[$option['content_model']];
  }
  $form['serialized_chosen'] = array(
    '#type' => 'hidden',
    '#value' => $serialized_chosen,
  );
  $form['pager'] = array(
    '#type' => 'markup',
    '#markup' => theme('pager', array('quantity', count($options))),
  );
  $form['propogate'] = array(
    '#title' => t('Apply changes to related objects?'),
    '#default_value' => TRUE,
    '#description' => t("Objects associated with selected objects will also be purged/restored. ie page objects associated with a book object."),
    '#type' => 'checkbox',
  );
  $form['chosen'] = array(
    '#type' => 'hidden',
    '#value' => $chosen_contentmodels,
  );
  $form['objects_to_process'] = array(
    '#type' => 'tableselect',
    '#header' => array(
      'title' => t('Name'),
      'pid' => t('PID'),
      'content_model' => t('Content Model'),
    ),
    '#multiple' => TRUE,
    '#options' => $options,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Restore selected objects'),
    '#attributes' => array('source' => 'restore'),
  );
  if (user_access(ISLANDORA_PURGE)) {
    $form['purge'] = array(
      '#type' => 'submit',
      '#value' => t('Irrevocably purge selected objects'),
      '#attributes' => array('source' => 'purge'),
    );
  }
  return $form;
}

/**
 * Submit handler for deletion management form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function islandora_deleted_objects_manage_form_submit($form, $form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $serialized_chosen = isset($form_state['values']['serialized_chosen']) ? $form_state['values']['serialized_chosen'] : NULL;

  if (isset($form_state['clicked_button']['#attributes']['source']) && $form_state['clicked_button']['#attributes']['source'] == 'previous') {
    drupal_goto("admin/islandora/restore/prep/$serialized_chosen");
  }
  if ($form_state['clicked_button']['#attributes']['source'] == 'restore') {
    $descriptor = "Restoring";
    $action = 'islandora_restore_object_by_pid';
  }
  if ($form_state['clicked_button']['#attributes']['source'] == 'purge') {
    $descriptor = "Purging";
    $action = 'islandora_purge_object_by_pid';
  }
  $objects_to_process = array_filter($form_state['values']['objects_to_process']);
  $pids_to_restore = $objects_to_process;
  if ($form_state['values']['propogate']) {
    foreach ($objects_to_process as $pid) {
      $fedora_object = islandora_object_load($pid);
      $temp = islandora_invoke_hook_list(ISLANDORA_UPDATE_RELATED_OBJECTS_PROPERTIES_HOOK, $fedora_object->models, array($fedora_object));
      if (!empty($temp)) {
        $pids_to_restore = array_merge_recursive($pids_to_restore, $temp);
      }
    }
  }
  $batch = array(
    'title' => t('@descriptor selected objects', array('@descriptor' => $descriptor)),
    'file' => drupal_get_path('module', 'islandora') . '/includes/manage_deleted_objects.inc',
    'operations' => array(),
  );

  foreach ($pids_to_restore as $pid) {
    $batch['operations'][] = array(
      $action,
      array($pid),
    );
  }
  batch_set($batch);
  batch_process("admin/islandora/restore/manage/$serialized_chosen");
}

/**
 * Gets PIDS of all deleted objects.
 *
 * @return array
 *   PIDS of deleted objects
 */
function islandora_get_deleted_objects($content_models, $limit, $offset) {
  $tuque = islandora_get_tuque_connection();
  $repository = $tuque->repository;
  $query = islandora_get_deleted_query($content_models, $offset);
  $objects = $repository->ri->sparqlQuery($query, $limit);
  $deleted_objects = array();
  foreach ($objects as $object) {
    if ($object['object']['value'] != "fedora-system:FedoraObject-3.0") {
      $pid = $object['subject']['value'];
      $cm_pid = $object['object']['value'];
      $title = $object['label']['value'];
      $deleted_objects[$pid] = array(
        'title' => $title, 'pid' => $pid,
        'content_model' => $content_models[$cm_pid],
      );
    }
  }
  return $deleted_objects;
}

/**
 * Gets PIDS of all content models associated with deleted objects.
 *
 * @return array
 *   array of content model pids
 */
function islandora_get_contentmodels_with_deleted_members() {
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $query = "PREFIX fm: <info:fedora/fedora-system:def/model#>
    SELECT DISTINCT ?object ?label FROM <#ri>
            WHERE {
                   {?subject fm:state fm:Deleted;
                             fm:hasModel ?object;
                   }
            OPTIONAL{
                        ?object fm:label  ?label
                     }
                  }";

  $objects = $repository->ri->sparqlQuery($query, -1);
  $content_models = array();
  foreach ($objects as $object) {
    if ($object['object']['value'] != "fedora-system:FedoraObject-3.0") {
      $content_models[$object['object']['value']] = $object['label']['value'];
    }
  }
  return $content_models;
}

/**
 * Restores deleted object.
 *
 * @param String $pid
 *   PID of object to be restored
 */
function islandora_restore_object_by_pid($pid) {
  $fedora_object = islandora_object_load($pid);
  $fedora_object->state = 'A';
}

/**
 * Purges deleted object.
 *
 * @param String $pid
 *   PID of object to be restored
 */
function islandora_purge_object_by_pid($pid) {
  $fedora_object = islandora_object_load($pid);
  $fedora_object->repository->purgeObject($pid);
}

/**
 * Get query to find all deleted objects by content type.
 *
 * @param array $content_models
 *   Content models to restrict search
 * @param int $offset
 *   offset to be added to search
 *
 * @return String
 *   Sparql query
 */
function islandora_get_deleted_query($content_models, $offset = 0) {
  $candidates = array_keys($content_models);
  $first_contentmodel = array_shift($candidates);
  $prefix = "PREFIX fm: <" . FEDORA_MODEL_URI . "> ";
  $select = "SELECT DISTINCT ?subject ?label ?object FROM <#ri> WHERE { ";
  $where_clause = "{?subject fm:hasModel <info:fedora/$first_contentmodel>;
                            fm:state fm:Deleted;
                            fm:hasModel ?object;
                   }";
  $suffix = "} ORDER BY ?subject OFFSET $offset";
  $unions = '';
  foreach ($candidates as $contentmodel) {
    $unions .= "UNION {?subject fm:hasModel <info:fedora/$contentmodel>;
                            fm:state fm:Deleted;
                            fm:hasModel ?object;
                   } 
                ";
  }
  $optional = "OPTIONAL{?subject fm:label ?label}";
  return "$prefix $select $where_clause $unions $optional $suffix";
}
