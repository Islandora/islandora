<?php

/**
 * @file
 * Does rule type stuff,
 */

/**
 * Helper function to get reused "parameter" array.
 */
function islandora_rules_relationship_parameter_array() {
  $to_return = array(
    'subject' => array(
      'type' => 'islandora_object',
      'label' => t('Subject'),
      'description' => t('An object of which we should check the relationships (The "subject" of the relationship).'),
    ),
    'pred_uri' => array(
      'type' => 'text',
      'label' => t('Predicate URI'),
      'description' => t('The URI namespace to which the predicate belongs.'),
    ),
    'pred' => array(
      'type' => 'text',
      'label' => t('Predicate'),
      'description' => t('The predicate of the relationship.'),
    ),
    'object' => array(
      'type' => 'text',
      'label' => t('Object'),
      'description' => t('The object of the relationship.'),
      'allow null' => TRUE,
      'default value' => NULL,
    ),
    'type' => array(
      'type' => 'integer',
      'label' => t('Object type in the relationship'),
      'description' => t('0=URI, 1=plain literal'),
      'default value' => 0,
    ),
  );

  return $to_return;
}

/**
 * Implements hook_rules_condition_info().
 */
function islandora_rules_condition_info() {
  $cond = array();

  $cond['islandora_object_has_relationship'] = array(
    'label' => t('Check object for a relationship'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_relationship_parameter_array(),
  );

  return $cond;
}

/**
 * Implements hook_rules_action_info().
 */
function islandora_rules_action_info() {
  $cond = array();

  $cond['islandora_object_remove_relationship'] = array(
    'label' => t('Remove a relationship from an object'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_relationship_parameter_array(),
  );

  $cond['islandora_object_add_relationship'] = array(
    'label' => t('Add a relationship to an object'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_relationship_parameter_array(),
  );
  return $cond;
}
/**
 * Checks that there is a relationship match on the given object.
 *
 * Takes a subject (either a AbstractObject or a FedoraDatastream), as well as
 * the parameters for FedoraRelsExt::get() or FedoraRelsInt::get(), to try to
 * find a match.
 *
 * @see FedoraRelsExt::get()
 */
function islandora_object_has_relationship($sub, $pred_uri, $pred, $object, $type) {
  $relationships = $sub->relationships->get($pred_uri, $pred, $object, $type);
  return !empty($relationships);
}

/**
 * Remove a relationship from the given object.
 *
 * Takes a subject (either a AbstractObject or a FedoraDatastream), as well as
 * the parameters for FedoraRelsExt::remove() or FedoraRelsInt::remove(), to
 * try to find a match.
 *
 * @see FedoraRelsExt::get()
 */
function islandora_object_remove_relationship($sub, $pred_uri, $pred, $object, $type) {
  $sub->relationships->remove($pred_uri, $pred, $object, $type);
}

/**
 * Add a relationship to the given object.
 *
 * Takes a subject (either a AbstractObject or a FedoraDatastream), as well as
 * the parameters for FedoraRelsExt::add() or FedoraRelsInt::add(), and adds
 * the represented relationship.
 *
 * @see FedoraRelsExt::get()
 */
function islandora_object_add_relationship($sub, $pred_uri, $pred, $object, $type) {
  $sub->relationships->add($pred_uri, $pred, $object, $type);
}
