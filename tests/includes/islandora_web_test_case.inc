<?php

/**
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
   *
   * @var bool
   */
  protected $deleteObjectsOnTeardown = TRUE;

  /**
   * Instantiates an IslandoraWebTestCase with a configuration.
   *
   * @throws Exception
   *   If the required test config file is not found.
   */
  public function __construct($test_id = NULL) {
    $this->configuration = IslandoraTestUtilityClass::getTestConfiguration();
    parent::__construct($test_id);
  }

  /**
   * Defers to IslandoraTestUtilities for missing methods.
   *
   * @param string $method
   *   The method being called.
   * @param array $args
   *   The arguments for that method.
   *
   * @return bool
   *   TRUE if the result was a pass, or FALSE otherwise.
   */
  public function __call($method, array $args) {
    module_load_include('inc', 'islandora', 'tests/includes/utilities');
    $params = array(
      'logged_in_user' => $this->loggedInUser,
      'db_access' => TRUE,
    );
    $utilities = new IslandoraTestUtilities($this->configuration, $params);
    if (!method_exists($utilities, $method)) {
      $caller = $this->getAssertionCall();
      throw new BadMethodCallException("Exception: undefined method $method in {$caller['file']}, line {$caller['line']}.");
    }
    $result = call_user_func_array(array(&$utilities, $method), $args);
    $this->parseUtilityResults($utilities);
    return $result;
  }

  /**
   * Parses utility results and passes them to the test results as an assertion.
   *
   * @param IslandoraTestUtilities $utility
   *   An instance of IslandoraTestUtilities with populated results.
   */
  public function parseUtilityResults(IslandoraTestUtilities $utility) {
    foreach ($utility->getResults() as $result) {
      $this->assert($result->getType(), $result->getMessage(), 'Islandora', $result->getCaller());
    }
  }

  /**
   * Run all tests in this class.
   *
   * Attempts to figure out if the Drupal filter is writable before running any
   * tests.
   *
   * @see DrupalWebTestCase::run()
   */
  public function run(array $methods = array()) {
    // Determine if the Drupal filter is writable so we know if we can proceed.
    if (is_writable($this->configuration['drupal_filter_file'])) {
      // Set up the SimpleTest environment.
      simpletest_verbose(NULL, variable_get('file_public_path', conf_path() . '/files'), get_class($this));
      $this->httpauth_method = variable_get('simpletest_httpauth_method', CURLAUTH_BASIC);
      $username = variable_get('simpletest_httpauth_username', NULL);
      $password = variable_get('simpletest_httpauth_password', NULL);
      if ($username && $password) {
        $this->httpauth_credentials = $username . ':' . $password;
      }
      set_error_handler(array($this, 'errorHandler'));
      $class = get_class($this);
      // Iterate through all the methods in this class, unless a specific list
      // of methods to run was passed.
      $class_methods = get_class_methods($class);
      if ($methods) {
        $class_methods = array_intersect($class_methods, $methods);
      }
      foreach ($class_methods as $method) {
        // If the current method starts with "test", run it - it's a test.
        if (strtolower(substr($method, 0, 4)) == 'test') {
          // Insert a fail record. This will be deleted on completion to ensure
          // that testing completed.
          $method_info = new ReflectionMethod($class, $method);
          $caller = array(
            'file' => $method_info->getFileName(),
            'line' => $method_info->getStartLine(),
            'function' => $class . '->' . $method . '()',
          );
          $completion_check_id = DrupalTestCase::insertAssert($this->testId, $class, FALSE, t('The test did not complete due to a fatal error.'), 'Completion check', $caller);
          try {
            $this->setUp();
            $this->$method();
          }
          catch (Exception $e) {
            $this->exceptionHandler($e);
          }
          $this->tearDown();
          // Remove the completion check record.
          DrupalTestCase::deleteAssert($completion_check_id);
        }
      }
    }
    // If the Drupal filter is not writable, skip testing and error out.
    else {
      $method_info = new ReflectionMethod($this, 'run');
      $class = get_class($this);
      set_error_handler(array($this, 'errorHandler'));
      $caller = array(
        'file' => $method_info->getFileName(),
        'line' => $method_info->getStartLine(),
        'function' => $class . '->run()',
      );
      $this->assert(FALSE, "Unable to proceed; the Drupal filter is not writable by the server.", "Completion check", $caller);
    }
    drupal_get_messages();
    restore_error_handler();
  }

  /**
   * Sets up the web test case.
   *
   * @see DrupalWebTestCase::setUp()
   */
  public function setUp() {
    $args = func_get_args();
    $args = (isset($args[0]) && is_array($args[0])) ? $args[0] : $args;
    // Always enable islandora.
    $args[] = 'islandora';
    parent::setUp($args);

    // It's possible test are running before class autoloading.
    module_load_include('inc', 'islandora', 'includes/tuque');
    module_load_include('inc', 'islandora', 'includes/tuque_wrapper');
    module_load_include('inc', 'islandora', 'tests/includes/utilities');

    if ($this->configuration['use_drupal_filter']) {
      $this->setUpDrupalFilter();
    }
    $this->admin = $this->createAdminUser();
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
      islandora_repair_drupal_filter();
    }
    unset($this->admin);
    parent::tearDown();
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
   *   The PID of the collection to be deleted.
   * @param string $button
   *   The label of the first 'Delete' button.
   * @param bool $safety
   *   If TRUE, this will only delete objects owned by users in $this->users.
   *
   * @return bool
   *   If the deletion fails, return FALSE.
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

  // These are a few quick helper functions to fill in a gap in the standard
  // DrupalWebTestCase where no function exists to test for the simple existence
  // or non-existence of an error.
  // @codingStandardsIgnoreLine

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
    return $this->assertFieldByXPath('//div[contains(@class, "message") and contains(@class, "error")]', NULL, $message, $group);
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
    return $this->assertNoFieldByXPath('//div[contains(@class, "message") and contains(@class, "error")]', NULL, $message, $group);
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
    return $this->assertFieldByXPath('//div[contains(@class, "message") and contains(@class, "warning")]', NULL, $message, $group);
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
    return $this->assertNoFieldByXPath('//div[contains(@class, "message") and contains(@class, "warning")]', NULL, $message, $group);
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
   * @param mixed $form_html_id
   *   (optional) HTML ID of the form to be submitted.
   * @param mixed $extra_post
   *   (optional) A string of additional data to append to the POST submission.
   *
   * @return bool|string
   *   The content from the POST request's curlExec, or FALSE on fail.
   *
   * @see DrupalWebTestCase::drupalPost()
   */
  public function drupalPostById($path, array $edit, $submit, $id, array $options = array(), array $headers = array(), $form_html_id = NULL, $extra_post = NULL) {
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
