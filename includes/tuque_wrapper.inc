<?php

/**
 * @file
 * Wrapper around the tuque library, allows for autoloading of Islandora Tuque
 * Objects.
 *
 * @todo Overload functions and apply pre/post hooks.
 */

// This function may not exist when a batch operation is running from a
// multistep form.
if (function_exists('drupal_get_path')) {
  $islandora_module_path = drupal_get_path('module', 'islandora');
}

// @todo this until we expose these in a module or library
@include_once 'sites/all/libraries/tuque/Datastream.php';
@include_once 'sites/all/libraries/tuque/FedoraApi.php';
@include_once 'sites/all/libraries/tuque/FedoraApiSerializer.php';
@include_once 'sites/all/libraries/tuque/Object.php';
@include_once 'sites/all/libraries/tuque/RepositoryConnection.php';
@include_once 'sites/all/libraries/tuque/Cache.php';
@include_once 'sites/all/libraries/tuque/RepositoryException.php';
@include_once 'sites/all/libraries/tuque/Repository.php';
@include_once 'sites/all/libraries/tuque/FedoraRelationships.php';

@include_once "$islandora_module_path/libraries/tuque/Datastream.php";
@include_once "$islandora_module_path/libraries/tuque/FedoraApi.php";
@include_once "$islandora_module_path/libraries/tuque/FedoraApiSerializer.php";
@include_once "$islandora_module_path/libraries/tuque/Object.php";
@include_once "$islandora_module_path/libraries/tuque/RepositoryConnection.php";
@include_once "$islandora_module_path/libraries/tuque/Cache.php";
@include_once "$islandora_module_path/libraries/tuque/RepositoryException.php";
@include_once "$islandora_module_path/libraries/tuque/Repository.php";
@include_once "$islandora_module_path/libraries/tuque/FedoraRelationships.php";

/**
 * Allow modules to alter an object before a mutable event occurs.
 */
function islandora_alter_object(AbstractObject $object, array &$context) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  drupal_alter(islandora_build_hook_list('islandora_object', $object->models), $object, $context);
}

/**
 * Allow modules to alter a datastream before a mutable event occurs.
 */
function islandora_alter_datastream(AbstractObject $object, AbstractDatastream $datastream, array &$context) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $types = array();
  foreach ($object->models as $model) {
    $types[] = "{$model}_{$datastream->id}";
  }
  drupal_alter(islandora_build_hook_list('islandora_datastream', $types), $object, $datastream, $context);
}

/**
 * Constructs a list of hooks from the given parameters and invokes them.
 */
function islandora_invoke_object_hooks($hook, array $models) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  return islandora_invoke_hook_list($hook, $models, array_slice(func_get_args(), 2));
}

/**
 * Constructs a list of hooks from the given parameters and invokes them.
 */
function islandora_invoke_datastream_hooks($hook, array $models, $dsid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $refinements = array();
  foreach ($models as $model) {
    $refinements[] = "{$model}_{$dsid}";
  }
  return islandora_invoke_hook_list($hook, $refinements, array_slice(func_get_args(), 3));
}

class IslandoraFedoraRepository extends FedoraRepository {
  protected $queryClass = 'IslandoraRepositoryQuery';
  protected $newObjectClass = 'IslandoraNewFedoraObject';
  protected $objectClass = 'IslandoraFedoraObject';

  /**
   * Ingest the given object.
   *
   * @see FedoraRepository::ingestObject()
   */
  public function ingestObject(NewFedoraObject &$object) {
    try {
      foreach ($object as $dsid => $datastream) {
        $datastream_context = array(
          'action' => 'ingest',
          'block' => FALSE,
        );
        islandora_alter_datastream($object, $datastream, $datastream_context);
        if ($datastream_context['block']) {
          throw new Exception(t('Object ingest blocked due to ingest of @dsid being blocked.', array(
            '@dsid' => $dsid,
          )));
        }
      }

      $object_context = array(
        'action' => 'ingest',
        'block' => FALSE,
      );
      islandora_alter_object($object, $object_context);
      if ($object_context['block']) {
        throw new Exception('Ingest Object was blocked.');
      }
      $ret = parent::ingestObject($object);
      islandora_invoke_object_hooks(ISLANDORA_OBJECT_INGESTED_HOOK, $object->models, $object);
      // Call the ingested datastream hooks for NewFedoraObject's after the
      // object had been ingested.
      foreach ($object as $dsid => $datastream) {
        islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_INGESTED_HOOK, $object->models, $dsid, $object, $datastream);
      }
      return $ret;
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to ingest object: @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $object->id,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
  }

