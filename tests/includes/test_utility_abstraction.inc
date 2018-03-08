<?php

/**
 * @file
 * Abstraction n' stuff for the test utilities and results.
 */

/**
 * A result from a utility method; $type defines TRUE/FALSE as pass/fail.
 */
class IslandoraTestUtilityResult {

  /**
   * The message for this result.
   *
   * @var string
   */
  protected $message;

  /**
   * The caller for this result.
   *
   * @var array
   */
  protected $caller;

  /**
   * The type of result this is - TRUE for pass, FALSE for fail.
   *
   * @var bool
   */
  protected $type;

  /**
   * Constructs an IslandoraTestUtilityResult.
   *
   * @param bool $type
   *   Whether this result should indicate a pass (TRUE) or fail (FALSE).
   * @param string $message
   *   The message that will be used by this result.
   * @param array $caller
   *   The caller for this result.
   */
  public function __construct($type, $message, array $caller) {
    $this->message = $message;
    $this->caller = $caller;
    $this->type = $type;
  }

  /**
   * Get the message for this result.
   *
   * @return string
   *   The message for this result.
   */
  public function getMessage() {
    return $this->message;
  }

  /**
   * Get the caller for this result.
   *
   * @return string
   *   The caller for this result.
   */
  public function getCaller() {
    return $this->caller;
  }

  /**
   * Get the type of result.
   *
   * @return bool
   *   The type of pass (TRUE for pass, FALSE for fail).
   */
  public function getType() {
    return $this->type;
  }

}

/**
 * Abstraction for test utility classes.
 *
 * This is to be implemented in any class that wants to have test utility
 * functionality (i.e. that wants to pass back results to tests). Check out the
 * datastream validator class for a good example of how this is implemented.
 *
 * Test utility classes should store per-installation configuration options in
 * a test_config.ini file, contained in the islandora/tests folder. A function
 * is included with the abstraction to parse the configuration file.
 */
abstract class IslandoraTestUtilityClass {

  /**
   * An array of IslandoraTestUtilityResults.
   *
   * These should be generated using $this->addResult.
   *
   * @var IslandoraTestUtilityResult[]
   */
  public $results = array();

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
  public static function getTestConfiguration() {
    $path = drupal_get_path('module', 'islandora');
    if (file_exists("$path/tests/test_config.ini")) {
      return parse_ini_file("$path/tests/test_config.ini");
    }
    elseif (file_exists("$path/tests/default.test_config.ini")) {
      return parse_ini_file("$path/tests/default.test_config.ini");
    }
    throw new Exception('Required default.test_config.ini/test_config.ini file not found');
  }

  /**
   * Returns an array of IslandoraTestUtilityResults.
   *
   * The particular testing implementation you are using should use this to
   * parse results from utilities and pass them through.
   *
   * @return IslandoraTestUtilityResult[]
   *   The results.
   */
  abstract public function getResults();

  /**
   * Adds a result to $this->results.
   *
   * @param bool $type
   *   The type of result (TRUE for pass, FALSE for fail).
   * @param string $message
   *   The message to put in the result.
   */
  abstract public function addResult($type, $message);

  /**
   * Gets the caller of the method that passed a result.
   *
   * @return array
   *   Array representing the true caller.
   */
  abstract public function getAssertionCall();

}
