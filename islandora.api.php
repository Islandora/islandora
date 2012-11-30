<?php

/**
 * @file
 * This file lists and documents all available hook functions to manipulate data.
 */

/**
 * Generate a repository objects view.
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject being operated on.
 * @param object $user
 *   The user accessing the object.
 * @param string $page_number
 *   The page in the content.
 * @param string $page_size: The size of the page.
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_islandora_view_object($fedora_object, $user, $page_number, $page_size) {}

/**
 * Generate an object's display for the given content model.
 *
 * Content models PIDs have colons and hyphens changed to underscores, to
 * create the hook name.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_CMODEL_PID_islandora_view_object($fedora_object) {}


/**
 * Alter display output after it has been generated.
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject being operated on.
 * @param array $arr
 *   An arr of rendered views.
 */
function hook_islandora_view_object_alter(&$fedora_object, &$arr) {}

/**
 * Generate an object's management display.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_islandora_edit_object($fedora_object) {}

/**
 * Generate an object's management display for the given content model.
 *
 * Content models PIDs have colons and hyphens changed to underscores, to
 * create the hook name.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 *
 * @return array
 *   An array whose values are markup.
 */
function hook_CMODEL_PID_islandora_edit_object($fedora_object) {}

/**
 * Allow management display output to be altered.
 *
 * @param type $fedora_object
 *   A Tuque FedoraObject
 * @param type $arr
 *   an arr of rendered views
 */
function hook_islandora_edit_object_alter(&$fedora_object, &$arr) {}

/**
 * Allows modules to add to an objects ingest process.
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject.
 */
function hook_islandora_ingest_post_ingest($fedora_object) {}

/**
 * Allow modules to add to the ingest process of a specific content model.
 */
function hook_CMODEL_PID_islandora_ingest_post_ingest($fedora_object) {}

/**
 * Allows modules to add to a repository objects view/edit(/misc) process.
 *
 * If you implement this hook you must also register your module with
 * hook_islandora_hook_info().
 *
 * @param FedoraObject $fedora_object
 *   A Tuque FedoraObject.
 *
 * @return array|null
 *   An associative array with 'deleted' mapped to TRUE--indicating that the
 *   object should just be marked as deleted, instead of actually being
 *   purged--or NULL/no return if we just need to do something before the
 *   is purged.
 */
function hook_islandora_pre_purge_object($fedora_object) {}

/**
 * Allow modules to react to the purge process of a specific content model.
 *
 * @see hook_islandora_pre_purge_object()
 */
function hook_CMODEL_PID_islandora_pre_purge_object($fedora_object) {}

/**
 * Register potential ingest routes.
 *
 * Implementations should return an array containing possible routes.
 */
function hook_islandora_ingest_registry($collection_object) {
  $reg = array(
    array(
      'name' => t('Ingest route name'),
      'url' => 'ingest_route/url',
      'weight' => 0,
    ),
  );
  return $reg;
}

/**
 * Register a datastream edit route/form.
 *
 * @param $islandora_object
 * @param $ds_id
 */
function hook_islandora_edit_datastream_registry($islandora_object, $ds_id) {}

/**
 * Alter an object before it gets used further down the stack.
 *
 * @param type $object
 *   A Tuque FedoraObject
 */
function hook_islandora_object_alter($fedora_object) {}

/**
 * Allow modification of an object before ingesting.
 *
 * @param type $islandora_object
 *   A Tuque FedoraObject
 */
function hook_islandora_ingest_pre_ingest($islandora_object) {}

/**
 * Allow modification of objects of a certain content model before ingesting.
 *
 * @see hook_islandora_ingest_pre_ingest()
 */
function hook_CMODEL_PID_islandora_ingest_pre_ingest($islandora_object) {}

/**
 * Allow modules to setup for the purge of a datastream.
 *
 * @param object $datastream
 *   A Tuque FedoraDatastream object.
 */
function hook_islandora_pre_purge_datastream($datastream) {}

/**
 * Allow modules to react after a datastream is purged.
 *
 * @param object $object
 *   A Tuque FedoraObject.
 * @param string $dsid
 *   A id of the former datastream.
 */
function hook_islandora_post_purge_datastream($object, $dsid) {}

/**
 * Allow modules to react post-purge.
 *
 * @param string $object_id
 *   The former object's PID.
 * @param array $content_models
 *   An array containing the models to which the former object.
 */
function hook_islandora_post_purge_object($object_id, $content_models) {}

/**
 * Registry hook for required objects.
 *
 * Solution packs can include data to create certain objects that describe or
 * help the objects it would create. This includes collection objects and content
 * models.
 *
 * @see islandora_solution_packs_admin()
 * @see islandora_install_solution_pack()
 * @example islandora_islandora_required_objects()
 */
function hook_islandora_required_objects() {}

/**
 * Registry hook for viewers that can be implemented by solution packs.
 *
 * Solution packs can use viewers for their data. This hook lets Islandora know
 * which viewers there are available.
 *
 * @see islandora_get_viewers()
 * @see islandora_get_viewer_callback()
 */
function hook_islandora_viewer_info() {}


/**
 * Returns a list of datastreams that are determined to be undeletable.
 */
function hook_islandora_undeletable_datastreams(array $models) {}

/**
 * Define steps used in the islandora_ingest_form() ingest process.
 *
 * @return array
 *   An array of associative arrays which define each step in the ingest
 *   process.  Steps are defined by by a number of properties (keys) including:
 *   - type: The type of step.  Currently, only "form" is implemented.
 *   - weight: The "weight" of this step--heavier(/"larger") values sink to the
 *     end of the process while smaller(/"lighter") values are executed first.
 *   - form_id: The form building function to call to get the form structure
 *     for this step.
 *   - args: An array of arguments to pass to the form building function.
 *   And may optionally include both:
 *   - module: A module from which we want to load an include.
 *   - file: A file to include (relative to the module's path, including the
 *     file's extension).
 */
function hook_islandora_ingest_steps(array $configuration) {
  return array(
    array(
      'type' => 'form',
      'weight' => 1,
      'form_id' => 'my_cool_form',
      'args' => array('arg_one', 'numero deux'),
    ),
  );
}
/**
 * Content model specific version of hook_islandora_ingest_steps().
 *
 * @see hook_islandora_ingest_steps()
 */
function hook_CMODEL_PID_islandora_ingest_steps(array $configuration) {}
