<?php
/**
 * @file
 * This file documents all available hook functions to manipulate data.
 */

/**
 * Generate a repository objects view.
 *
 * @param AbstractObject $object
 *   The object to display
 * @param object $user
 *   The user accessing the object.
 * @param string $page_number
 *   The page in the content.
 * @param string $page_size
 *   The size of the page.
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_islandora_view_object($object, $user, $page_number, $page_size) {
  $output = array();
  if (in_array('islandora:sp_basic_image', $object->models)) {
    $resource_url = url("islandora/object/{$object->id}/datastream/OBJ/view");
    $params = array(
      'title' => $object->label,
      'path' => $resource_url,
    );
    // Theme the image seperatly.
    $variables['islandora_img'] = theme('image', $params);
    $output = theme('islandora_default_print', array(
    'islandora_content' => $variables['islandora_img']));
  }
  return $output;
}

/**
 * Generate a print friendly page for the given object.
 *
 * @param object $object
 *   The object form to print.
 */
function hook_islandora_view_print_object($object) {

}

/**
 * Generate an object's display for the given content model.
 *
 * Content models PIDs have colons and hyphens changed to underscores, to
 * create the hook name.
 *
 * @param AbstractObject $object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_cmodel_pid_islandora_view_object($object) {

}

/**
 * Alter display output after it has been generated.
 *
 * @param AbstractObject $object
 *   A Tuque AbstractObject being operated on.
 * @param array $rendered
 *   The array of rendered views.
 */
function hook_islandora_view_object_alter(&$object, &$rendered) {

}

/**
 * Alter display output if the object has the given model.
 *
 * @see hook_islandora_view_object_alter()
 *
 * @param AbstractObject $object
 *   A Tuque AbstractObject being operated on.
 * @param array $rendered
 *   The array of rendered views.
 */
function hook_cmodel_pid_islandora_view_object_alter(&$object, &$rendered) {

}

/**
 * Generate an object's datastreams management display.
 *
 * @param AbstractObject $object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_islandora_edit_object($object) {

}

/**
 * Generate an object's datastreams management display based on content model.
 *
 * Content models PIDs have colons and hyphens changed to underscores, to
 * create the hook name.
 *
 * @param AbstractObject $object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_cmodel_pid_islandora_edit_object($object) {

}

/**
 * Allow datastreams management display output to be altered.
 *
 * @param AbstractObject $object
 *   A Tuque FedoraObject
 * @param array $rendered
 *   an arr of rendered views
 */
function hook_islandora_edit_object_alter(&$object, &$rendered) {

}

/**
 * Allows modules to alter the object or block/modify the given action.
 *
 * This alter hook will be called before any object is ingested, modified or
 * purged.
 *
 * Changing object properties such as "label", or "state", are considered
 * modifications, where as manipulating an object's datstreams are not.
 *
 * @param AbstractObject $object
 *   The object to alter.
 * @param array $context
 *   An associative array containing:
 *   - action: A string either 'ingest', 'purge', 'modify'.
 *   - block: Either TRUE or FALSE, if TRUE the action won't take place.
 *     Defaults to FALSE.
 *   - purge: Either TRUE or FALSE, only present when the action is 'purge'.
 *     If 'delete' or 'block' is set to TRUE, they will take precedence.
 *     Defaults to TRUE.
 *   - delete: Either TRUE or FALSE, only present when the action is 'purge'.
 *     If TRUE it will cause the object's state to be set to 'D' instead.
 *     If 'block' is set to TRUE, it will take precedence.
 *     Defaults to FALSE,
 *   - params: An associative array, only present when the action is 'modify'.
 *     The key value pairs represent what values will be changed. The params
 *     will match the same params as passed to FedoraApiM::modifyObject().
 *
 * @see FedoraApiM::modifyObject()
 */
function hook_islandora_object_alter(AbstractObject $object, array &$context) {

}

/**
 * Allows modules to alter the object or block/modify the given action.
 *
 * @see hook_islandora_object_alter()
 */
function hook_cmodel_pid_islandora_object_alter(AbstractObject $object, array &$context) {

}

