<?php

namespace Drupal\Tests\islandora_text_extraction\Functional;

use Drupal\Core\Url;
use Drupal\Tests\islandora\Functional\IslandoraFunctionalTestBase;

/**
 * Simple test to ensure that main page loads with module enabled.
 *
 * @group islandora_text_extraction
 */
class LoadTest extends IslandoraFunctionalTestBase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = ['islandora_text_extraction'];

  /**
   * A user with permission to administer site configuration.
   *
   * @var \Drupal\user\UserInterface
   */
  protected $user;

  /**
   * {@inheritdoc}
   */
  public function setUp() {
    parent::setUp();
    $this->user = $this->drupalCreateUser(['administer site configuration']);
    $this->drupalLogin($this->user);
  }

  /**
   * Tests that the home page loads with a 200 response.
   */
  public function testLoad() {
    $this->drupalGet(Url::fromRoute('<front>'));
    $this->assertSession()->statusCodeEquals(200);
  }

}
