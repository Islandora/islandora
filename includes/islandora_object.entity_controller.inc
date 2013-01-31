<?php

/**
 * Very basic entity controller...
 */
class IslandoraObjectEntityController implements DrupalEntityControllerInterface {
  public function __construct($entityType) {
    // No-op...
  }

  public function load($ids = array(), $conditions = array()) {
    if (!empty($conditions)) {
      throw new Exception('Conditions not implemented.');
    }

    $loaded = array();
    foreach ($ids as $id) {
      $load = islandora_object_load($id);
      if ($load) {
        $loaded[] = $load;
      }
    }

    return $loaded;
  }

  public function resetCache(array $ids = NULL) {
    // no-op
  }
}

