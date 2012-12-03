<?php

/**
 * @file
 *
 * Wrapper around the tuque library, allows for autoloading of Islandora Tuque
 * Objects.
 *
 * @todo Overload functions and apply pre/post hooks.
 */

$islandora_module_path = drupal_get_path('module', 'islandora');

//do this until we expost these in a module or library
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

class IslandoraFedoraRepository extends FedoraRepository {
  protected $queryClass = 'IslandoraRepositoryQuery';
  protected $newObjectClass = 'IslandoraNewFedoraObject';
  protected $objectClass = 'IslandoraFedoraObject';
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
}

class IslandoraRepositoryConnection extends RepositoryConnection {}

class IslandoraFedoraApi extends FedoraApi {}

class IslandoraSimpleCache extends SimpleCache {}

class IslandoraNewFedoraDatastream extends NewFedoraDatastream {
  protected $fedoraRelsIntClass = 'IslandoraFedoraRelsInt';
  protected $fedoraDatastreamVersionClass = 'IslandoraFedoraDatastreamVersion';
}

class IslandoraFedoraDatastream extends FedoraDatastream {
  protected $fedoraRelsIntClass = 'IslandoraFedoraRelsInt';
  protected $fedoraDatastreamVersionClass = 'IslandoraFedoraDatastreamVersion';
}

class IslandoraFedoraDatastreamVersion extends FedoraDatastreamVersion {
  protected $fedoraRelsIntClass = 'IslandoraFedoraRelsInt';
  protected $fedoraDatastreamVersionClass = 'IslandoraFedoraDatastreamVersion';
}

class IslandoraFedoraRelsExt extends FedoraRelsExt {}

class IslandoraFedoraRelsInt extends FedoraRelsInt {}