/**
 * Allows modules to alter the datastream or block/modify the given action.
 *
 * This alter hook will be called before any datastream is ingested, modified or
 * purged.
 *
 * Adding datastreams to NewFedoraObject's will not trigger this alter hook
 * immediately, instead it will be triggered for all datastreams at the time
 * of the NewFedoraObject's ingest.
 *
 * Purging datastreams from a AbstractObject will not trigger this alter hook
 * at all.
 *
 * Changing datastream's properties such as "label", or "state", are considered
 * modifications, as well as changing the datastreams content.
 *
 * @param AbstractObject $object
 *   The object to the datastream belong to.
 * @param AbstractDatastream $datastream
 *   The datastream to alter.
 * @param array $context
 *   An associative array containing:
 *   - action: A string either 'ingest', 'purge', 'modify'.
 *   - block: Either TRUE or FALSE, if TRUE the action won't take place.
 *     Defaults to FALSE.
 *   - purge: Either TRUE or FALSE, only present when the action is 'purge'.
 *     If 'delete' or 'block' is set to TRUE, they will take precedence.
 *     Defaults to TRUE.
 *   - delete: Either TRUE or FALSE, only present when the action is 'purge'.
 *     If TRUE it will cause the object's state to be set to 'D' instead.
 *     If 'block' is set to TRUE, it will take precedence.
 *     Defaults to FALSE,
 *   - params: An associative array, only present when the action is 'modify'.
 *     The key value pairs represent what values will be changed. The params
 *     will match the same params as passed to FedoraApiM::modifyDatastream().
 *
 * @see FedoraApiM::modifyDatastream()
 */
function hook_islandora_datastream_alter(AbstractObject $object, AbstractDatastream $datastream, array &$context) {

}

/**
 * Allows modules to alter the datastream or block/modify the given action.
 *
 * @see hook_islandora_datastream_alter()
 */
function hook_cmodel_pid_dsid_islandora_datastream_alter(AbstractObject $object, AbstractDatastream $datastream, array &$context) {

}

/**
 * Notify modules that the given object was ingested.
 *
 * This hook is called after an object has been successfully ingested via a
 * FedoraRepository object.
 *
 * @note
 * If ingested directly via the FedoraApiM object this will not be called as we
 * don't have access to the ingested object at that time.
 *
 * @param AbstractObject $object
 *   The object that was ingested.
 */
function hook_islandora_object_ingested(AbstractObject $object) {

}

/**
 * Notify modules that the given object was ingested.
 *
 * @see hook_islandora_object_ingested()
 */
function hook_cmodel_pid_islandora_object_ingested(AbstractObject $object) {

}

/**
 * Notify modules that the given object was modified.
 *
 * This hook is called after an object has been successfully modified.
 *
 * Changing object properties such as "label", or "state", are considered
 * modifications, where as manipulating an object's datstreams are not.
 *
 * @param AbstractObject $object
 *   The object that was modified.
 *
 * @todo We should also include what changes were made in a additional
 *   parameter.
 */
function hook_islandora_object_modified(AbstractObject $object) {

}

/**
 * Notify modules that the given object was modified.
 *
 * @see hook_islandora_object_modified()
 */
function hook_cmodel_pid_islandora_object_modified(AbstractObject $object) {

}

/**
 * Notify modules that the given object was purged/deleted.
 *
 * This hook is called after an object has been successfully purged, or
 * when its state has been changed to "Deleted".
 *
 * @param string $pid
 *   The ID of the object that was purged/deleted.
 */
function hook_islandora_object_purged($pid) {

}

/**
 * Notify modules that the given object was purged/deleted.
 *
 * @see hook_islandora_object_purged()
 */
function hook_cmodel_pid_islandora_object_purged($pid) {

}

/**
 * Notify modules that the given datastream was ingested.
 *
 * This hook is called after the datastream has been successfully ingested.
 *
 * @note
 * If ingested directly via the FedoraApiM object this will not be called as we
 * don't have access to the ingested datastream at that time.
 *
 * @param AbstractObject $object
 *   The object the datastream belongs to.
 * @param AbstractDatastream $datastream
 *   The ingested datastream.
 */
function hook_islandora_datastream_ingested(AbstractObject $object, AbstractDatastream $datastream) {

}

/**
 * Notify modules that the given datastream was ingested.
 *
 * @see hook_islandora_object_ingested()
 */
