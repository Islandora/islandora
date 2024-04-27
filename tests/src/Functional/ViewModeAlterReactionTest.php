<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests altering view modes with context ui.
 *
 * @group islandora
 */
class ViewModeAlterReactionTest extends IslandoraFunctionalTestBase {

  /**
   * Node that has node and term entity reference fields.
   *
   * @var \Drupal\node\NodeInterface
   */
  protected $referencer;

  /**
   * Another similar node, to be referenced by referencer.
   *
   * @var \Drupal\node\NodeInterface
   */
  protected $referenced;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    // Node to be referenced via member of.
    $this->referenced = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => $this->testType->id(),
      'title' => 'Referenced',
    ]);
    $this->referenced->save();

    // Node that is member of something.
    $this->referencer = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => $this->testType->id(),
      'title' => 'Referencer',
      'field_member_of' => [$this->referenced->id()],
    ]);
    $this->referencer->save();
  }

  /**
   * @covers \Drupal\islandora\Plugin\ContextReaction\ViewModeAlterReaction::execute
   * @covers \Drupal\islandora\Plugin\ContextReaction\ViewModeAlterReaction::buildConfigurationForm
   * @covers \Drupal\islandora\Plugin\ContextReaction\ViewModeAlterReaction::submitConfigurationForm
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

    // Stash the node's url.
    $url = $this->referencer->toUrl('canonical', ['absolute' => TRUE]);
    $this->drupalGet($url);

    // Make sure we're viewing the default (e.g. the media field is displayed).
    $this->assertSession()->pageTextContains("Member Of");

    // Create a context and set the view mode to alter to "teaser".
    $this->createContext('Test', 'test');

    $this->drupalGet("admin/structure/context/test/reaction/add/view_mode_alter");
    $this->getSession()->getPage()->findById("edit-reactions-view-mode-alter-mode")->selectOption('node.teaser');
    $this->getSession()->getPage()->pressButton('Save and continue');
    $this->assertSession()->statusCodeEquals(200);

    drupal_flush_all_caches();

    // Re-visit the node and make sure we're in teaser mode (e.g. the media
    // field is not displayed).
    $this->drupalGet($url);
    $this->assertSession()->pageTextNotContains("Member Of");
  }

}
