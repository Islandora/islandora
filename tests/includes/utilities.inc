<?php

/**
 * Utilities classes for simpletest.
 *
 * These utilities are shared between the Islandora Web and Unit test cases and
 * can potentially be used for future testing implementations, as they pass back
 * a consistent style of result.
 *
 * Check the implementations in IslandoraUnitTestCase and IslandoraWebTestCase
 * for some examples of how this is done in a Drupal context; in the future,
 * more methods for result-passing could potentially be added to this without
 * breaking existing implementations.
 */
class IslandoraTestUtilities extends IslandoraTestUtilityClass {

  protected $configuration;

  protected $params;

  public $results = array();

  protected $repository;

  /**
   * Constructs an IslandoraTestUtilities object.
   *
   * @param array $configuration
   *   The parsed test configuration.
   * @param array $params
   *   Any additional parameters the method called may need to function.
   */
  public function __construct(array $configuration, array $params = array()) {
    $this->configuration = $configuration;
    $this->params = $params;
    $connection = new RepositoryConnection($this->configuration['fedora_url'], $this->configuration['admin_user'], $this->configuration['admin_pass']);
    $api = new FedoraApi($connection);
    $this->repository = new FedoraRepository($api, new SimpleCache());
  }

  /**
   * Sets up a drupal filter that can read from the tests users table.
   */
  public function setUpDrupalFilter() {
    $original_drupal_filter_content = file_get_contents($this->configuration['drupal_filter_file']);
    $connection_info = Database::getConnectionInfo('default');
    $drupal_filter_dom = new DomDocument();
    $drupal_filter_dom->loadXML($original_drupal_filter_content);
    $prefix = $connection_info['default']['prefix']['default'];
    $filter_drupal_connection_node = $drupal_filter_dom->getElementsByTagName('FilterDrupal_Connection')->item(0);
    $first_connection_node = $drupal_filter_dom->getElementsByTagName('connection')->item(0);
    $connection_node = $filter_drupal_connection_node->insertBefore($drupal_filter_dom->createElement('connection'), $first_connection_node);
    $connection_node->setAttribute('server', $connection_info['default']['host']);
    $connection_node->setAttribute('dbname', $connection_info['default']['database']);
    $connection_node->setAttribute('user', $connection_info['default']['username']);
    $connection_node->setAttribute('password', $connection_info['default']['password']);
    $connection_node->setAttribute('port', $connection_info['default']['port'] ? $connection_info['default']['port'] : '3306');
    $sql_node = $drupal_filter_dom->createElement('sql', "SELECT DISTINCT u.uid AS userid, u.name AS Name, u.pass AS Pass, r.name AS Role FROM ({$prefix}users u LEFT JOIN {$prefix}users_roles ON u.uid={$prefix}users_roles.uid) LEFT JOIN {$prefix}role r ON r.rid={$prefix}users_roles.rid WHERE u.name=? AND u.pass=?;");
    $connection_node->appendChild($sql_node);
    file_put_contents($this->configuration['drupal_filter_file'], $drupal_filter_dom->saveXML());
  }

  /**
   * Returns an array of IslandoraTestUtilityResults.
   *
   * @return IslandoraTestUtilityResult[]
   *   The results.
   */
  public function getResults() {
    return $this->results;
  }

  /**
   * Adds a result to $this->results.
   *
   * @param bool $type
   *   The type of result (TRUE for pass, FALSE for fail).
   * @param string $message
   *   The message to put in the result.
   */
  public function addResult($type, $message) {
    $result = new IslandoraTestUtilityResult($type, $message, $this->getAssertionCall());
    $this->results[] = $result;
  }

  /**
   * Cycles through backtrace until the first non-assertion method is found.
   *
   * This is a manipulated version of DrupalWebTestCase::getAssertionCall().
   * We use it here so that we can pass back assertion calls from
   * DatastreamValidator assertions instead of less useful TestCase functions.
   *
   * @return array
   *   Array representing the true caller.
   */
  public function getAssertionCall() {
    $backtrace = debug_backtrace();
    array_shift($backtrace);
    return _drupal_get_last_caller($backtrace);
  }