function hook_cmodel_pid_dsid_islandora_datastream_ingested(AbstractObject $object, AbstractDatastream $datastream) {

}

/**
 * Notify modules that the given datastream was modified.
 *
 * This hook is called after an datastream has been successfully modified.
 *
 * Changing datastream properties such as "label", or "state", are considered
 * modifications, as well as the datastreams content.
 *
 * @param AbstractObject $object
 *   The object the datastream belongs to.
 * @param AbstractDatastream $datastream
 *   The datastream that was modified.
 * @param array $params
 *   The parameters from FedoraDatastream::modifyDatastream() used to modify the
 *   datastream.
 */
function hook_islandora_datastream_modified(AbstractObject $object, AbstractDatastream $datastream, array $params) {
  // Sample of sanitizing a label.
  $datastream->label = trim($datastream->label);
  // Sample of using modifyDatastream parameters.
  if (isset($params['mimetype'])) {
    $datastream->label .= " ({$params['mimetype']})";
  }
}

/**
 * Notify modules that the given datastream was modified.
 *
 * @see hook_islandora_datastream_modified()
 */
function hook_cmodel_pid_dsid_islandora_datastream_modified(AbstractObject $object, AbstractDatastream $datastream, array $params) {

}

/**
 * Notify modules that the given datastream was purged/deleted.
 *
 * This hook is called after an datastream has been successfully purged, or
 * when its state has been changed to "Deleted".
 *
 * @param AbstractObject $object
 *   The object the datastream belonged to.
 * @param string $dsid
 *   The ID of the datastream that was purged/deleted.
 */
function hook_islandora_datastream_purged(AbstractObject $object, $dsid) {

}

/**
 * Notify modules that the given datastream was purged/deleted.
 *
 * @see hook_islandora_datastream_purged()
 */
function hook_cmodel_pid_dsid_islandora_datastream_purged(AbstractObject $object, $dsid) {

}

/**
 * Register a datastream edit route/form.
 *
 * @param AbstractObject $object
 *   The object to check.
 * @param string $dsid
 *   A string indicating the datastream for which to get the registry.
 *
 * @return array
 *   An array of associative arrays, each mapping:
 *   - name: A string containg a human-readable name for the entry.
 *   - url: A string containing the URL to which to the user will be routed.
 */
function hook_islandora_edit_datastream_registry($object, $dsid) {
  $routes = array();
  $routes[] = array(
    'name' => t('My Awesome Edit Route'),
    'url' => "go/edit/here/{$object->id}/{$dsid}",
  );
  return $routes;
}

/**
 * Registry hook for required objects.
 *
 * Solution packs can include data to create certain objects that describe or
 * help the objects it would create. This includes collection objects and
 * content models.
 *
 * @see islandora_solution_packs_admin()
 * @see islandora_install_solution_pack()
 * @example islandora_islandora_required_objects()
 */
function hook_islandora_required_objects() {

}

/**
 * Registry hook for viewers that can be implemented by solution packs.
 *
 * Solution packs can use viewers for their data. This hook lets Islandora know
 * which viewers there are available.
 *
 * @see islandora_get_viewers()
 * @see islandora_get_viewer_callback()
 */
function hook_islandora_viewer_info() {

}

/**
 * Returns a list of datastreams that are determined to be undeletable.
 *
 * The list is used to prevent delete links from being shown.
 *
 * @param array $models
 *   An array of content models for the current object.
 *
 * @return array
 *   An array of DSIDs that shouldn't be deleted.
 */
function hook_islandora_undeletable_datastreams(array $models) {
  return array('DC', 'MODS');
}

