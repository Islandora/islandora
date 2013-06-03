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

    // Its possible test are running before class autoloading.
    module_load_include('inc', 'islandora', 'includes/tuque');
    module_load_include('inc', 'islandora', 'includes/tuque_wrapper');

    $this->configuration = $this->getTestConfiguration();
    if ($this->configuration['use_drupal_filter']) {
      $this->backUpDrupalFilter();
      $this->setUpDrupalFilter();
    }
    $this->admin = $this->createAdminUser();
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
    $roles = user_roles();
    $index = array_search('administrator', $roles);
    $user = $this->drupalCreateUser();
    $user->roles[$index] = 'administrator';
    $user->name = $this->configuration['admin_user'];
    $user->pass = $this->configuration['admin_pass'];
    $user = user_save($user);
    $url = variable_get('islandora_base_url', $this->configuration['fedora_url']);
    $connection = islandora_get_tuque_connection($user, $url);
    $user->repository = $connection->repository;
    return $user;
  }

  /**
   * Logs in the given user, handles the special case where the user is admin.
   *
   * @see DrupalWebTestCase::drupalLogin()
   */
  protected function drupalLogin(stdClass $account) {
    if ($account->uid == $this->admin->uid) {
      // Create password for Drupal.
      $edit = array('pass' => user_password());
      $account = user_save($account, $edit);
      // Raw password is used to login.
      $account->pass_raw = $edit['pass'];
      // We must login before changing the password for fedora.
      parent::drupalLogin($account);
      $account->name = $this->configuration['admin_user'];
      $account->pass = $this->configuration['admin_pass'];
      // Save the fedora admin credentials for later GET/POST requests.
      $account = user_save($account);
    }
    else {
      parent::drupalLogin($account);
    }
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

  /**
   * Asserts that the given datastreams exist on the object.
   *
   * @param AbstractObject $object
   *   The PID of the object
   * @param array $datastreams
   *   An array of strings containing  datastream names
   */
  public function assertDatastreams($object, array $datastreams) {
    if (!is_object($object)) {
      $this->fail("Failed. Object passed in is invalid.");
      return;
    }

    foreach ($datastreams as $datastream) {
      if (isset($object[$datastream])) {
        $this->pass("Loaded datastream {$datastream} from PID {$object->id}");
      }
      else {
        $this->fail("Failed to load datastream {$datastream} from PID {$object->id}");
      }
    }
  }

  /**
   * Gets a tuque object from a path.
   *
   * @param string $path
   *   A full or partial path to an islandora object.
   *
   * @return AbstractObject
   *   The pid of the object or FALSE if a PID is not found.
   */
  public function getObjectFromPath($path) {
    $path_parts = explode('/', $path);
    $array_length = count($path_parts);
    for ($i = 0; $i < $array_length; $i++) {
      if ($path_parts[$i] == 'islandora' && isset($path_parts[$i + 1]) && $path_parts[$i + 1] == 'object') {
        if (isset($path_parts[$i + 2])) {
          return islandora_object_load(urldecode($path_parts[$i + 2]));
        }
      }
    }
    $this->fail("Failed to parse path : $path.");
    return FALSE;
  }

  /**
   * Deletes an object using the PID. This does the deletion using the UI.
   *
   * @param string $pid
   *   The PID of the collection to be deleted
   */
  public function deleteObject($pid) {
    $current_user = $this->loggedInUser;
    $user = $this->drupalCreateUser(array(
      'manage object properties',
      'delete fedora objects and datastreams',
      'view fedora repository objects',
    ));

    $this->drupalLogin($user);

    $path = 'islandora/object/' . $pid . '/manage/properties';
    $edit = array();
    $this->drupalPost($path, $edit, t('Delete'));
    $this->drupalPost($this->url, $edit, t('Delete'));
    $object = islandora_object_load($pid);

    $this->drupalGet("islandora/object/$pid");
    $this->assertResponse(404, "Object $pid successfully deleted.");

    if ($current_user) {
      $this->drupalLogin($current_user);
    }
    else {
      $this->drupalLogout();
    }
  }
}
