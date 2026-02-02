<?php

namespace Drupal\Tests\islandora\Functional;

use PHPUnit\Framework\Attributes\Group;

/**
 * Tests the ManageMembersController.
 */
#[Group('islandora')]
class AddMediaTest extends IslandoraFunctionalTestBase {

  /**
   * Tests adding media to a node.
   */
  public function testAddMedia() {
    $account = $this->drupalCreateUser([
      'bypass node access',
      'create media',
    ]);
    $this->drupalLogin($account);

    $parent = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => 'test_type',
      'title' => 'Parent',
    ]);
    $parent->save();

    // Visit the add media page.
    $this->drupalGet("/node/{$parent->id()}/media/add");

    // Assert that test_meida_type is on the list.
    $this->assertSession()->pageTextContains($this->testMediaType->label());
    $this->clickLink($this->testMediaType->label());
    $url = $this->getUrl();

    // Assert that the link creates the correct prepopulate query param.
    $substring = 'media/add/test_media_type?edit%5Bfield_media_of%5D%5Bwidget%5D%5B0%5D%5Btarget_id%5D=1';
    $this->assertTrue(
      strpos($url, $substring) !== FALSE,
      "Malformed URL, could not find $substring in $url."
    );
  }

}