/**
 * Define steps used in the islandora_ingest_form() ingest process.
 *
 * @param array $form_state
 *   An array containing the form_state, on which infomation from step storage
 *   might be extracted.  Note that the
 *
 * @return array
 *   An associative array of associative arrays which define each step in the
 *   ingest process.  Each step should consist of a unique name mapped to an
 *   array of properties (keys) which take different paramaters based upon type:
 *   - type: Type of step.  Only "form" and "callback" are implemented so far.
 *   Required "form" type specific parameters:
 *   - form_id: The form building function to call to get the form structure
 *     for this step.
 *   - args: An array of arguments to pass to the form building function.
 *   "Callback" type specific parameters:
 *   - do_function: A required associative array including:
 *       - 'function': The callback function to be called.
 *       - 'args': An array of arguments to pass to the callback function.
 *       - 'file': A file to include (relative to the module's path, including
 *          the file's extension).
 *   - undo_function: An optional associative array including:
 *       - 'function': The callback function to be called to reverse the
 *          executed action in the ingest steps.
 *       - 'args': An array of arguments to pass to the callback function.
 *       - 'file': A file to include (relative to the module's path, including
 *          the file's extension).
 *   Shared parameters between both types:
 *   - weight: The "weight" of this step--heavier(/"larger") values sink to the
 *     end of the process while smaller(/"lighter") values are executed first.
 *   Both types may optionally include:
 *   - module: A module from which we want to load an include.
 *   "Form" type may optionally include:
 *   - 'file': A file to include (relative to the module's path, including the
 *     file's extension).
 */
function hook_islandora_ingest_steps(array $form_state) {
  return array(
    'my_cool_step_definition' => array(
      'type' => 'form',
      'weight' => 1,
      'form_id' => 'my_cool_form',
      'args' => array('arg_one', 'numero deux'),
    ),
    'my_cool_step_callback' => array(
      'type' => 'callback',
      'weight' => 2,
      'do_function' => array(
        'function' => 'my_cool_execute_function',
        'args' => array('arg_one', 'numero deux'),
      ),
      'undo_function' => array(
        'function' => 'my_cool_undo_function',
        'args' => array('arg_one', 'numero deux'),
      ),
    ),
  );
}

/**
 * Alter the generated ingest steps.
 *
 * @param array $steps
 *   An array of steps as generated by hook_islandora_ingest_steps().
 *
 * @param array $form_state
 *   An array containing the Drupal form_state.
 */
function hook_islandora_ingest_steps_alter(array &$steps, array &$form_state) {

}

/**
 * Content model specific version of hook_islandora_ingest_steps().
 *
 * XXX: Content models are not selected in a generic manner. Currently, this
 *   gets called for every content model in the "configuration", yet the
 *   configuration never changes.  We should determine a consistent way to bind
 *   content models, so as to consistently be able to build steps.
 *
 * @see hook_islandora_ingest_steps()
 */
function hook_cmodel_pid_islandora_ingest_steps(array $form_state) {

}

/**
 * Alter the generated ingest steps for the given content model.
 *
 * @param array $steps
 *   An array of steps as generated by hook_islandora_ingest_steps().
 *
 * @param array $form_state
 *   An array containing the Drupal form_state.
 */
function hook_cmodel_pid_islandora_ingest_steps_alter(array &$steps, array &$form_state) {

}

/**
 * Object-level access callback hook.
 *
 * @param string $op
 *   A string define an operation to check. Should be defined via
 *   hook_permission().
 * @param AbstractObject $object
 *   An object to check the operation on.
 * @param object $user
 *   A loaded user object, as the global $user variable might contain.
 *
 * @return bool|NULL|array
 *   Either boolean TRUE or FALSE to explicitly allow or deny the operation on
 *   the given object, or NULL to indicate that we are making no assertion
 *   about the outcome. Can also be an array containing multiple
 *   TRUE/FALSE/NULLs, due to how hooks work.
 */
function hook_islandora_object_access($op, $object, $user) {
  switch ($op) {
    case 'create stuff':
      return TRUE;

    case 'break stuff':
      return FALSE;

    case 'do a barrel roll!':
      return NULL;
  }
}

/**
 * Content model specific version of hook_islandora_object_access().
 *
 * @see hook_islandora_object_access()
 */
function hook_cmodel_pid_islandora_object_access($op, $object, $user) {

}

/**
 * Datastream-level access callback hook.
 *
 * @param string $op
 *   A string define an operation to check. Should be defined via
 *   hook_permission().
 * @param AbstractDatastream $object
 *   An object to check the operation on.
 * @param object $user
 *   A loaded user object, as the global $user variable might contain.
 *
 * @return bool|NULL|array
 *   Either boolean TRUE or FALSE to explicitly allow or deny the operation on
 *   the given object, or NULL to indicate that we are making no assertion
 *   about the outcome. Can also be an array containing multiple
 *   TRUE/FALSE/NULLs, due to how hooks work.
 */
