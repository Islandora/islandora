<?php

/**
 * Islandora extensions for DrupalUnitTestCase.
 */
class IslandoraUnitTestCase extends DrupalUnitTestCase {

  /**
   * By default, deleteUserCreatedObjects() runs on each tearDown() step.
   *
   * @var bool
   */
  protected $deleteObjectsOnTeardown = TRUE;

  /**
   * Defers to IslandoraTestUtilities for missing methods.
   *
   * @param string $method
   *   The method being called.
   * @param array $args
   *   The arguments for that method.
   */
  public function __call($method, array $args) {
    module_load_include('inc', 'islandora', 'tests/includes/utilities');
    $params = array('db_access' => FALSE);
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
   * Sets up the Drupal filter to access this test Drupal instances database.
   *
   * @see DrupalWebTestCase::setUp()
   */
  public function setUp() {
    parent::setUp();

    // It's possible test are running before class autoloading.
    module_load_include('inc', 'islandora', 'includes/tuque');

    $this->configuration = IslandoraTestUtilityClass::getTestConfiguration();
    $this->connection = new RepositoryConnection($this->configuration['fedora_url'], $this->configuration['admin_user'], $this->configuration['admin_pass']);
    $api = new FedoraApi($this->connection);
    $this->repository = new FedoraRepository($api, new SimpleCache());
  }

  /**
   * Frees any allocated resources.
   *
   * @see DrupalWebTestCase::tearDown()
   */
  public function tearDown() {
    unset($this->configuration);
    parent::tearDown();
  }

}
