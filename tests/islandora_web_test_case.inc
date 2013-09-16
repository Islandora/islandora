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
   * Asserts that the given datastreams exist correctly on the object.
   *
   * @param AbstractObject $object
   *   The PID of the object
   * @param array $datastreams
   *   An array of strings containing datastream names
   */
  public function assertDatastreams($object, array $datastreams) {
    if (!is_object($object)) {
      $this->fail("Failed. Object passed in is invalid.", 'Islandora');
    }
    else {
      foreach ($datastreams as $datastream) {
        if (isset($object[$datastream])) {
          $this->pass("Loaded datastream {$datastream} from PID {$object->id}.", 'Islandora');
        }
        else {
          $this->fail("Failed to load datastream {$datastream} from PID {$object->id}.", 'Islandora');
        }
      }
    }
  }

  /**
   * Asserts that an object's given datastreams are common-type image files.
   *
   * Uses PHPGD to run the assertion check. This means that only certain kinds
   * of image files can be checked. Please check the documentation for the PHPGD
   * imagecreatefromstring() function to determine what filetypes are valid.
   *
   * @param AbstractObject $object
   *   The PID of the object.
   * @param array $datastreams
   *   An array of datastreams to check.
   */
  public function assertImageDatastreams($object, array $datastreams) {
    if (!is_object($object)) {
      $this->fail("Failed. Object passed in is invalid.", 'Islandora');
    }
    else {
      foreach ($datastreams as $datastream) {
        $datastream_string = $object[$datastream]->content;
        if (!imagecreatefromstring($datastream_string)) {
          $this->fail("Image datastream {$datastream} is either invalid or corrupt.", 'Islandora');
        }
        else {
          $this->pass("Image datastream {$datastream} is valid.", 'Islandora');
        }
      }
    }
  }

  /**
   * Asserts the validity of any .tif/.tiff datastream, which we have a lot of.
   *
   * @param AbstractObject $object
   *   The PID of the object.
   * @param array $datastreams
   *   An array of .tif or .tiff datastreams to check.
   */
  public function assertTiffDatastreams($object, array $datastreams) {
    if (!is_object($object)) {
      $this->fail("Failed. Object passed in is invalid.", 'Islandora');
      return FALSE;
    }
    foreach ($datastreams as $datastream) {
      $datastream_string = $object[$datastream]->content;
      $datastream_header_hex = substr(bin2hex($datastream_string), 0, 8);
      if ($datastream_header_hex == "49492a00") {
        // In this case, the ingested TIFF is designated as using the "Intel
        // byte-order" (e.g. little-endian) by starting with the characters "II"
        // (repeated so that byte order does not yet need to be significant).
        // The number that follows is '42' in little-endian hex, a number of
        // 'deep philosophical significance' to the TIFF format creators.
        $this->pass(t("{$datastream} datastream asserts that it is a valid Intel-byte-orderded TIF/TIFF file."), 'Islandora');
      }
      else if ($datastream_header_hex == "4d4d002a") {
        // In this case, the ingested TIFF is designated as using the "Motorola
        // byte-order" (e.g. big-endian) by starting with the characters "MM"
        // instead. 42 follows once again, this time in big-endian hex.
        $this->pass(t("{$datastream} datastream asserts that it is a valid Motorola-byte-ordered TIF/TIFF file."), 'Islandora');
      }
      else {
        $this->fail(t("{$datastream} datastream does not assert that it is a valid TIF/TIFF file."), 'Islandora');
      }
    }
  }

  /**
   * Asserts the validity of any .jp2 datastreams, which we also have a lot of.
   *
   * @param AbstractObject $object
   *   The PID of the object.
   * @param array $datastreams
   *   An array of .jp2 datastreams to check.
   */
  public function assertJp2Datastreams($object, array $datastreams) {
    if (!is_object($object)) {
      $this->fail("Failed. Object passed in is invalid.", 'Islandora');
      return FALSE;
    }
    foreach ($datastreams as $datastream) {
      $datastream_hex = bin2hex($object[$datastream]->content);
      // .jp2 files begin with an offset header at the second 32-bit integer,
      // 0x6A502020. This header is in all .jp2s, and we check for it here.
      $this->assertTrue(substr($datastream_hex, 8, 8) == '6a502020', t("{$datastream} datastream begins correctly with the appropriate .jp2 header."), 'Islandora');
      // .jp2 files have their codestream capped with a marker, 0xFFD9. We're
      // just checking for it here to see if the .jp2 encoder finished okay.
      $this->assertTrue(substr($datastream_hex, strlen($datastream_hex) - 4, 4) == 'ffd9', t("{$datastream} datastream ends correctly with the appropriate .jp2 marker."), 'Islandora');
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
   */
  public function deleteObject($pid, $button = 'Delete') {
    $path = 'islandora/object/' . $pid . '/manage/properties';
    $edit = array();
    $this->drupalPost($path, $edit, $button);
    $this->drupalPost($this->url, $edit, t('Delete'));
    $object = islandora_object_load($pid);

    $this->drupalGet("islandora/object/$pid");
    $this->assertResponse(404, "Object $pid successfully deleted.");
  }

  /**
   * Reverses a hex string and converts it to a little-endian-formatted integer.
   *
   * This is useful for running checks on values that appear in the binary
   * of a datastream. Returns FALSE if the hex value contains non-hex characters
   * or if the string would not return a 16- or 32-bit formatted big-endian
   * signed integer.
   *
   * @param string $hex
   *   The hex value being converted.
   *
   * @return bool|int
   *   FALSE or the integer value that is converted.
   */
  public function convertHexToInt($hex) {

    // A couple of quick string checks.
    if (!ctype_xdigit($hex)) {
      $this->fail('String passed to convertHexToInt() contains non-hexidecimal characters.', 'PHP');
      return FALSE;
    }
    if (!strlen($hex) === 4 || !strlen($hex) === 8) {
      $this->fail('String passed to convertHexToInt() cannot create a 16- or 32-bit little-endian signed integer', 'PHP');
      return FALSE;
    }

    // The actual conversion.
    $reverse_hex = implode('', array_reverse(str_split($hex, 2)));
    $int = hexdec($reverse_hex);
    return $int;
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
   *   'models' - An array that can contain multiple content model PIDs.
   *   'owner' - The object's owner.
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
    module_load_include('inc', 'islandora', 'includes/tuque');
    $tuque = new IslandoraTuque();
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

    if (isset($properties['models']) && is_array($properties['models'])) {
      foreach ($properties['models'] as $model) {
        $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', $model);
      }
    }
    elseif (isset($properties['models']) && !is_array($properties['models'])) {
      $this->fail(t("'models' key of properties variable is not an array. Content model(s) will not be set."), 'Islandora');
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

}
