<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests indexing and de-indexing in hooks with pre-configured actions.
 *
 * @group islandora
 */
class DerivativeReactionTest extends IslandoraFunctionalTestBase {

  /**
   * Node to hold the media.
   *
   * @var \Drupal\node\NodeInterface
   */
  protected $node;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $this->node = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => 'test_type',
      'title' => 'Test Node',
    ]);
    $this->node->save();
  }

  /**
   * @covers \Drupal\islandora\IslandoraUtils::executeDerivativeReactions
   */
  public function testExecuteDerivativeReaction() {
    // Create a test user.
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer contexts',
      'administer actions',
      'create media',
      'update media',
    ]);
    $this->drupalLogin($account);

    $this->createContext('Test', 'test');
    $this->addPresetReaction('test', 'derivative', 'hello_world');

    // Create a new media.
    $values = [
      'name[0][value]' => 'Test Media',
      'files[field_media_file_0]' => __DIR__ . '/../../fixtures/test_file.txt',
      'field_media_of[0][target_id]' => 'Test Node',
    ];
    $this->drupalGet('media/add/' . $this->testMediaType->id());
    $this->submitForm($values, 'Save');

    // field_media_of is set and there's a file, so derivatives should fire.
    $this->assertSession()->pageTextContains("Hello World!");

    // Change media but not file, so derivatives should not fire.
    $values = [
      'name[0][value]' => 'Test Media Changed',
    ];
    $this->postEntityEditForm($this->getUrl(), $values, 'Save');
    $media_url = $this->getUrl();
    $this->assertSession()->pageTextNotContains("Hello World!");

    // Change the file, so derivatives should fire again.
    $values = [
      'files[field_media_file_0]' => __DIR__ . '/../../fixtures/test_file2.txt',
    ];
    $this->drupalGet($media_url . '/edit');
    $this->getSession()->getPage()->pressButton('Remove');
    $this->getSession()->getPage()->fillField('files[field_media_file_0]', __DIR__ . '/../../fixtures/test_file2.txt');
    $this->getSession()->getPage()->pressButton('Save');
    $this->assertSession()->pageTextContains("Hello World!");
  }

}
