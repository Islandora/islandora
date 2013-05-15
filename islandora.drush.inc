<?php

/**
 * @file
 * Drush integration for the islandora module.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_drush_command() {
  return array(
    'islandora-correct-type-hinting' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
      'description' => dt('Corrects the type hinting for function parameters that reference non-abstract Tuque objects. Converts to a more generic form of AbstractObject. Be warned this will modify the source code in your site!'),
    ),
  );
}

/**
 * Drush command callback.
 *
 * Corrects the type hinting in parameters such that they only use AbstractObject.
 */
function drush_islandora_correct_type_hinting() {
  drush_print(dt('This will modify source code within your Drupal site. This can not be reversed.'));
  if (!drush_confirm(dt('Do you really want to continue?'))) {
    return drush_user_abort();
  }
  // Only change files within modules directories and only modify php files.
  $ret = -1;
  passthru('find . -regex ".*\.\(php\|inc\|test\|module\|install\)" -path "*/modules/*" | xargs perl -pi -e "s/(\(.*)(AbstractFedoraObject|FedoraObject|NewFedoraObject|IslandoraFedoraObject|IslandoraNewFedoraObject)( .*\))/\1AbstractObject\3/g"', $ret);
  passthru('find . -regex ".*\.\(php\|inc\|test\|module\|install\)" -path "*/modules/*" | xargs perl -pi -e "s/(\(.*)(AbstractFedoraDatastream|AbstractExistingFedoraDatastream|FedoraDatastream|NewFedoraDatastream|FedoraDatastreamVersion|IslandoraNewFedoraDatastream|IslandoraFedoraDatastream|IslandoraFedoraDatastreamVersion)( .*\))/\1AbstractDatastream\3/g"', $ret);
}

function drush_islandora_correct_type_hinting_validate() {
  if (drush_is_windows()) {
    drush_set_error(dt("Sorry but this hasn't been implemented for Windows, it only works on unix systems."));
  }
}