function hook_islandora_datastream_access($op, $object, $user) {
  switch ($op) {
    case 'create stuff':
      return TRUE;

    case 'break stuff':
      return FALSE;

    case 'do a barrel roll!':
      return NULL;
  }
}

/**
 * Content model specific version of hook_islandora_datastream_access().
 *
 * @see hook_islandora_datastream_access()
 */
function hook_cmodel_pid_islandora_datastream_access($op, $object, $user) {

}

/**
 * Lets one add to the overview tab in object management.
 */
function hook_islandora_overview_object(AbstractObject $object) {
  return drupal_render(drupal_get_form('some_form', $object));
}

/**
 * Lets one add to the overview tab in object management.
 *
 * Content model specific.
 */
function hook_cmodel_pid_islandora_overview_object(AbstractObject $object) {
  return drupal_render(drupal_get_form('some_form', $object));
}

/**
 * Lets one alter the overview tab in object management.
 */
function hook_islandora_overview_object_alter(AbstractObject &$object, &$output) {
  $output = $output . drupal_render(drupal_get_form('some_form', $object));
}

/**
 * Lets one alter the overview tab in object management.
 *
 * Content model specific.
 */
function hook_cmodel_pid_islandora_overview_object_alter(AbstractObject &$object, &$output) {
  $output = $output . drupal_render(drupal_get_form('some_form', $object));
}

/**
 * Defines derivative functions to be executed based on certain conditions.
 *
 * This hook fires when an object/datastream is ingested or a datastream is
 * modified. It may also be called to discover the datastream derivative
 * hierarchy.
 *
 * @param AbstractObject $object
 *   Optional object to which derivatives will be added
 * @param array $ds_modified_params
 *   An array that will contain the properties changed on the datastream if
 *   derivatives were triggered from a datastream_modified hook, as well as a
 *   'dsid' key naming the datastream that was modified. Can be populated
 *   manually, but likely empty otherwise.
 *
 * @return array
 *   An array containing an entry for each derivative to be created. Each entry
 *   is an array of parameters containing:
 *   - force: Bool denoting whether we are forcing the generation of
 *     derivatives.
 *   - source_dsid: (Optional) String of the datastream id we are generating
 *     from or NULL if it's the object itself. Does not impact function
 *     ordering.
 *   - destination_dsid: (Optional) String of the datastream id that is being
 *     created. To be used in the UI. Does not impact function ordering.
 *   - weight: A string denoting the weight of the function. This value is
 *     sorted upon to run functions in order.
 *   - function: An array of function(s) to be ran when constructing
 *     derivatives. Functions that are defined to be called for derivation
 *     creation must have the following structure:
 *     module_name_derivative_creation_function($object, $force = FALSE, $hook)
 *     These functions must return an array in the structure of:
 *     - success: Bool denoting whether the operation was successful.
 *     - messages: An array structure containing zero or more array's with the
 *       following fields:
 *       - message: A string passed through t() describing the
 *         outcome of the operation.
 *       - message_sub: (Optional) A substitution array as acceptable by t() or
 *         watchdog.
 *       - type: A string denoting whether the output is to be
 *         drupal_set_messaged (dsm) or watchdogged (watchdog).
 *       - severity: (Optional) A severity level / status to be used when
 *         logging messages. Uses the defaults of drupal_set_message and
 *         watchdog if not defined.
 *   - file: A string denoting the path to the file where the function
 *     is being called from.
 */
function hook_islandora_derivative(AbstractObject $object = NULL, $ds_modified_params = array()) {
  $derivatives[] = array(
    'source_dsid' => 'OBJ',
    'destination_dsid' => 'DERIV',
    'weight' => '0',
    'function' => array(
      'islandora_derivatives_test_create_deriv_datastream',
    ),
  );
  // Test object before adding this derivative.
  if ($object['SOMEWEIRDDATASTREAM']->mimetype == "SOMETHING/ODD") {
    $derivatives[] = array(
      'source_dsid' => 'SOMEWEIRDDATASTREAM',
      'destination_dsid' => 'STANLEY',
      'weight' => '-1',
      'function' => array(
        'islandora_derivatives_test_create_some_weird_datastream',
      ),
    );
  }
  $derivatives[] = array(
    'source_dsid' => NULL,
    'destination_dsid' => 'NOSOURCE',
    'weight' => '-3',
    'function' => array(
      'islandora_derivatives_test_create_nosource_datastream',
    ),
  );
  return $derivatives;
}

