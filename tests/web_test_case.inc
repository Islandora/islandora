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
   * Creates a temporary Drupal user with administrative privileges. This is much simpler than the probably-unnecessary process of determining the correct permissions for most tests.
   *
   * @param $permissions
   *   An array of permissions which the temporary admin user will adopt; should be left blank, as this will just pull all the permissions currently assigned to the Administrator role.
   *
   */
  public function createDrupalAdminUser(array $permissions = array()) {
    $roles = user_roles();
    $index = array_search('administrator', $roles);
    $user = $this->drupalCreateUser($permissions);
    $user->roles[$index] = 'administrator';
    return user_save($user);
  }

  /**
   * Creates a randomly generated child collection in the top-level collection through the testing interface. This should be used in lieu of throwing test objects into a standard collection, in case that collection has been deleted.
   *
   * @param $label
   *   The label that should be assigned to the collection
   * @param $model
   *   The content model the collection should use
   * @param $pid
   *   The PID that should be assigned to the collection (left blank by default, using a sequentially assigned number)
   * @see IslandoraWebTestCase::deleteRootTestCollection()
   */
  public function createRootTestCollection($label, $model, $pid = '') {
    $this->drupalGet('islandora');
    $root = $this->url;
    $path = $root . '/manage/collection';
    $this->clickLink(t('Manage'));
    $this->clickLink(t('Collection'));
    $edit = array('label' => $label, 'pid' => $pid, 'inherit_policy' => FALSE, 'content_models[' . $model . ']' => TRUE);
    $this->drupalPost($path, $edit, t('Create collection'), $options = array(), $headers = array(), $form_html_id = 'islandora-basic-collection-create-child-collection-form');
    $this->assertText($label, t('Created collection %label using the content model %model', array('%label' => $label, '%model' => $model)), 'Islandora');
  }

  /**
   * Deletes a child collection from the root collection by searching for a particular label.
   *
   * @param $label
   *   The label of the collection to be deleted
   * @see IslandoraWebTestCase::createRootTestCollection()
   */
  public function deleteRootTestCollectionByLabel($label) {
    $collection = $this->openRootTestCollectionByLabel($label);
    if ($collection != FALSE) {
      $this->clickLink('Manage');
      $this->clickLink('Properties');
      $path = $this->url;

      $edit = array();
      $this->drupalPost($path, $edit, t('Delete Collection'));
      $path = $this->url;
      $this->drupalPost($path, $edit, t('Delete'));
      $this->pass(t('Deleted collection %label by searching the root collection', array('%label' => $label)), t('Islandora'));
    }
  }

  /**
   * Deletes a child collection from the root collection directly via PID
   */
  public function deleteRootTestCollectionByPid($label, $pid) {
    $path = 'islandora/object/' . $pid . '/manage/properties';
    $edit = array();
    $this->drupalPost($path, $edit, t('Delete Collection'));
    $path = $this->url;
    $this->drupalPost($path, $edit, t('Delete'));
    $this->assertText('children from', t('Deleted collection %label directly via PID %pid', array('%label' => $label, '%pid' => $pid)), t('Islandora'));
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
   * Searches through the pages inside the root collection and opens the one specified by its label. This is much slower than doing it directly by PID; use this in tests where the PID is unknown.
   *   @param $label
   *     The label of the collection
   *
   */
  public function openRootTestCollectionByLabel($label, $index = 0) {
    $nexturls = $this->xpath('//a[normalize-space(text())=:label]', array(':label' => 'next >'));
    if (isset($nexturls[$index])) {
      while (isset($nexturls[$index])) {
        $urls = $this->xpath('//a[normalize-space(text())=:label]', array(':label' => $label));

        if (isset($urls[$index])) {
          $url_target = $this->getAbsoluteUrl($urls[$index]['href']);
          }

        if (isset($url_target)) {
          $this->assertTrue(isset($urls[$index]), t('Found collection %label in root collection', array('%label' => $label)), t('Islandora'));
          return $this->drupalGet($url_target);
          }
        }
        return FALSE;
      }
      else {
        $urls = $this->xpath('//a[normalize-space(text())=:label]', array(':label' => $label));

        if (isset($urls[$index])) {
          $url_target = $this->getAbsoluteUrl($urls[$index]['href']);
          }

        if (isset($url_target)) {
          $this->assertTrue(isset($urls[$index]), t('Found collection %label in root collection', array('%label' => $label)), t('Islandora'));
          return $this->drupalGet($url_target);
        }
      }
      return FALSE;
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
