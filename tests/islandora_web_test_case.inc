<?php

/**
 * @file
 * Defines the class IslandoraWebTestCase, which allows tests to access Fedora.
 */

class IslandoraWebTestCase extends DrupalWebTestCase {

  /**
   * An array of users that may be created over the course of a test.
   *
   * @var array
   */
  protected $users = array();

  /**
   * By default, deleteUserCreatedObjects() runs on each tearDown() step.
   */
  protected $deleteObjectsOnTeardown = TRUE;

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
    $server = $connection_info['default']['host'];
    $dbname = $connection_info['default']['database'];
    $user = $connection_info['default']['username'];
    $password = $connection_info['default']['password'];
    $port = $connection_info['default']['port'] ? $connection_info['default']['port'] : '3306';
    $prefix = $connection_info['default']['prefix']['default'];
    $filter_drupal_connection_node = $drupal_filter_dom->getElementsByTagName('FilterDrupal_Connection')->item(0);
    $first_connection_node = $drupal_filter_dom->getElementsByTagName('connection')->item(0);
    $connection_node = $filter_drupal_connection_node->insertBefore($drupal_filter_dom->createElement('connection'), $first_connection_node);
    $connection_node->setAttributeNode(new DOMAttr('server', $server));
    $connection_node->setAttributeNode(new DOMAttr('dbname', $dbname));
    $connection_node->setAttributeNode(new DOMAttr('user', $user));
    $connection_node->setAttributeNode(new DOMAttr('password', $password));
    $connection_node->setAttributeNode(new DOMAttr('port', $port));
    $sql_node = $connection_node->appendChild(new DOMElement('sql'));
    $sql_node->appendChild($drupal_filter_dom->createTextNode("SELECT DISTINCT u.uid AS userid, u.name AS Name, u.pass AS Pass, r.name AS Role FROM ({$prefix}users u LEFT JOIN {$prefix}users_roles ON u.uid={$prefix}users_roles.uid) LEFT JOIN {$prefix}role r ON r.rid={$prefix}users_roles.rid WHERE u.name=? AND u.pass=?;"));
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
      $this->users[] = $account->name;
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
   * To safeguard against leaving test objects in the repository, tearDown()
   * calls deleteUserCreatedObjects() every time by default. This feature can be
   * toggled by setting $this->deleteObjectsOnTeardown to TRUE or FALSE.
   *
   * @see DrupalWebTestCase::tearDown()
   */
  public function tearDown() {
    if ($this->deleteObjectsOnTeardown) {
      foreach ($this->users as $user) {
        $this->deleteUserCreatedObjects($user);
      }
    }
    if ($this->configuration['use_drupal_filter']) {
      $this->restoreDrupalFilter();
    }
    unset($this->admin);
    unset($this->configuration);
    parent::tearDown();
  }

  /**
   * Asserts that the given datastreams exist correctly on the object.
   *
   * @param AbstractObject $object
   *   The PID of the object
   * @param array $datastreams
   *   An array of strings containing datastream names
   *
   * @return bool
   *   TRUE on success, FALSE on fail.
   */
  public function assertDatastreams($object, array $datastreams) {
    if (!is_object($object)) {
      $this->fail("Failed. Object passed in is invalid.", 'Islandora');
    }
    else {
      $missing_datastreams = array_diff_key(array_flip($datastreams), $this->admin->repository->api->a->listDatastreams($object->id));

      if (!empty($missing_datastreams)) {
        $this->fail("Failed to find datastream(s) " . implode(', ', array_flip($missing_datastreams)) . " in object {$object->id}.");
        return FALSE;
      }

      $this->pass("Found required datastream(s) in object {$object->id}");
      return TRUE;
    }
  }