  /**
   * Asserts that the given datastreams exist correctly on the object.
   *
   * @param AbstractObject $object
   *   The object to check.
   * @param array $datastreams
   *   An array of strings containing datastream names.
   *
   * @return bool
   *   TRUE on success, FALSE on fail.
   */
  public function assertDatastreams(AbstractObject $object, array $datastreams) {
    if (!self::assertFedoraObject($object)) {
      $this->addResult(FALSE, "Failed. Object passed in is invalid.", 'Islandora');
    }
    else {
      $missing_datastreams = array_diff_key(array_flip($datastreams), $object->repository->api->a->listDatastreams($object->id));

      if (!empty($missing_datastreams)) {
        $this->addResult(FALSE, "Failed to find datastream(s) " . implode(', ', array_flip($missing_datastreams)) . " in object {$object->id}.");
        return FALSE;
      }

      $this->addResult(TRUE, "Found required datastream(s) in object {$object->id}");
      return TRUE;
    }
  }

  /**
   * Asserts that the given datastreams do not exist on the object.
   *
   * @param AbstractObject $object
   *   The object to check.
   * @param array $datastreams
   *   An array of datastreams to confirm not present.
   */
  public function assertNoDatastreams(AbstractObject $object, array $datastreams) {
    if (!self::assertFedoraObject($object)) {
      $this->addResult(FALSE, "Failed. Object passed in is invalid.", 'Islandora');
      return;
    }
    $found_datastreams = array_intersect_key(array_flip($datastreams), $object->repository->api->a->listDatastreams($object->id));

    if (!empty($found_datastreams)) {
      $this->addResult(FALSE, "Found unwanted datastream(s)" . implode(', ', array_flip($found_datastreams)) . " in object {$object->id}.");
      return FALSE;
    }

    $this->addResult(TRUE, "Unwanted datastream(s) not found in object {$object->id}");
    return TRUE;
  }

  /**
   * Attempts to validate an array of datastreams, generally via binary checks.
   *
   * Datastream validation classes exist in, and can be added to, the file
   * 'datastream_validators.inc', which is found in this folder. Datastream
   * validator classes use the naming convention 'PrefixDatastreamValidator',
   * and that 'Prefix' is what this function uses to determine what class to
   * instantiate.
   *
   * $param IslandoraFedoraObject $object
   *   The object to load datastreams from.
   * $param array $datastreams
   *   An array of arrays that pair DSIDs, DatastreamValidator class prefixes,
   *   and optional params. You can check some of the existing implementations
   *   for examples.
   */
  public function validateDatastreams($object, array $datastreams) {

    if (!self::assertFedoraObject($object)) {
      $this->addResult(FALSE, "Datastream validation failed; Object passed in is invalid.", 'Islandora');
      return;
    }

    module_load_include('inc', 'islandora', 'tests/includes/datastream_validators');

    foreach ($datastreams as $datastream) {
      // XXX: The "+ array (2 => array())" is to allow the value to be optional.
      list($dsid, $prefix, $params) = $datastream + array(2 => array());

      // Instantiate the appropriate class, and grab the results.
      $class_name = "{$prefix}DatastreamValidator";
      if (class_exists($class_name)) {
        $validator = new $class_name($object, $dsid, $params);
        foreach ($validator->getResults() as $result) {
          $this->addResult($result->getType(), $result->getMessage());
        }
      }
      else {
        $this->addResult(FALSE, "No DatastreamValidator class was found with the name '$class_name'; are you sure the prefix given to IslandoraWebTestCase->validateDatastreams() was entered correctly, or that such a validator exists?", 'Islandora');
      }
    }
  }

