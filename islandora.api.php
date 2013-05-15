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
function hook_CMODEL_PID_islandora_view_object($object) {
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
function hook_CMODEL_PID_islandora_view_object_alter(&$object, &$rendered) {
}

/**
 * Generate an object's management display.
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
 * Generate an object's management display for the given content model.
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
function hook_CMODEL_PID_islandora_edit_object($object) {
}

/**
 * Allow management display output to be altered.
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
 *     The key value pairs repersent what values will be changed. The params
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
function hook_CMODEL_PID_islandora_object_alter(AbstractObject $object, array &$context) {
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
 *     The key value pairs repersent what values will be changed. The params
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
function hook_CMODEL_PID_DSID_islandora_datastream_alter(AbstractObject $object, AbstractDatastream $datastream, array &$context) {
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
function hook_CMODEL_PID_islandora_object_ingested(AbstractObject $object) {
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
 *   The object that was ingested.
 *
 * @todo We should also include what changes were made in a additional
 *   parameter.
 */
function hook_islandora_object_modified(AbstractObject $object) {
}

/**
 * Notify modules that the given object was ingested.
 *
 * @see hook_islandora_object_modified()
 */
function hook_CMODEL_PID_islandora_object_modified(AbstractObject $object) {
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
function hook_CMODEL_PID_islandora_object_purged($pid) {
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
function hook_CMODEL_PID_DSID_islandora_datastream_ingested(AbstractObject $object, AbstractDatastream $datastream) {
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
 *   The datastream that was ingested.
 *
 * @todo We should also include what changes were made in a additional
 *   parameter.
 */
function hook_islandora_datastream_modified(AbstractObject $object, AbstractDatastream $datastream) {
}

/**
 * Notify modules that the given datastream was modified.
 *
 * @see hook_islandora_datastream_modified()
 */
function hook_CMODEL_PID_islandora_datastream_modified(AbstractObject $object, AbstractDatastream $datastream) {
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
function hook_CMODEL_PID_islandora_datastream_purged(AbstractObject $object, $dsid) {
}

/**
 * Register a datastream edit route/form.
 *
 * @param AbstractObject $object
 *   The object to check.
 * @param string $dsid
 *   todo
 */
function hook_islandora_edit_datastream_registry($object, $dsid) {
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
 */
function hook_islandora_undeletable_datastreams(array $models) {
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
 *   Required "callback" type specific parameters:
 *   - do_function: An associate array including:
 *       - 'function': The callback function to be called.
 *       - 'args': An array of arguments to pass to the callback function.
 *   - undo_function: An associate array including:
 *       - 'function': The callback function to be called to reverse the
 *          executed action in the ingest steps.
 *       - 'args': An array of arguments to pass to the callback function.
 *   Shared parameters between both types:
 *   - weight: The "weight" of this step--heavier(/"larger") values sink to the
 *     end of the process while smaller(/"lighter") values are executed first.
 *   Both types may optionally include:
 *   - module: A module from which we want to load an include.
 *   "Form" type may optionally include:
 *   - file: A file to include (relative to the module's path, including the
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
 * Content model specific version of hook_islandora_ingest_steps().
 *
 * XXX: Content models are not selected in a generic manner. Currently, this
 *   gets called for every content model in the "configuration", yet the
 *   configuration never changes.  We should determine a consistent way to bind
 *   content models, so as to consistently be able to build steps.
 *
 * @see hook_islandora_ingest_steps()
 */
function hook_CMODEL_PID_islandora_ingest_steps(array $form_state) {
}