/**
 * Content model specific version of hook_islandora_derivative().
 *
 * @see hook_islandora_derivative()
 */
function hook_cmodel_pid_islandora_derivative() {

}

/**
 * Allows for the altering of defined derivative functions.
 */
function hook_islandora_derivative_alter(&$derivatives, AbstractObject $object = NULL, $ds_modified_params = array()) {
  foreach ($derivatives as $key => $derivative) {
    if ($derivative['destination_dsid'] == 'TN') {
      unset($derivatives[$key]);
    }
  }
  // Example of altering out derivative generation if only a specified set of
  // datastream parameters have been modified.
  $mask = array(
    'label' => NULL,
    'dateLastModified' => NULL,
    'dsid' => NULL,
  );
  $param_diff = array_diff_key($ds_modified_params, $mask);
  if (empty($param_diff)) {
    $derivatives = array();
  }
}

/**
 * Content model specific version of hook_islandora_derivative_alter().
 *
 * @see hook_islandora_derivative_alter()
 */
function hook_cmodel_pid_islandora_derivative_alter() {
}

/**
 * Retrieves PIDS of related objects for property updating.
 *
 * @param AbstractObject $object
 *   AbstractObject representing deleted object
 */
function hook_islandora_update_related_objects_properties(AbstractObject $object) {
  $related_objects = get_all_children_siblings_and_friends($object);
  $pids_to_return = array();
  foreach ($related_objects as $related_object) {
    $pids_to_return[] = $related_object->id;
  }
  return $pids_to_return;
}

/**
 * Alters breadcrumbs used on Solr search results and within Islandora views.
 *
 * @param array $breadcrumbs
 *   Breadcrumbs array to be altered by reference. Each element is markup.
 * @param string $context
 *   Where the alter is originating from for distinguishing.
 * @param AbstractObject $object
 *   (Optional) AbstractObject representing object providing breadcrumb path
 */
function hook_islandora_breadcrumbs_alter(&$breadcrumbs, $context, $object = NULL) {

}

/**
 * Registry hook for metadata display viewers.
 *
 * Modules can use this hook to override the default Dublin Core display.
 * This hook lets Islandora know which viewers there are available.
 *
 * @return array
 *   An associative array where the values are the following:
 *   -label: Human readable display label for selection.
 *   -description: A description of what the metadata display viewer does.
 *   -metadata callback: A callable function that provides the markup to be
 *    passed  off to the template files. Returns markup or FALSE if the viewer
 *    wishes to default back to the Dublin Core display for the current object.
 *   -description callback: A callable function that provides the markup to be
 *    passed for the description. Returns markup or FALSE if the viewer
 *    wishes to default back to the Dublin Core display for the current object.
 *   -configuration (Optional): A path to the administration page for the
 *    metadata display.
 *
 * @see islandora_retrieve_metadata_markup()
 */
function hook_islandora_metadata_display_info() {
  return array(
    'hookable_displays_yay' => array(
      'label' => t('Hookable display yay!'),
      'description' => t('This is purely an example of how to implement this.'),
      'metadata callback' => 'hookable_displays_some_function_that_returns_metadata_markup',
      'description callback' => 'hookable_displays_some_function_that_returns_description_markup',
      'configuration' => 'admin/hookable_displays_yay/somepath',
    ),
  );
}

/**
 * Allows modifications to the object whose metadata is being rendered.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 */
function hook_islandora_metadata_object_alter(AbstractObject &$object) {
  $this_other_object = islandora_object_load('awild:indirectionappears');
  $object = $this_other_object;
}

/**
 * Allows modifications to the object whose metadata is being rendered.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 */
function hook_islandora_metadata_object_description_alter(AbstractObject &$object) {
  $this_other_object = islandora_object_load('awild:indirectionappears');
  $object = $this_other_object;
}

/**
 * Defines predicates to be searched for when constructing breadcrumbs.
 *
 * @return array
 *   An array containing strings of predicates to be ORed together to be
 *   matched on in SPARQL.
 */