  /**
   * Constructs a Fedora Object.
   *
   * @see FedoraRepository::constructObject
   */
  public function constructObject($id = NULL, $create_uuid = NULL) {
    // Enforces UUID when set, but allows to override if called
    // with $create_uuid set to bool.
    return parent::constructObject($id, static::useUUIDs($create_uuid));
  }

  /**
   * Get the next PID(s) from Repo.
   *
   * @see FedoraRepository::getNextIdentifier()
   */
  public function getNextIdentifier($namespace = NULL, $create_uuid = NULL, $count = 1) {
    // Enforces UUID when set, but allows to override if called
    // with $create_uuid set to bool.
    return parent::getNextIdentifier($namespace, static::useUUIDs($create_uuid), $count);
  }

  /**
   * Helper for three-valued logic with UUIDs.
   *
   * @param bool|NULL $to_create
   *   The variable to test.
   *
   * @return bool
   *   If $to_create is NULL, the value of the
   *   'islandora_basic_collection_generate_uuid' Drupal variable; otherwise,
   *   the value of $to_create itself.
   */
  protected static function useUUIDs($to_create) {
    return is_null($to_create) ?
      variable_get('islandora_basic_collection_generate_uuid', FALSE) :
      $to_create;
  }
}

class IslandoraRepositoryQuery extends RepositoryQuery {}

class IslandoraNewFedoraObject extends NewFedoraObject {
  protected $newFedoraDatastreamClass = 'IslandoraNewFedoraDatastream';
  protected $fedoraDatastreamClass = 'IslandoraFedoraDatastream';
  protected $fedoraRelsExtClass = 'IslandoraFedoraRelsExt';
}

class IslandoraFedoraObject extends FedoraObject {
  protected $newFedoraDatastreamClass = 'IslandoraNewFedoraDatastream';
  protected $fedoraDatastreamClass = 'IslandoraFedoraDatastream';
  protected $fedoraRelsExtClass = 'IslandoraFedoraRelsExt';

  /**
   * Magical magic, to allow recursive modifications.
   *
   * So... Magic functions in PHP are not re-entrant... Meaning that if you
   * have something which tries to call __set on an object anywhere later in
   * the callstack after it has already been called, it will not call the
   * magic method again; instead, it will set the property on the object
   * proper. Here, we detect the property being set on the object proper, and
   * restore the magic functionality as long as it keeps getting set...
   *
   * Not necessary to try to account for this in Tuque proper, as Tuque itself
   * does not have a mechanism to trigger modifications resulting from other
   * modifications.
   *
   * @param string $name
   *   The name of the property being set.
   * @param mixed $value
   *   The value to which the property should be set.
   */
  public function __set($name, $value) {
    parent::__set($name, $value);

    // Recursion only matters for magic properties... "Plain" properties cannot
    // call other code in order to start recursing, and in fact we would get
    // stuck looping with a "plain" property.
    if ($this->propertyIsMagical($name)) {
      // XXX: Due to the structure of the code, we cannot use property_exists()
      // (because many of the properties are declared in the class, and the
      // magic triggers due them being NULLed), nor can we use isset() (because
      // it is implemented as another magic function...).
      $vars = get_object_vars($this);
      while (isset($vars[$name])) {
        $new_value = $this->$name;
        unset($this->$name);
        parent::__set($name, $new_value);
        $vars = get_object_vars($this);
      }
    }
  }

