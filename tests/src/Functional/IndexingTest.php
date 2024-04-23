<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests indexing and de-indexing in hooks with pre-configured actions.
 *
 * @group islandora
 */
class IndexingTest extends IslandoraFunctionalTestBase {

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    // Create an action that dsm's "Goodbye, Cruel World!".
    $goodbye_world = $this->container->get('entity_type.manager')->getStorage('action')->create([
      'id' => 'goodbye_world',
      'label' => 'Goodbye World',
      'type' => 'system',
      'plugin' => 'action_message_action',
      'configuration' => [
        'message' => 'Goodbye, Cruel World!',
      ],
    ]);
    $goodbye_world->save();
  }

  /**
   * @covers \Drupal\islandora\ContextProvider\NodeContextProvider::__construct
   * @covers \Drupal\islandora\ContextProvider\NodeContextProvider::getRuntimeContexts
   * @covers \Drupal\islandora\IslandoraContextManager::evaluateContexts
   * @covers \Drupal\islandora\IslandoraContextManager::applyContexts
   * @covers \Drupal\islandora\PresetReaction\PresetReaction::buildConfigurationForm
   * @covers \Drupal\islandora\PresetReaction\PresetReaction::submitConfigurationForm
   * @covers \Drupal\islandora\PresetReaction\PresetReaction::execute
   * @covers \Drupal\islandora\IslandoraServiceProvider::alter
   */
  public function testIndexing() {
    // Create a test user.
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer contexts',
      'administer actions',
    ]);
    $this->drupalLogin($account);

    $this->createContext('Test', 'test');
    $this->addPresetReaction('test', 'index', 'hello_world');

    // Create a new node and confirm Hello World! is printed to the screen.
    $this->postNodeAddForm('test_type', ['title[0][value]' => 'Test Node'], 'Save');
    $this->assertSession()->pageTextContains("Hello World!");

    // Stash the node's url.
    $url = $this->getUrl();

    // Edit the node and confirm Hello World! is printed to the screen.
    $this->postEntityEditForm($url, ['title[0][value]' => 'Test Node Changed'], 'Save');
    $this->assertSession()->pageTextContains("Hello World!");

    // Add the Goodbye World reaction.
    $this->addPresetReaction('test', 'delete', 'goodbye_world');
    $this->drupalGet("$url/delete");

    // Delete the node.
    $this->submitForm([], 'Delete');
    $this->assertSession()->statusCodeEquals(200);

    // Confirm Goodbye, Cruel World! is printed to the screen.
    $this->assertSession()->pageTextContains("Goodbye, Cruel World!");
  }

}
