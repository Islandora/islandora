<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests the ManageMembersController.
 *
 * @group islandora
 */
class AddChildTest extends IslandoraFunctionalTestBase {

  /**
   * The taxonomy term representing "Collection" items.
   *
   * @var \Drupal\taxonomy\TermInterface
   */
  protected $collectionTerm;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $this->collectionTerm = $this->container->get('entity_type.manager')->getStorage('taxonomy_term')->create([
      'name' => 'Collection',
      'vid' => $this->testVocabulary->id(),
      'field_external_uri' => [['uri' => "http://purl.org/dc/dcmitype/Collection"]],
    ]);
    $this->collectionTerm->save();
  }

  /**
   * @covers \Drupal\islandora\Controller\ManageMembersController::addToNodePage
   * @covers \Drupal\islandora\Controller\ManageMediaController::access
   * @covers \Drupal\islandora\IslandoraUtils::isIslandoraType
   */
  public function testAddChild() {
    $account = $this->drupalCreateUser([
      'bypass node access',
    ]);
    $this->drupalLogin($account);

    $parent = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => 'test_type',
      'title' => 'Parent',
    ]);
    $parent->save();

    // Visit the add member page.
    $this->drupalGet("/node/{$parent->id()}/members/add");

    // Assert that test_type is on the list.
    $this->assertSession()->pageTextContains($this->testType->label());
    $this->clickLink($this->testType->label());
    $url = $this->getUrl();

    // Assert that the link creates the correct prepopulate query param.
    $substring = 'node/add/test_type?edit%5Bfield_member_of%5D%5Bwidget%5D%5B0%5D%5Btarget_id%5D=1';
    $this->assertTrue(
      strpos($url, $substring) !== FALSE,
      "Malformed URL, could not find $substring in $url."
    );
  }

}
