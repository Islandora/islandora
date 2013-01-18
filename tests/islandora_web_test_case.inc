<?php

class IslandoraWebTestCase extends DrupalWebTestCase {

  public function setUp() {
    $args = func_get_args();
    call_user_func_array(array('parent', 'setUp'), $args);

    $islandora_path = drupal_get_path('module', 'islandora');
    $this->config_info = parse_ini_file("$islandora_path/tests/test_config.ini");
    $connection_info = Database::getConnectionInfo('default');

    $this->original_drupal_fiter = file_get_contents($this->config_info['filter_drupal_file']);

    $drupal_filter_dom = new DomDocument();
    $drupal_filter_dom->loadXML($this->original_drupal_fiter);
    $drupal_filter_xpath = new DOMXPath($drupal_filter_dom);

    $server = $connection_info['default']['host'];
    $dbname = $connection_info['default']['database'];
    $user = $connection_info['default']['username'];
    $password = $connection_info['default']['password'];
    $port = $connection_info['default']['port'] ? $connection_info['default']['port'] : '3306';
    $prefix = $connection_info['default']['prefix']['default'];

    $results = $drupal_filter_xpath->query("/FilterDrupal_Connection/connection[@server='$server' and @dbname='$dbname' and @user='$user' and @password='$password' and @port='$port']/sql");
    $results->item(0)->nodeValue = "SELECT DISTINCT u.uid AS userid, u.name AS Name, u.pass AS Pass, r.name AS Role FROM ({$prefix}users u LEFT JOIN {$prefix}users_roles ON u.uid={$prefix}users_roles.uid) LEFT JOIN {$prefix}role r ON r.rid={$prefix}users_roles.rid WHERE u.name=? AND u.pass=?;";

    file_put_contents($this->config_info['filter_drupal_file'], $drupal_filter_dom->saveXML());
  }

  public function tearDown() {
    file_put_contents($this->config_info['filter_drupal_file'], $this->original_drupal_fiter);
    parent::tearDown();
  }
}