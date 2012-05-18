<?php

//do this until we expost these in a module or library
require_once 'sites/all/libraries/tuque/Datastream.php';
require_once 'sites/all/libraries/tuque/FedoraApi.php';
require_once 'sites/all/libraries/tuque/FedoraApiSerializer.php';
require_once 'sites/all/libraries/tuque/Object.php';
require_once 'sites/all/libraries/tuque/RepositoryConnection.php';
require_once 'sites/all/libraries/tuque/Cache.php';
require_once 'sites/all/libraries/tuque/RepositoryException.php';
require_once 'sites/all/libraries/tuque/Repository.php';

class RestConnection {

  public $connection = NULL;
  public $api = NULL;
  public $cache = NULL;
  public $repository = NULL;

  function RestConnection($user) {
    $url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
    $this->connection = new RepositoryConnection($url, $user->name, $user->password);
    $this->connection->reuseConnection = TRUE;
    $this->api = new FedoraApi($this->connection);
    $this->cache = new SimpleCache();
    $this->repository = new FedoraRepository($this->api, $this->cache);
  }

}