  /**
   * Constructs and ingests a Fedora object and datastream(s) via tuque.
   *
   * All keys inside the parameter arrays for this function are optional. it
   * can be run simply by calling the method with no arguments.
   *
   * If your test case supports logged in Drupal users, IslandoraTestUtilities
   * can be instantiated with $params['logged_in_user'] as that user object, and
   * this method will set the owner of the ingested object as that user by
   * default.
   *
   * @param array $properties
   *   An array containing object information using these keys:
   *   'label' - The object label; randomized if not set.
   *   'pid' - 'namespace:pid', or just 'namespace' to generate the suffix.
   *   'models' - An array that can contain multiple content model PIDs, or a
   *   string containing a single content model PID.
   *   'owner' - The object's owner. Defaults to the currently logged-in user,
   *   if available. It is recommended to set this to a value that can be found
   *   in $this->users; otherwise, this object will have to be manually deleted.
   *   'parent' - The PID of the parent collection.
   * @param array $datastreams
   *   An array containing zero or more datastream arrays that use the keys:
   *   'dsid' - The datastream ID; randomized if not set.
   *   'path' - The path to the file to use; defaults to fixtures/test.jpg.
   *   'string' - A string to set the datastream from; overrides 'path'.
   *   'control_group' - The single-letter control group identifier.
   *   'mimetype' - The datastream's mimetype.
   *
   * @return bool|AbstractObject
   *   FALSE if the object ingest failed, or the object if successful.
   */
  public function ingestConstructedObject(array $properties = array(), array $datastreams = array()) {
    if (!isset($properties['pid'])) {
      $properties['pid'] = "islandora";
    }
    $object = $this->repository->constructObject($properties['pid']);

    // Set the object properties before ingesting it.
    if (isset($properties['label'])) {
      $object->label = $properties['label'];
    }
    else {
      $properties['label'] = DrupalUnitTestCase::randomName();
      $object->label = $properties['label'];
    }

    if (isset($properties['owner'])) {
      $object->owner = $properties['owner'];
    }
    elseif (isset($this->params['logged_in_user'])) {
      $object->owner = $this->params['logged_in_user']->name;
    }

    if (isset($properties['models'])) {
      try {
        $object->models = (array) $properties['models'];
      }
      catch (Exception $e) {
        $this->addResult(FALSE, "Encountered an exception when trying to add content models to {$object->id}: $e");
        return FALSE;
      }
    }

    // Chuck in some datastreams.
    if (!empty($datastreams)) {
      foreach ($datastreams as $datastream) {
        if (!isset($datastream['dsid'])) {
          $datastream['dsid'] = DrupalUnitTestCase::randomName();
        }
        if (!isset($datastream['control_group'])) {
          $new_datastream = $object->constructDatastream($datastream['dsid']);
        }
        else {
          $new_datastream = $object->constructDatastream($datastream['dsid'], $datastream['control_group']);
        }
        $new_datastream->label = $datastream['dsid'];
        if (isset($datastream['mimetype'])) {
          $new_datastream->mimetype = $datastream['mimetype'];
        }
        if (isset($datastream['string'])) {
          $new_datastream->setContentFromString($datastream['string']);
        }
        else {
          $path = isset($datastream['path']) ? $datastream['path'] : drupal_get_path('module', 'islandora') . '/tests/fixtures/test.jpg';
          $new_datastream->setContentFromFile($path);
        }
        $object->ingestDatastream($new_datastream);
      }
    }

    $this->repository->ingestObject($object);
    if (!$object) {
      $this->addResult(FALSE, t("Failed to ingest object."), 'Islandora');
      return FALSE;
    }
    else {
      $this->addResult(TRUE, t("Ingested object %object", array('%object' => $object->id)), 'Islandora');
    }

    // Add a parent relationship, if necessary.
    if (isset($properties['parent'])) {
      $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $properties['parent']);
    }

    return $object;
  }

  /**
   * Deletes all objects created by the given user.
   *
   * To safeguard against leaving test objects in the repository, this is called
   * each time DrupalTestCase::run() calls tearDown(). This feature can be
   * toggled by setting $this->deleteObjectsOnTeardown to TRUE or FALSE.
   *
   * @param object $username
   *   The user whose objects we'd like to remove.
   *
   * @return bool
   *   TRUE if all objects were removed, or FALSE if any of them still remained
   *   after removal.
   */
  public function deleteUserCreatedObjects($username) {
    if ($username === $this->configuration['admin_user']) {
      $this->addResult(FALSE, "This function will under no circumstance attempt deletion of all objects owned by the configured Fedora admin user ({$this->configuration['admin_user']}), as this could irreparably damage the repository.", 'Islandora');
      return FALSE;
    }

    $query = <<<QUERY
SELECT ?object FROM <#ri> WHERE
{
  ?object <fedora-model:ownerId> "$username"
}
QUERY;

    $objects = $this->repository->ri->sparqlQuery($query);
    $return_value = TRUE;
    foreach ($objects as $object) {
      $loaded_object = islandora_object_load($object['object']['value']);
      $this->repository->purgeObject($loaded_object->id);
      if (!islandora_object_load($object['object']['value'])) {
        $this->addResult(TRUE, "Object {$object['object']['value']} successfully removed from repository.");
      }
      else {
        $this->addResult(FALSE, "Unable to remove object {$object['object']['value']} from the repository.");
        $return_value = FALSE;
      }
    }
    return $return_value;
  }

  /**
   * Asserts that an object is a FedoraObject or an IslandoraFedoraObject.
   *
   * @param object $object
   *   The object to assess.
   *
   * @return bool
   *   TRUE if it is either of those object types, or FALSE otherwise.
   */
  public static function assertFedoraObject($object) {
    return ($object instanceof FedoraObject);
  }

}