  /**
   * Attempts to validate an array of datastreams, generally via binary checks.
   *
   * These functions exist in, and can be added to, datastream_validators.inc,
   * which is found in this folder.
   *
   * $param AbstractObject $object
   *   The object to load datastreams from.
   * $param array $datastreams
   *   An array of paired DSIDs, validate function names, and optional params.
   */
  public function validateDatastreams($object, array $datastreams) {
    if (!is_object($object)) {
      $this->fail("Failed. Object passed in is invalid.", 'Islandora');
    }
    module_load_include('inc', 'islandora', 'tests/datastream_validators');
    foreach ($datastreams as $datastream) {
      if (isset($object[$datastream[0]])) {
        $function = 'islandora_validate_' . $datastream[1] . '_datastream';
        if (function_exists($function)) {
          if (isset($datastream[2])) {
            $results = $function($object, $datastream[0], $datastream[2]);
          }
          else {
            $results = $function($object, $datastream[0]);
          }
          foreach ($results as $result) {
            $this->assertTrue($result[0], $result[1], 'Islandora');
          }
        }
        else {
          $this->fail("No {$datastream[0]} validation function exists for the {$datastream[1]} datastream.", 'Islandora');
        }
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
    $this->fail("Failed to parse path: $path.");
    return FALSE;
  }

  /**
   * Deletes an object using the PID. This does the deletion using the UI.
   *
   * @param string $pid
   *   The PID of the collection to be deleted
   * @param string $button
   *   The label of the first 'Delete' button
   * @param bool $safety
   *   If TRUE, this will only delete objects owned by users in $this->users.
   */
  public function deleteObject($pid, $button = NULL, $safety = TRUE) {
    $object = islandora_object_load($pid);
    if (!$safety || in_array($object->owner, $this->users)) {
      $path = "islandora/object/$pid/manage/properties";
      if (isset($button)) {
        $this->drupalPost($path, array(), $button);
      }
      else {
        $object = islandora_object_load($pid);
        $this->drupalPost($path, array(), "Permanently remove '{$object->label}' from repository");
      }
      $this->drupalPost($this->url, array(), t('Delete'));

      $this->drupalGet("islandora/object/$pid");
      $this->assertResponse(404, "Object $pid successfully deleted.");
    }
    else {
      $this->fail("Cannot delete object {$pid}; it is owned by non-test user {$object->owner}, and this function was called with the safety on.");
      return FALSE;
    }
  }

  /**
   * Constructs and ingests a Fedora object and datastream(s) via tuque.
   *
   * All keys inside the parameter arrays for this function are optional. it
   * can be run simply by calling $this->ingestConstructedObject();.
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
   *   'dsid' - the datastream ID; randomized if not set.
   *   'path' - The path to the file to use; defaults to fixtures/test.jpg.
   *   'control_group' - The single-letter control group identifier.
   *   'mimetype' - The datastream's mimetype.
   *
   * @return bool|array
   *   FALSE if the object ingest failed, or the object array if successful.
   */
  public function ingestConstructedObject(array $properties = array(), array $datastreams = array()) {
    $tuque = islandora_get_tuque_connection($this->admin);
    $repository = $tuque->repository;
    if (!isset($properties['pid'])) {
      $properties['pid'] = "islandora";
    }
    $object = $repository->constructObject($properties['pid']);

    // Set the object properties before ingesting it.
    if (isset($properties['label'])) {
      $object->label = $properties['label'];
    }
    else {
      $properties['label'] = $this->randomName(16);
      $object->label = $properties['label'];
    }

    if (isset($properties['owner'])) {
      $object->owner = $properties['owner'];
    }
    elseif ($this->loggedInUser !== FALSE) {
      $object->owner = $this->loggedInUser->name;
    }

    if (isset($properties['models'])) {
      try {
        $object->models = (array) $properties['models'];
      }
      catch (Exception $e) {
        $this->fail("Encountered an exception when trying to add content models to {$object->id}: $e");
        return FALSE;
      }
    }

    $repository->ingestObject($object);
    if (!$object) {
      $this->fail(t("Failed to ingest object."), 'Islandora');
      return FALSE;
    }
    else {
      $this->pass(t("Ingested object %object", array('%object' => $object->id)), 'Islandora');
    }

    // Chuck in some datastreams.
    if (!empty($datastreams)) {
      foreach ($datastreams as $datastream) {
        if (!isset($datastream['dsid'])) {
          $datastream['dsid'] = $this->randomName(8);
        }
        if (!isset($datastream['path'])) {
          $datastream['path'] = drupal_get_path('module', 'islandora') . '/tests/fixtures/test.jpg';
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
        $new_datastream->setContentFromFile($datastream['path']);
        $object->ingestDatastream($new_datastream);
      }
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
   *   TRUE on success, FALSE on failure.
   */
  public function deleteUserCreatedObjects($username) {
    if ($username === $this->configuration['admin_user']) {
      $this->fail("This function will under no circumstance attempt deletion of all objects owned by the configured Fedora admin user ({$this->configuration['admin_user']}), as this could irreparably damage the repository.", 'Islandora');
      return FALSE;
    }

    $query = <<<QUERY
SELECT ?object FROM <#ri> WHERE
{
  ?object <fedora-model:ownerId> "$username"
}
QUERY;

    $objects = $this->admin->repository->ri->sparqlQuery($query);
    foreach ($objects as $object) {
      $loaded_object = islandora_object_load($object['object']['value']);
      islandora_delete_object($loaded_object);
      if ($this->assertFalse(islandora_object_load($object['object']['value']), "Object {$object['object']['value']} successfully removed from repository.", 'Islandora')) {
        return FALSE;
      }
      return TRUE;
    }
  }

  /**
   * These are a few quick helper functions to fill in a gap in the standard
   * DrupalWebTestCase where no function exists to test for the simple existence
   * or non-existence of an error.
   */

  /**
   * Asserts that an error is found in $this->content.
   *
   * @param string $message
   *   The message to pass on to the results.
   * @param string $group
   *   The group to place the result in.
   *
   * @return bool
   *   TRUE on success, FALSE on failure.
   */
  public function assertError($message = '', $group = 'Other') {
    if (!$message) {
      $message = "Error found on current page when error was expected.";
    }
    return $this->assertRaw("<div class=\"messages error\">", $message, $group);
  }

  /**
   * Asserts that no error is found in $this->content.
   *
   * @param string $message
   *   The message to pass on to the results.
   * @param string $group
   *   The group to place the result in.
   *
   * @return bool
   *   TRUE on success, FALSE on failure.
   */
  public function assertNoError($message = '', $group = 'Other') {
    if (!$message) {
      $message = "No error found on current page when no error was expected.";
    }
    return $this->assertNoRaw("<div class=\"messages error\">", $message, $group);
  }

  /**
   * Asserts that a warning is found in $this->content.
   *
   * @param string $message
   *   The message to pass on to the results.
   * @param string $group
   *   The group to place the result in.
   *
   * @return bool
   *   TRUE on success, FALSE on failure.
   */
  public function assertWarning($message = '', $group = 'Other') {
    if (!$message) {
      $message = "Warning found on current page when warning was expected.";
    }
    return $this->assertRaw("<div class=\"messages warning\">", $message, $group);
  }

  /**
   * Asserts that no warning is found in $this->content.
   *
   * @param string $message
   *   The message to pass on to the results.
   * @param string $group
   *   The group to place the result in.
   *
   * @return bool
   *   TRUE on success, FALSE on failure.
   */
  public function assertNoWarning($message = '', $group = 'Other') {
    if (!$message) {
      $message = "No warning found on current page when no warning was expected.";
    }
    return $this->assertNoRaw("<div class=\"messages error\">", $message, $group);
  }

  /**
   * Makes a drupalPost() request, using the form submit button's ID.
   *
   * Because drupalPost() is silly and doesn't let us choose which button we'd
   * like to select if multiple buttons have the same value, this function
   * allows us to select whatever button we'd like based on the ID instead.
   *
   * This is done via the absolutely hilarious method of fudging the actual
   * button labels that don't have the ID we want, so that the only one left
   * with the correct label is the one with the right ID.
   *
   * @see DrupalWebTestCase::drupalPost()
   *
   * @param string $path
   *   Location of the post form.
   * @param array $edit
   *   Field data in an associative array.
   * @param string $submit
   *   Value of the submit button whose click is to be emulated.
   * @param string $id
   *   ID of the submit button whose click is to be emulated.
   * @param array $options
   *   Options to be forwarded to url().
   * @param array $headers
   *   An array containing additional HTTP request headers, each formatted as
   *   "name: value".
   * @param null $form_html_id
   *   (optional) HTML ID of the form to be submitted.
   * @param null $extra_post
   *   (optional) A string of additional data to append to the POST submission.
   *
   * @return bool|string
   *   The content from the POST request's curlExec, or FALSE on fail.
   */
  public function drupalPostByID($path, $edit, $submit, $id, array $options = array(), array $headers = array(), $form_html_id = NULL, $extra_post = NULL) {
    $buttons = $this->xpath("//input[@type=\"submit\" and @value=\"{$submit}\"]");
    if (empty($buttons)) {
      $this->fail("No buttons found on the page with value '$submit'");
      return FALSE;
    }
    $i = 0;
    foreach ($buttons as $button) {
      if ($button['id'] !== $id) {
        $button['value'] .= "_$i";
        $i++;
      }
    }
    return $this->drupalPost($path, $edit, $submit, $options, $headers, $form_html_id, $extra_post);
  }

}
