<?php

/**
 * @file
 * Wrapper around the tuque library, allows for autoloading of Islandora Tuque
 * Objects.
 *
 * @todo Overload functions and apply pre/post hooks.
 */

$islandora_module_path = drupal_get_path('module', 'islandora');

@include_once 'sites/all/libraries/tuque/Bootstrap.php';

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
 * Constructs a list of hooks from the given paramenters and invokes them.
 */
function islandora_invoke_object_hooks($hook, array $models) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  return islandora_invoke_hook_list($hook, $models, array_slice(func_get_args(), 2));
}

/**
 * Constructs a list of hooks from the given paramenters and invokes them.
 */
function islandora_invoke_datastream_hooks($hook, array $models, $dsid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $refinements = array();
  foreach ($models as $model) {
    $refinements[] = "{$model}_{$dsid}";
  }
  return islandora_invoke_hook_list($hook, $refinements, array_slice(func_get_args(), 3));
}

class IslandoraFedoraRepository extends Tuque\Repository {

  /**
   * The decorator class to wrap existing objects.
   * @var string
   */
  protected $objectDecorator = 'IslandoraFedoraObject';
  protected $newObjectDecorator = 'IslandoraNewFedoraObject';

  /**
   * Creates a repository from the given configuration.
   */
  public static function fromConfig(Tuque\RepositoryConfig $config) {
    return new IslandoraFedoraRepository(Tuque\RepositoryFactory::getRepository($config));
  }

  /**
   * Constructor for the Repository object.
   *
   * @param AbstractRepository $repository
   *   The configuration setting that defines what kind of repository to
   *   instantiate.
   */
  public function __construct(AbstractRepository $repository) {
    parent::__construct($repository);
    // @todo Get rid of this class at some point.
    $this->api->m = new IslandoraFedoraApiM($this->api->m);
  }

  /**
   * Ingest the given object.
   *
   * @see FedoraRepository::ingestObject()
   */
  public function ingestObject(AbstractObject &$object) {
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
}

class IslandoraFedoraObject extends Tuque\Object {

  /**
   * The decorator class to wrap the repository reference.
   * @var string
   */
  protected $repositoryDecorator = 'IslandoraFedoraRepository';

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
      watchdog('islandora', 'Failed to ingest object: @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $object->id,
          '@dsid' => $datastream->id,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
  }

  /**
   * Get the string repersentation of this object.
   */
  public function __tostring() {
    return $this->id;
  }

}

class IslandoraNewFedoraObject extends Tuque\NewObject {

  /**
   * The decorator class to wrap the repository reference.
   * @var string
   */
  protected $repositoryDecorator = 'IslandoraFedoraRepository';

  /**
   * Get the string repersentation of this object.
   */
  public function __tostring() {
    return $this->id;
  }

}

/**
 * @todo Find a way to use this without referencing the implementation class.
 */
class IslandoraFedoraApiM extends Tuque\Delegate {

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
    if (isset($params['lastModifiedDate'])) {
      $params['lastModifiedDate'] = (string) $object[$dsid]->createdDate;
    }
    try {
      if ($context['block']) {
        throw new Exception('Modify Datastream was blocked.');
      }
      $ret = parent::modifyDatastream($pid, $dsid, $params);
      islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_MODIFIED_HOOK, $object->models, $dsid, $object, $datastream);
      if (isset($params['dsState']) && $params['dsState'] == 'D') {
        islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_PURGED_HOOK, $object->models, $dsid, $object, $dsid);
      }
      return $ret;
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to modify datastream @dsid from @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $pid,
          '@dsid' => $dsid,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
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
    try {
      if ($context['block']) {
        throw new Exception('Modify Object was blocked.');
      }
      $ret = parent::modifyObject($pid, $params);
      islandora_invoke_object_hooks(ISLANDORA_OBJECT_MODIFIED_HOOK, $object->models, $object);
      if (isset($params['state']) && $params['state'] == 'D') {
        islandora_invoke_object_hooks(ISLANDORA_OBJECT_PURGED_HOOK, $object->models, $object->id);
      }
      return $ret;
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to modify object: @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $pid,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
  }

  /**
   * Purge a datastream from from Fedora.
   *
   * @see FedoraApiM::purgeDatastream
   */
  public function purgeDatastream($pid, $dsid, $params = array()) {
    $object = islandora_object_load($pid);
    $context = array(
      'action' => 'purge',
      'purge' => TRUE,
      'delete' => FALSE,
      'block' => FALSE,
    );
    islandora_alter_datastream($object, $object[$dsid], $context);
    try {
      $action = $context['block'] ? 'block' : FALSE;
      $action = (!$action && $context['delete']) ? 'delete' : $action;
      $action = !$action ? 'purge' : $action;
      switch ($action) {
        case 'block':
          throw new Exception('Purge Datastream was blocked.');

        case 'delete':
          $object[$dsid]->state = 'D';
          return array();

        default:
          $ret = parent::purgeDatastream($pid, $dsid, $params);
          islandora_invoke_datastream_hooks(ISLANDORA_DATASTREAM_PURGED_HOOK, $object->models, $dsid, $object, $dsid);
          return $ret;
      }
    }
    catch (Exception $e) {
      watchdog('islandora', 'Failed to purge datastream @dsid from @pid</br>code: @code<br/>message: @msg', array(
          '@pid' => $pid,
          '@dsid' => $dsid,
          '@code' => $e->getCode(),
          '@msg' => $e->getMessage()), WATCHDOG_ERROR);
      throw $e;
    }
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
          $ret = parent::purgeObject($pid, $log_message);
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

}
