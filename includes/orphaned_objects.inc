<?php

/**
 * @file
 * A list of orphaned Islandora objects.
 */

/**
 * Builds the Orphaned Islandora Objects management form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   An array containing the form to be rendered.
 */
function islandora_manage_orphaned_objects_form($form, $form_state) {
  if (isset($form_state['show_confirm'])) {
    $pids = $form_state['pids_to_delete'];
    $form['confirm_message'] = array(
      '#type' => 'item',
      '#markup' => format_plural(count($form_state['pids_to_delete']),
      'Are you sure you want to delete this object? This action cannot be reversed.',
      'Are you sure you want to delete these @count objects? This action cannot be reversed.'),
    );
    if (count($pids) <= 10) {
      $form['pids_to_delete'] = array(
        '#type' => 'markup',
        '#theme' => 'item_list',
        '#list_type' => 'ol',
      );
      $options = array('attributes' => array('target' => '_blank'));
      foreach ($pids as $pid) {
        $form['pids_to_delete']['#items'][] = l($pid, "/islandora/object/{$pid}", $options);
      }
    }
    $form['confirm_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Confirm'),
      '#weight' => 2,
      '#submit' => array('islandora_manage_orphaned_objects_confirm_submit'),
    );
    $form['cancel_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#weight' => 3,
    );
  }
  else {
    drupal_set_message(t('This page lists objects that have at least one parent, according to their RELS-EXT, that does not
    exist in the Fedora repository. These orphans might exist due to a failed batch ingest, their parents being deleted,
    or a variety of other reasons. Some of these orphans may exist intentionally.
    Please be cautious when deleting, as this action is irreversible.'), 'warning');
    $orphaned_objects = islandora_get_orphaned_objects();
    $rows = array();
    foreach ($orphaned_objects as $orphaned_object) {
      $pid = $orphaned_object['object']['value'];
      if (islandora_namespace_accessible($pid)) {
        $rows[$pid] = array(l($orphaned_object['title']['value'] . " (" . $pid . ")", "islandora/object/$pid"));
      }
    }
    ksort($rows);
    $form['management_table'] = array(
      '#type' => 'tableselect',
      '#header' => array(t('Object')),
      '#options' => $rows,
      '#attributes' => array(),
      '#empty' => t('No orphaned objects were found.'),
    );
    if (!empty($rows)) {
      $form['submit_selected'] = array(
        '#type' => 'submit',
        '#name' => 'islandora-orphaned-objects-submit-selected',
        '#validate' => array('islandora_delete_selected_orphaned_objects_validate'),
        '#submit' => array('islandora_delete_orphaned_objects_submit'),
        '#value' => t('Delete Selected'),
      );
      $form['submit_all'] = array(
        '#type' => 'submit',
        '#name' => 'islandora-orphaned-objects-submit-all',
        '#submit' => array('islandora_delete_orphaned_objects_submit'),
        '#value' => t('Delete All'),
      );
    }
  }
  return $form;
}

/**
 * Validation for the Islandora Orphaned Objects management form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_delete_selected_orphaned_objects_validate($form, $form_state) {
  $selected = array_filter($form_state['values']['management_table']);
  if (empty($selected)) {
    form_error($form['management_table'], t('At least one object must be selected to delete!'));
  }
}
/**
 * Submit handler for the delete buttons in the workflow management form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_delete_orphaned_objects_submit(&$form, &$form_state) {
  if ($form_state['triggering_element']['#name'] == 'islandora-orphaned-objects-submit-selected') {
    $selected = array_keys(array_filter($form_state['values']['management_table']));
  }
  else {
    $selected = array_keys($form_state['values']['management_table']);
  }
  $form_state['pids_to_delete'] = $selected;
  // Rebuild to show the confirm form.
  $form_state['rebuild'] = TRUE;
  $form_state['show_confirm'] = TRUE;
}
/**
 * Submit handler for the workflow management confirm form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_manage_orphaned_objects_confirm_submit($form, &$form_state) {
  $batch = islandora_delete_orphaned_objects_create_batch($form_state['pids_to_delete']);
  batch_set($batch);
}

/**
 * Query for orphaned objects.
 *
 * @return array
 *   An array containing the results of the orphaned objects queries.
 */
function islandora_get_orphaned_objects() {
  $connection = islandora_get_tuque_connection();
  // SPARQL: get orphaned objects, exclude any with a living parent.
  $object_query = <<<EOQ
!prefix
SELECT DISTINCT ?object ?title 
WHERE {
  ?object <fedora-model:hasModel> <info:fedora/fedora-system:FedoraObject-3.0> ;
               ?p ?otherobject .
   ?object <fedora-model:label> ?title;
  OPTIONAL {
    ?otherobject <fedora-model:hasModel> ?model .
  } .
  FILTER (!bound(?model))

  # Filter by "parent" relationships
  FILTER (!dead_parent_relationships)
 
  # Exclude objects with live parents
  OPTIONAL {
    !live_parent_relationships .
    ?liveparent <fedora-model:hasModel> <info:fedora/fedora-system:FedoraObject-3.0> .
  }
  !optionals
  !filters
  FILTER (!bound(?liveparent))
} ORDER BY ?object
EOQ;
  $parent_relationships = module_invoke_all('islandora_solution_pack_child_relationships', 'all');
  $parent_relationships['prefix'] = array_unique($parent_relationships['prefix']);
  $parent_relationships['predicate'] = array_unique($parent_relationships['predicate']);
  if (count($parent_relationships['predicate']) == 0) {
    // No predicates to search for. Exit early.
    return array();
  }
  $optionals = (array) module_invoke('islandora_xacml_api', 'islandora_basic_collection_get_query_optionals', 'view');
  $filter_modules = array(
    'islandora_xacml_api',
    'islandora',
  );
  $filters = array();
  foreach ($filter_modules as $module) {
    $filters = array_merge_recursive($filters, (array) module_invoke($module, 'islandora_basic_collection_get_query_filters', 'view'));
  }
  $filter_map = function ($filter) {
    return "FILTER($filter)";
  };
  $parent_map = function ($parent) {
    return "?object $parent ?liveparent";
  };
  // Use separate queries for different object types.
  $sparql_query_objects = format_string($object_query, array(
    '!optionals' => !empty($optionals) ? ('OPTIONAL {{' . implode('} UNION {', $optionals) . '}}') : '',
    '!filters' => !empty($filters) ? implode(' ', array_map($filter_map, $filters)) : '',
    '!dead_parent_relationships' => '?p = ' . implode(' || ?p = ', $parent_relationships['predicate']),
    '!live_parent_relationships' => '{' . implode(' } UNION { ', array_map($parent_map, $parent_relationships['predicate'])) . '}',
    '!prefix' => implode("\n", $parent_relationships['prefix']),
  ));
  $results = $connection->repository->ri->sparqlQuery($sparql_query_objects);
  return $results;
}

/**
 * Constructs the batch that will go out and delete objects.
 *
 * @param array $pids
 *   The array of pids to be deleted.
 *
 * @return array
 *   An array detailing the batch that is about to be run.
 */
function islandora_delete_orphaned_objects_create_batch($pids) {
  // Set up a batch operation.
  $batch = array(
    'operations' => array(
      array('islandora_delete_orphaned_objects_batch_operation', array($pids)),
    ),
    'title' => t('Deleting the selected objects...'),
    'init_message' => t('Preparing to delete objects.'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'finished' => 'islandora_delete_orphaned_objects_batch_finished',
    'file' => drupal_get_path('module', 'islandora') . '/includes/orphaned_objects.inc',
  );
  return $batch;
}
/**
 * Constructs and performs the deleting batch operation.
 *
 * @param array $pids
 *   An array of pids to be deleted.
 * @param array $context
 *   The context of the Drupal batch.
 */
function islandora_delete_orphaned_objects_batch_operation($pids, &$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['pids'] = $pids;
    $context['sandbox']['total'] = count($pids);
    $context['results']['success'] = array();
  }
  if (!empty($context['sandbox']['pids'])) {
    $target_pid = array_pop($context['sandbox']['pids']);
    $target_object = islandora_object_load($target_pid);
    $context['message'] = t('Deleting @label (@pid) (@current of @total)...', array(
        '@label' => $target_object->label,
        '@pid' => $target_pid,
        '@current' => $context['sandbox']['progress'],
        '@total' => $context['sandbox']['total'],
    ));
    islandora_delete_object($target_object);
    $object_check = islandora_object_load($target_pid);
    if ($object_check) {
      drupal_set_message(t('Could not delete %pid. You may not have permission to manage this object.',
        array(
          '%pid' => $target_pid,
        )), 'error');
    }
    else {
      $context['results']['success'][] = $target_pid;
    }
    $context['sandbox']['progress']++;
  }
  $context['finished'] = ($context['sandbox']['total'] == 0) ? 1 : ($context['sandbox']['progress'] / $context['sandbox']['total']);
}
/**
 * Finished function for the orphaned objects delete batch.
 *
 * @param bool $success
 *   Whether the batch was successful or not.
 * @param array $results
 *   An array containing the results of the batch operations.
 * @param array $operations
 *   The operations array that was used in the batch.
 */
function islandora_delete_orphaned_objects_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results['success']), 'One object deleted.', '@count objects deleted.');
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
}