function hook_islandora_get_breadcrumb_query_predicates() {
  return array(
    'somepredicate',
    'someotherpredicate',
  );
}

/**
 * Use alter hook to modify registry paths before the paths are rendered.
 *
 * @param array $edit_registry
 *   The array of registry paths.
 * @param array $context
 *   An associative array containing:
 *   - object: The object that owns the datastream being edited.
 *   - datastream: The datastream being edited.
 *   - original_edit_registry: The original edit_registry prior to any
 *     modifications.
 */
function hook_islandora_edit_datastream_registry_alter(&$edit_registry, $context) {
  // Example: Remove xml form builder edit registry.
  if (isset($edit_registry['xml_form_builder_edit_form_registry'])) {
    unset($edit_registry['xml_form_builder_edit_form_registry']);
  }
  // Add custom form to replace the removed form builder edit_form.
  $edit_registry['somemodule_custom_form'] = array(
    'name' => t('Somemodule Custom Form'),
    'url' => "islandora/custom_form/{$context['object']->id}/{$context['datastream']->id}",
  );
}

/**
 * Permit configuration of connection parameters.
 *
 * @param RepositoryConnection $instance
 *   The connection being constructed. See the relevant Tuque ancestor classes
 *   for the particulars.
 *
 * @see https://github.com/Islandora/tuque/blob/1.x/HttpConnection.php
 */
function hook_islandora_repository_connection_construction_alter(RepositoryConnection $instance) {
  $instance->userAgent = "Tuque/cURL";
}

/**
 * Allow a overridable backend for generating breadcrumbs.
 *
 * Stolen shamelessly from @adam-vessey.
 *
 * @return array
 *   Should return an associative array mapping unique (module-prefixed,
 *   preferably) keys to associative arrays containing:
 *   - title: A human-readable title for the backend.
 *   - callable: A PHP callable to call for this backend, implementing
 *     callback_islandora_basic_collection_query_backends().
 *   - file: An optional file to load before attempting to call the callable.
 */
function hook_islandora_breadcrumbs_backends() {
  return array(
    'awesome_backend' => array(
      'title' => t('Awesome Backend'),
      'callable' => 'callback_islandora_breadcrumbs_backends',
    ),
  );
}

/**
 * Generate an array of links for breadcrumbs leading to $object, root first.
 *
 * Stolen shamelessly from @adam-vessey.
 *
 * @param AbstractObject $object
 *   The object to generate breadcrumbs for.
 *
 * @return array
 *   Array of links from root to the parent of $object.
 */
function callback_islandora_breadcrumbs_backends(AbstractObject $object) {
  // Do something to get an array of breadcrumb links for $object, root first.
  return array($root_link, $collection_link, $object_link);
}

/**
 * Permit modules to alter the filename of a downloaded datastream.
 *
 * @param string $filename
 *   The filename being created.
 *
 * @param AbstractDatastream $datastream
 *   The datastream object being downloaded.
 */
function hook_islandora_datastream_filename_alter(&$filename, AbstractDatastream $datastream) {

  // Example taken from islandora_datastream_filenamer.
  $pattern = variable_get('islandora_ds_download_filename_pattern', FALSE);
  if ($pattern) {
    $filename = token_replace($pattern,
      array('datastream' => $datastream),
      array('clear' => TRUE)
    );
  }
}

/**
 * Allow solution packs to register relationships used for children.
 *
 * @param string|array $cmodels
 *   This takes either:
 *      - string: the string 'all'. Function returns all child relationships.
 *      - array: an array of cmodel PIDs to return the relationships for.
 *
 * @return array
 *   - prefix (array): This is is a valid snip-it of SPARQL to register
 *     prefixes used in the predicates array.
 *   - predicate (array): This array contains predicates used by the solution
 *     pack for child objects.
 */
function hook_islandora_solution_pack_child_relationships($cmodels) {
  if ($cmodels === 'all' || in_array('my:cmodel_pid', $cmodels)) {
    return array(
      'prefix' => array('PREFIX islandora: <http://islandora.ca/ontology/relsext#>'),
      'predicate' => array(
        '<fedora-rels-ext:isMemberOfCollection>',
        '<fedora-rels-ext:isMemberOf>',
        '<islandora:isPageOf>',
      ),
    );
  }
}
