<?php

/**
 * @file
 *
 * Wrapper around the tuque library, allows for autoloading of Islandora Tuque
 * Objects.
 *
 * @todo Overload functions and apply pre/post hooks.
 */

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
