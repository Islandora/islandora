<?php


/**
 * @file
 * Generic test case to be extended to implement Islandora testing.
 *
 * In order to use this properly, you must place a test user in
 * your Fedora installation.  Please add the follwing snippet to your
 * $FEDORA_HOME/server/config/fedora-users.xml:
 *
 *    <user name="simpletestuser" password="41fe63c9636c6649f0a4747400f0f95e">
 *      <attribute name="fedoraRole">
 *        <value>administrator</value>
 *      </attribute>
 *    </user>
 */
abstract class IslandoraTestCase extends DrupalWebTestCase {

  /**
   * User with the rights required to perform the test
   */
  protected $privileged_user;

  /**
   * Override this method to provide the name of the module,
   * e.g. the name of the .module file 
   *
   * @return string - The name of the module
   */
  abstract protected function getModuleName();

  /**
   * Returns basic permissions needed for running Islandora tests.
   * Override this method to provide additional permissions, but 
   * be sure to append your new permissions to the return value
   * of parent::getUserPermissions and return that.
   *
   * @return array - An array of strings describing permissions
   */
  protected function getUserPermissions() {
    return array(
      'access content',
      'add fedora datastreams',
      'edit fedora meta data',
      'edit tags datastream',
      'ingest new fedora objects',
      'purge objects and datastreams',
      'view fedora collection',
      'view detailed list of content',
    );
  }

  /**
   * Creates a user with permissions required for the test, and logs in.
   */
  protected function createPrivilegedUser() {
    // Create a role with the permissions from getUserPermissions()
    $role = $this->drupalCreateRole($this->getUserPermissions());
  
    // Create a user model 
    $user = array(
      'name' => 'simpletestuser',
      'mail' => 'simpletestuser@example.com',
      'roles' => array( $role => $role ),
      'pass' => 'simpletestpass',
      'status' => 1,
    );

    // Create a user from the model
    $this->privileged_user = user_save('', $user);

    // Add the raw password so we can log in
    $this->privileged_user->pass_raw = $user['pass'];

    // Log in
    !$this->drupalLogin($this->privileged_user); 
  }

  /**
   * Automatically generates all module dependencies and enables them in order.
   * Also logs in a privileged user with the appropriate permissions for the
   * test.
   *
   * If you need to override this method to provide extra functionality,
   * please be sure to call parent::setUp so dependency resolution and 
   * authentication still happen.
   */
  public function setUp() {    
    // Grab all the available modules
    $modules = module_rebuild_cache();

    // Grab the module to test's dependencies
    $dependencies = $modules[$this->getModuleName()]->info['dependencies'];

    // Sort them so they're in the correct order to enable
    $dependencies = array_reverse($dependencies);

    // Add our module to the end
    $dependencies[] = $this->getModuleName();
    
    // Enable the module's dependencies in order
    call_user_func_array('parent::setUp', $dependencies);

    // Authenticate the privileged user
    $this->createPrivilegedUser();
  }

  public function invoke($hook_name) {
    $args = func_get_args();
    array_unshift($args, $this->getModuleName());
    return call_user_func_array('module_invoke', $args);
  }
}

/**
 * Class containing tests that are common to all solution packs.
 */
abstract class SolutionPackTestCase extends IslandoraTestCase {
 
  /**
   *  Tests the fedora_repository_required_fedora_objects hook.
   *
   *  Validates the schema of the array returned by the hook.  The array should
   *  be roughly of the form:
   *    [
   *      module_name => 
   *      [
   *        'module' => module_name,
   *        'title' => module_title,
   *        'objects' =>
   *        [
   *          ...
   *        ]
   *      ]
   *    ]
   *
   *  Where each entry of the 'objects' sub-array should be arrays themselves,
   *  each containing the information needed for drupal_get_form().
   *
   *  Note that the root key of the array is the actual module name (e.g. 
   *  islandora_audio_sp, islandora_image_sp, etc...), not 'module_name'
   */
  public function testFedoraRepositoryRequiredFedoraObjects() {
    
    // Invoke the hook
    $results = $this->invoke('fedora_repository_required_fedora_objects');

    // Validate the schema of the returned data structure
    $this->assertNotNull($results, 'A non-null result was returned from the fedora_repository_required_fedora_objects hook.');
    $this->assertTrue(is_array($results), 'An array was returned from the fedora_repository_required_fedora_objects hook.');
    $this->assertTrue(array_key_exists($this->getModuleName(), $results), "Returned array has top level key that is equal to the module's short form name");
    $this->assertTrue(is_array($results[$this->getModuleName()]), "Value associated with top level key that is equal to the module's short form name is an array");
    $this->assertTrue(array_key_exists('module', $results[$this->getModuleName()]), "Top level array has 'module' key");
    $this->assertEqual($results[$this->getModuleName()]['module'], $this->getModuleName(), "Value associated with 'module' key is equal to the module's short form name");
    $this->assertTrue(array_key_exists('title', $results[$this->getModuleName()]), "Top level array has 'title' key");
    $this->assertTrue($results[$this->getModuleName()]['title'], "Title string is not empty");
    $this->assertTrue(array_key_exists('objects', $results[$this->getModuleName()]), "Returned array has second level key equal to 'object'");
    $this->assertTrue(is_array($results[$this->getModuleName()]['objects']), "Value associated with second level 'object' key is an array");
  }
}
