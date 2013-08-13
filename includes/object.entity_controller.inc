<?php

/**
 * @file
 * Very basic entity controller.
 */

class IslandoraObjectEntityController implements DrupalEntityControllerInterface {

  /**
   * Constructor.
   *
   * @param string $entity_type
   *   The type of entity.
   */
  public function __construct($entity_type) {
    // No-op...
  }

  /**
   * Load all the entities.
   *
   * @param array $ids
   *   The ID's of the entities.
   * @param array $conditions
   *   The conditions to apply.
   *
   * @return array
   *   An array of loaded objects.
   */
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

  /**
   * Reset the cache.
   *
   * @param array $ids
   *   The ID's of the entities.
   */
  public function resetCache(array $ids = NULL) {
    // no-op.
  }
}