  /**
   * Ingest the given datastream.
   *
   * @see FedoraObject::ingestDatastream()
   */
  public function ingestDatastream(&$datastream) {
    $object = $datastream->parent;
    $context = array(
      'action' => 'ingest',
      'block' => FALSE,
    );
    islandora_alter_datastream($object, $datastream, $context);
    try {
      if ($context['block']) {
        throw new Exception('Ingest Datastream was blocked.');
      }
      $ret = parent::ingestDatastream($datastream);
      islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_INGESTED_HOOK, $object->models, $datastream->id, $object, $datastream);
      return $ret;
    }
    catch (Exception $e) {
      if ($e instanceof DatastreamExistsException && variable_get('islandora_deprecation_return_false_when_datastream_exists', TRUE)) {
        $message = format_string('The datastream @dsid already exists on the object: @pid. As of 7.x-1.10 the function signature for ingestDatastream will be changed from returning FALSE on failure to throwing an exception.', array(
          '@dsid' => $datastream->id,
          '@pid' => $object->id,
        ));
        trigger_error(filter_xss($message), E_USER_DEPRECATED);
        return FALSE;
      }
      watchdog('islandora', 'Failed to ingest datastream @dsid on object: @pid</br>code: @code<br/>message: @msg', array(
        '@pid' => $object->id,
        '@dsid' => $datastream->id,
        '@code' => $e->getCode(),
        '@msg' => $e->getMessage(),
      ), WATCHDOG_ERROR);
      throw $e;
    }
  }

  /**
   * Inherits.
   *
   * Calls parent and invokes object modified and deleted(/purged) hooks.
   *
   * @see FedoraObject::modifyObject()
   */
  protected function modifyObject($params) {
    try {
      parent::modifyObject($params);
      islandora_invoke_object_hooks(ISLANDORA_OBJECT_MODIFIED_HOOK, $this->models, $this);
      if ($this->state == 'D') {
        islandora_invoke_object_hooks(ISLANDORA_OBJECT_PURGED_HOOK, $this->models, $this->id);
      }
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to modify object: @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $this->id,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
  }

  /**
   * Purge a datastream.
   *
   * Invokes datastream altered/purged hooks and calls the API-M method.
   *
   * @see FedoraObject::purgeObject()
   */
  public function purgeDatastream($id) {
    $this->populateDatastreams();

    if (!array_key_exists($id, $this->datastreams)) {
      return FALSE;
    }
    $context = array(
      'action' => 'purge',
      'purge' => TRUE,
      'delete' => FALSE,
      'block' => FALSE,
    );
    try {
      islandora_alter_datastream($this, $this[$id], $context);
      $action = $context['block'] ? 'block' : FALSE;
      $action = (!$action && $context['delete']) ? 'delete' : $action;
      $action = !$action ? 'purge' : $action;
      switch ($action) {
        case 'block':
          throw new Exception('Purge Datastream was blocked.');

        case 'delete':
          $this[$id]->state = 'D';
          return array();

        default:
          $to_return = parent::purgeDatastream($id);
          islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_PURGED_HOOK, $this->models, $id, $this, $id);
          return $to_return;
      }
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to purge datastream @dsid from @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $this->id,
          '@dsid' => $id,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
  }
}

class IslandoraRepositoryConnection extends RepositoryConnection {
  /**
   * Constructor.
   *
   * Invokes parent, but additionally invokes an alter to allow modules to
   * effect the configuration of the connection.
   */
  public function __construct($url = NULL, $username = NULL, $password = NULL) {
    if ($url === NULL) {
      $url = static::FEDORA_URL;
    }
    parent::__construct($url, $username, $password);
    drupal_alter('islandora_repository_connection_construction', $this);
  }
}

class IslandoraFedoraApi extends FedoraApi {

  /**
   * Instantiate a IslandoraFedoraApi object.
   *
   * @see FedoraApi::__construct()
   */
  public function __construct(IslandoraRepositoryConnection $connection, FedoraApiSerializer $serializer = NULL) {
    if (!$serializer) {
      $serializer = new FedoraApiSerializer();
    }
    $this->a = new FedoraApiA($connection, $serializer);
    $this->m = new IslandoraFedoraApiM($connection, $serializer);
    $this->connection = $connection;
  }
}

