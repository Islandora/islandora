<?php

/**
 * @file
 * Defines the class IslandoraWebTestCase, which allows tests to access Fedora.
 */

class IslandoraWebTestCase extends DrupalWebTestCase {

  /**
   * Sets up the Drupal filter to access this test Drupal instances database.
   *
   * @see DrupalWebTestCase::setUp()
   */
  public function setUp() {
    $args = func_get_args();
    $args = (isset($args[0]) && is_array($args[0])) ? $args[0] : $args;
    // Always enable islandora.
    $args[] = 'islandora';
    parent::setUp($args);
    $this->configuration = $this->getTestConfiguration();
    if ($this->configuration['use_drupal_filter']) {
      $this->backUpDrupalFilter();
      $this->setUpDrupalFilter();
    }
    $this->createAdminUser();
  }

  /**
   * Parses and returns the settings from the test configuration file.
   *
   * If no install specific test_config.ini file is found, it will use the
   * assumed default configs found in default.test_config.ini.
   *
   * @return array
   *   The test configuration.
   *
   * @see parse_ini_file()
   */
  protected function getTestConfiguration() {
    $path = drupal_get_path('module', 'islandora');
    if (file_exists("$path/tests/test_config.ini")) {
      $this->pass('Using custom test configuration.');
      return parse_ini_file("$path/tests/test_config.ini");
    }
    elseif (file_exists("$path/tests/default.test_config.ini")) {
      $this->pass('Using default test configuration.');
      return parse_ini_file("$path/tests/default.test_config.ini");
    }
    throw new Exception('Required default.test_config.ini/test_config.ini file not found');
  }

  /**
   * Stores the content of the Drupal Filter for later restoration.
   */
  protected function backUpDrupalFilter() {
    if (file_exists($this->configuration['drupal_filter_file'])) {
      $this->originalDrupalFilterContent = file_get_contents($this->configuration['drupal_filter_file']);
    }
    else {
      throw new Exception('Failed to find the required Drupal Filter configuration file.');
    }
  }

  /**
   * Sets up a drupal filter that can read for the tests users table.
   */
  protected function setUpDrupalFilter() {
    $connection_info = Database::getConnectionInfo('default');
    $drupal_filter_dom = new DomDocument();
    $drupal_filter_dom->loadXML($this->originalDrupalFilterContent);
    $drupal_filter_xpath = new DOMXPath($drupal_filter_dom);
    $server = $connection_info['default']['host'];
    $dbname = $connection_info['default']['database'];
    $user = $connection_info['default']['username'];
    $password = $connection_info['default']['password'];
    $port = $connection_info['default']['port'] ? $connection_info['default']['port'] : '3306';
    $prefix = $connection_info['default']['prefix']['default'];
    $results = $drupal_filter_xpath->query("/FilterDrupal_Connection/connection[@server='$server' and @dbname='$dbname' and @user='$user' and @password='$password' and @port='$port']/sql");
    $results->item(0)->nodeValue = "SELECT DISTINCT u.uid AS userid, u.name AS Name, u.pass AS Pass, r.name AS Role FROM ({$prefix}users u LEFT JOIN {$prefix}users_roles ON u.uid={$prefix}users_roles.uid) LEFT JOIN {$prefix}role r ON r.rid={$prefix}users_roles.rid WHERE u.name=? AND u.pass=?;";
    file_put_contents($this->configuration['drupal_filter_file'], $drupal_filter_dom->saveXML());
  }

  /**
   * Creates the a full fedora admin user with a repository connection.
   */
  protected function createAdminUser() {
    $this->admin = new stdClass();
    $this->admin->uid = 1;
    $this->admin->name = $this->configuration['admin_user'];
    $this->admin->pass = $this->configuration['admin_pass'];
    $url = variable_get('islandora_base_url', $this->configuration['fedora_url']);
    $connection = islandora_get_tuque_connection($this->admin, $url);
    $this->admin->repository = $connection->repository;
    return $this->admin;
  }
  /**
   * Stores the content of the Drupal Filter for later restoration.
   */
  protected function restoreDrupalFilter() {
    $file = $this->configuration['drupal_filter_file'];
    if (isset($this->originalDrupalFilterContent)) {
      file_put_contents($file, $this->originalDrupalFilterContent);
    }
    elseif (file_exists($file)) {
      // Remove if there was never an original.
      drupal_unlink($file);
    }
  }

  /**
   * Restores the original Drupal filter, frees any allocated resources.
   *
   * @see DrupalWebTestCase::tearDown()
   */
  public function tearDown() {
    if ($this->configuration['use_drupal_filter']) {
      $this->restoreDrupalFilter();
    }
    unset($this->admin);
    unset($this->configuration);
    parent::tearDown();
  }
}
