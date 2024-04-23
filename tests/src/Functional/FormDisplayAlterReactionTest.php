<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests altering form displays with context ui.
 *
 * @group islandora
 */
class FormDisplayAlterReactionTest extends IslandoraFunctionalTestBase {

  /**
   * @covers \Drupal\islandora\Plugin\ContextReaction\FormDisplayAlterReaction::execute
   * @covers \Drupal\islandora\Plugin\ContextReaction\FormDisplayAlterReaction::buildConfigurationForm
   * @covers \Drupal\islandora\Plugin\ContextReaction\FormDisplayAlterReaction::submitConfigurationForm
   */
  public function testViewModeAlter() {

    // Create a test user.
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer contexts',
      'administer actions',
      'view media',
      'create media',
      'update media',
    ]);
    $this->drupalLogin($account);

    // Create a new node referencing the media.
    $this->postNodeAddForm(
      'test_type',
      [
        'title[0][value]' => 'Test Node',
      ],
      'Save'
    );

    // Stash the node's url.
    $url = $this->getUrl();

    // Visit the edit url and make sure we're on the default form display
    // (e.g. there's an autocomplete for the member of field).
    $this->drupalGet($url . "/edit");
    $this->assertSession()->pageTextContains("Member Of");

    // Create a context and set the form mode to alter to "secondary".
    $this->createContext('Test', 'test');

    $this->drupalGet("admin/structure/context/test/reaction/add/form_display_alter");
    $this->getSession()->getPage()->findById("edit-reactions-form-display-alter-mode")->selectOption('node.secondary');
    $this->getSession()->getPage()->pressButton('Save and continue');
    $this->assertSession()->statusCodeEquals(200);

    drupal_flush_all_caches();

    // Re-visit the node and make sure we're in secondar mode (e.g. the media
    // field is not displayed).
    $this->drupalGet($url . "/edit");
    $this->assertSession()->pageTextNotContains("Member Of");

  }

}