class IslandoraFedoraApiM extends FedoraApiM {

  /**
   * Update a datastream.
   *
   * Either changing its metadata, updaing the datastream contents or both.
   *
   * @throws Exception
   *   If the modify datastream request was block by some module.
   *
   * @see FedoraApiM::modifyDatastream
   */
  public function modifyDatastream($pid, $dsid, $params = array()) {
    $object = islandora_object_load($pid);
    $datastream = $object[$dsid];
    $context = array(
      'action' => 'modify',
      'block' => FALSE,
      'params' => $params,
    );
    islandora_alter_datastream($object, $datastream, $context);
    $params = $context['params'];
    // Anything may be altered during the alter_datastream hook invocation so
    // we need to update our time to the change we know about.
    if (isset($params['lastModifiedDate']) && $params['lastModifiedDate'] < (string) $object->lastModifiedDate) {
      $params['lastModifiedDate'] = (string) $object->lastModifiedDate;
    }
    if ($context['block']) {
      throw new Exception('Modify Datastream was blocked.');
    }
    return $this->callParentWithLocking('modifyDatastream', $pid, $pid, $dsid, $params);
  }

  /**
   * Update Fedora Object parameters.
   *
   * @see FedoraApiM::modifyObject
   */
  public function modifyObject($pid, $params = NULL) {
    $object = islandora_object_load($pid);
    $context = array(
      'action' => 'modify',
      'block' => FALSE,
      'params' => $params,
    );
    islandora_alter_object($object, $context);
    $params = $context['params'];
    if ($context['block']) {
      throw new Exception('Modify Object was blocked.');
    }
    return $this->callParentWithLocking('modifyObject', $pid, $pid, $params);
  }

  /**
   * Purge an object.
   *
   * @see FedoraApiM::purgeObject
   */
  public function purgeObject($pid, $log_message = NULL) {
    $object = islandora_object_load($pid);
    $context = array(
      'action' => 'purge',
      'purge' => TRUE,
      'delete' => FALSE,
      'block' => FALSE,
    );
    islandora_alter_object($object, $context);
    try {
      $action = $context['block'] ? 'block' : FALSE;
      $action = (!$action && $context['delete']) ? 'delete' : $action;
      $action = !$action ? 'purge' : $action;
      $models = $object->models;
      switch ($action) {
        case 'block':
          throw new Exception('Purge object was blocked.');

        case 'delete':
          $object->state = 'D';
          return '';

        default:
          $ret = $this->callParentWithLocking('purgeObject', $pid, $pid, $log_message);
          islandora_invoke_object_hooks(ISLANDORA_OBJECT_PURGED_HOOK, $models, $pid);
          return $ret;
      }
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to purge object @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $pid,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
  }

  /**
   * Wraps purgeDatastream for semaphore locking.
   *
   * @see FedoraApiM::purgeDatastream
   */
  public function purgeDatastream($pid, $dsid, $params = array()) {
    return $this->callParentWithLocking('purgeDatastream', $pid, $pid, $dsid, $params);
  }

  /**
   * Wraps ingest for semaphore locking.
   *
   * @see FedoraApiM::ingest
   */
  public function ingest($params = array()) {
    if (isset($params['pid'])) {
      return $this->callParentWithLocking('ingest', $params['pid'], $params);
    }
    else {
      return parent::ingest($params);
    }
  }

  /**
   * Wraps addDatastream for semaphore locking.
   *
   * @see FedoraApiM::addDatastream
   */
  public function addDatastream($pid, $dsid, $type, $file, $params) {
    return $this->callParentWithLocking('addDatastream', $pid, $pid, $dsid, $type, $file, $params);
  }

  /**
   * Wraps addRelationship for semaphore locking.
   *
   * @see FedoraApiM::addRelationship
   */
  public function addRelationship($pid, $relationship, $is_literal, $datatype = NULL) {
    return $this->callParentWithLocking('addRelationship', $pid, $pid, $relationship, $is_literal, $datatype);
  }

  /**
   * Call a parent function while using semaphores as configured.
   *
   * All extra arguments are passed along to the callback.
   *
   * @param callable $callback
   *   The method we are wrapping.
   * @param string $pid
   *   The PID to create a semaphore for.
   */
  protected function callParentWithLocking($callback, $pid) {
    $args = array_slice(func_get_args(), 2);
    $locked = FALSE;

    if (variable_get('islandora_use_object_semaphores', FALSE)) {
      $lock_period = variable_get('islandora_semaphore_period', 600);
      while (!lock_acquire($pid, $lock_period)) {
        // Wait for the lock to be free. In the worst case forever.
        while (lock_wait($pid)) {
        }
      }
      $locked = TRUE;
    }

    if ($locked) {
      try {
        $to_return = call_user_func_array(array($this, "parent::$callback"), $args);
      }
      catch (Exception $e) {
        // Release the lock in event of exception.
        lock_release($pid);
        throw $e;
      }
      lock_release($pid);
      return $to_return;
    }
    else {
      return call_user_func_array(array($this, "parent::$callback"), $args);
    }
  }
}

class IslandoraSimpleCache extends SimpleCache {}

class IslandoraNewFedoraDatastream extends NewFedoraDatastream {
  protected $fedoraRelsIntClass = 'IslandoraFedoraRelsInt';
  protected $fedoraDatastreamVersionClass = 'IslandoraFedoraDatastreamVersion';
}

class IslandoraFedoraDatastream extends FedoraDatastream {
  protected $fedoraRelsIntClass = 'IslandoraFedoraRelsInt';
  protected $fedoraDatastreamVersionClass = 'IslandoraFedoraDatastreamVersion';

