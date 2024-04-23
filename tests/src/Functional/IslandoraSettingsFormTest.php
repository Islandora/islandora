<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests Islandora Settings Form.
 *
 * @package Drupal\Tests\islandora\Functional
 * @group islandora
 * @coversDefaultClass \Drupal\islandora\Form\IslandoraSettingsForm
 */
class IslandoraSettingsFormTest extends IslandoraFunctionalTestBase {

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    // Create a test user.
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer site configuration',
      'view media',
      'create media',
      'update media',
    ]);
    $this->drupalLogin($account);
  }

  /**
   * Test form validation for JWT expiry.
   */
  public function testJwtExpiry() {
    $this->drupalGet('/admin/config/islandora/core');
    $this->assertSession()->statusCodeEquals(200);
    $this->assertSession()->pageTextContains("JWT Expiry");
    $this->assertSession()->fieldValueEquals('edit-jwt-expiry', '+2 hour');
    $this->drupalGet('/admin/config/islandora/core');
    // Blank is not allowed.
    $this->submitForm(['edit-jwt-expiry' => ""], 'Save configuration');
    $this->assertSession()->pageTextContainsOnce('"" is not a valid time or interval expression.');
    $this->drupalGet('/admin/config/islandora/core');
    // Negative is not allowed.
    $this->submitForm(['edit-jwt-expiry' => "-2 hours"], 'Save configuration');
    $this->assertSession()->pageTextContainsOnce('Time or interval expression cannot be negative');
    $this->drupalGet('/admin/config/islandora/core');
    // Must include an integer value.
    $this->submitForm(['edit-jwt-expiry' => "last hour"], 'Save configuration');
    $this->assertSession()->pageTextContainsOnce('No numeric interval specified, for example "1 day"');
    $this->drupalGet('/admin/config/islandora/core');
    // Must have an accepted interval.
    $this->submitForm(['edit-jwt-expiry' => "1 fortnight"], 'Save configuration');
    $this->assertSession()->pageTextContainsOnce('No time interval found, please include one of');
    $this->drupalGet('/admin/config/islandora/core');
    // Test a valid setting.
    $this->submitForm(['edit-jwt-expiry' => "2 weeks"], 'Save configuration');
    $this->assertSession()->pageTextContainsOnce('The configuration options have been saved.');

  }

}
