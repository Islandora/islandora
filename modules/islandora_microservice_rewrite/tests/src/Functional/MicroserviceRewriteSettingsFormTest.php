<?php

namespace Drupal\Tests\islandora_microservice_rewrite\Functional;

use Drupal\Tests\islandora\Functional\IslandoraFunctionalTestBase;

/**
 * Smoke tests the microservice rewrite settings page.
 *
 * @group islandora_microservice_rewrite
 */
class MicroserviceRewriteSettingsFormTest extends IslandoraFunctionalTestBase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  protected static $modules = [
    'islandora',
    'islandora_microservice_rewrite',
  ];

  /**
   * Tests that the settings page route loads.
   */
  public function testSettingsPageLoads() {
    $account = $this->drupalCreateUser([
      'access administration pages',
      'administer site configuration',
    ]);
    $this->drupalLogin($account);

    $this->drupalGet('/admin/config/islandora');
    $this->assertSession()->statusCodeEquals(200);
    $this->assertSession()->linkExists('Microservice Rewrite');

    $this->drupalGet('/admin/config/islandora/microservice-rewrite');
    $this->assertSession()->statusCodeEquals(200);
    $this->assertSession()->pageTextContains('Microservice Rewrite');
    $this->assertSession()->pageTextContains('Microservice URL rewrites');
    $this->assertSession()->fieldExists('rewrite_rules');
  }

}