  /**
   * Magical magic, to allow recursive modifications.
   *
   * So... Magic functions in PHP are not re-entrant... Meaning that if you
   * have something which tries to call __set on an object anywhere later in
   * the callstack after it has already been called, it will not call the
   * magic method again; instead, it will set the property on the object
   * proper. Here, we detect the property being set on the object proper, and
   * restore the magic functionality as long as it keeps getting set...
   *
   * Not necessary to try to account for this in Tuque proper, as Tuque itself
   * does not have a mechanism to trigger modifications resulting from other
   * modifications.
   *
   * @param string $name
   *   The name of the property being set.
   * @param mixed $value
   *   The value to which the property should be set.
   */
  public function __set($name, $value) {
    parent::__set($name, $value);

    // Recursion only matters for magic properties... "Plain" properties cannot
    // call other code in order to start recursing, and in fact we would get
    // stuck looping with a "plain" property.
    if ($this->propertyIsMagical($name)) {
      // XXX: Due to the structure of the code, we cannot use property_exists()
      // (because many of the properties are declared in the class, and the
      // magic triggers due them being NULLed), nor can we use isset() (because
      // it is implemented as another magic function...).
      $vars = get_object_vars($this);
      while (isset($vars[$name])) {
        $new_value = $this->$name;
        unset($this->$name);
        parent::__set($name, $new_value);
        $vars = get_object_vars($this);
      }
    }
  }

  /**
   * Inherits.
   *
   * Calls parent and invokes modified and purged hooks.
   *
   * @see FedoraDatastream::modifyDatastream()
   */
  protected function modifyDatastream(array $args) {
    try {
      parent::modifyDatastream($args);
      islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_MODIFIED_HOOK, $this->parent->models, $this->id, $this->parent, $this, $args);
      if ($this->state == 'D') {
        islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_PURGED_HOOK, $this->parent->models, $this->id, $this->parent, $this->id);
      }
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to modify datastream @dsid from @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $this->parent->id,
          '@dsid' => $this->id,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
  }
}

class IslandoraFedoraDatastreamVersion extends FedoraDatastreamVersion {
  protected $fedoraRelsIntClass = 'IslandoraFedoraRelsInt';
  protected $fedoraDatastreamVersionClass = 'IslandoraFedoraDatastreamVersion';
}

class IslandoraFedoraRelsExt extends FedoraRelsExt {}

class IslandoraFedoraRelsInt extends FedoraRelsInt {}
