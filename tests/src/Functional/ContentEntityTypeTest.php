<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests the ContentEntityType condition.
 *
 * @group islandora
 */
class ContentEntityTypeTest extends IslandoraFunctionalTestBase {

  /**
   * @covers \Drupal\islandora\ContextProvider\NodeContextProvider::__construct
   * @covers \Drupal\islandora\ContextProvider\NodeContextProvider::getRuntimeContexts
   * @covers \Drupal\islandora\ContextProvider\MediaContextProvider::__construct
   * @covers \Drupal\islandora\ContextProvider\MediaContextProvider::getRuntimeContexts
   * @covers \Drupal\islandora\IslandoraContextManager::evaluateContexts
   * @covers \Drupal\islandora\IslandoraContextManager::applyContexts
   * @covers \Drupal\islandora\Plugin\Condition\ContentEntityType::buildConfigurationForm
   * @covers \Drupal\islandora\Plugin\Condition\ContentEntityType::submitConfigurationForm
   * @covers \Drupal\islandora\Plugin\Condition\ContentEntityType::evaluate
   * @covers \Drupal\islandora\PresetReaction\PresetReaction::buildConfigurationForm
   * @covers \Drupal\islandora\PresetReaction\PresetReaction::submitConfigurationForm
   * @covers \Drupal\islandora\PresetReaction\PresetReaction::execute
   * @covers \Drupal\islandora\IslandoraServiceProvider::alter
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
