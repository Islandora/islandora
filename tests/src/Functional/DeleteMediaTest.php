<?php

namespace Drupal\Tests\islandora\Functional;

use Drupal\views\Views;

/**
 * Tests the DeleteMedia and DeleteMediaAndFile actions.
 *
 * @group islandora
 */
class DeleteMediaTest extends IslandoraFunctionalTestBase {

  /**
   * Modules to be enabled.
   *
   * @var array
   */
  protected static $modules = [
    'media_test_views',
    'context_ui',
    'field_ui',
    'islandora',
  ];

  /**
   * Media.
   *
   * @var \Drupal\media\MediaInterface
   */
  protected $media;

  /**
   * File to belong to the media.
   *
   * @var \Drupal\file\FileInterface
   */
  protected $file;

  /**
   * User account.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $account;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $permissions = ['create media', 'delete any media', 'delete any file'];

    // Create a test user.
    $this->account = $this->createUser($permissions);

    list($this->file, $this->media) = $this->makeMediaAndFile($this->account);
  }

  /**
   * Tests the delete_media_and_file action.
   *
   * @covers \Drupal\islandora\Plugin\Action\DeleteMediaAndFile::execute
   */
  public function testDeleteMediaAndFile() {
    $this->drupalLogin($this->account);
    $session = $this->getSession();
    $page = $session->getPage();

    $mid = $this->media->id();
    $fid = $this->file->id();

    // Ensure the media is in the test view.
    $view = Views::getView('test_media_bulk_form');
    $view->execute();
    $this->assertSame($view->total_rows, 1);

    $this->drupalGet('test-media-bulk-form');

    // Check that the option exists.
    $this->assertSession()->optionExists('action', 'delete_media_and_file');

    // Run the bulk action.
    $page->checkField('media_bulk_form[0]');
    $page->selectFieldOption('action', 'delete_media_and_file');
    $page->pressButton('Apply to selected items');
    $this->assertSession()->pageTextContains('Are you sure you want to delete this media and associated files?');
    $page->pressButton('Delete');
    // Should assert that a media and file were deleted.
    $this->assertSession()->pageTextContains("The media with the id $mid has been deleted");

    // Attempt to reload the entities.
    // Both media and file should be gone.
    $this->assertTrue(
      !$this->container->get('entity_type.manager')->getStorage('media')->load($mid),
      "Media must be deleted after running action"
    );
    $this->assertTrue(
      !$this->container->get('entity_type.manager')->getStorage('file')->load($fid),
      "File must be deleted after running action"
    );
  }

}
