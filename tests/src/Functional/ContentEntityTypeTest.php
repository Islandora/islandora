<?php

namespace Drupal\Tests\islandora\Functional;

use PHPUnit\Framework\Attributes\Group;

/**
 * Tests the ContentEntityType condition.
 */
#[Group('islandora')]
class ContentEntityTypeTest extends IslandoraFunctionalTestBase {

  /**
   * Tests the ContentEntityType condition.
   */
  public function testContentEntityType() {
    // Create a test user.
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer contexts',
      'view media',
      'create media',
      'update media',
    ]);
    $this->drupalLogin($account);

    $this->createContext('Test', 'test');
    $this->addCondition('test', 'content_entity_type');
    $this->getSession()->getPage()->checkField("edit-conditions-content-entity-type-types-node");
    $this->getSession()->getPage()->findById("edit-conditions-content-entity-type-context-mapping-node")->selectOption("@node.node_route_context:node");
    $this->getSession()->getPage()->pressButton('Save and continue');
    $this->addPresetReaction('test', 'index', 'hello_world');

    // Create a new node confirm Hello World! is printed to the screen.
    $this->postNodeAddForm('test_type', ['title[0][value]' => 'Test Node'], 'Save');
    $this->assertSession()->pageTextContains("Hello World!");

    // Add a new media and confirm Hello World! is not printed to the
    // screen.
    $values = [
      'name[0][value]' => 'Test Media',
      'files[field_media_file_0]' => __DIR__ . '/../../fixtures/test_file.txt',
    ];
    $this->drupalGet('media/add/' . $this->testMediaType->id());
    $this->submitForm($values, 'Save');
    $this->assertSession()->pageTextNotContains("Hello World!");
  }

}
