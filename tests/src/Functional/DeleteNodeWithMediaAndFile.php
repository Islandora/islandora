<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests the Delete Node with Media.
 *
 * @group islandora
 */
class DeleteNodeWithMediaAndFile extends IslandoraFunctionalTestBase {

  /**
   * Tests delete Node and its assoicated media.
   */
  public function testDeleteNodeWithMediaAndFile() {
    $account = $this->drupalCreateUser([
      'delete any media',
      'create media',
      'view media',
      'bypass node access',
      'access files overview',
      'administer site configuration',
    ]);
    $this->drupalLogin($account);

    $assert_session = $this->assertSession();

    $testImageMediaType = $this->createMediaType('image', ['id' => 'test_image_media_type']);
    $testImageMediaType->save();

    $this->createEntityReferenceField('media', $testImageMediaType->id(), 'field_media_of', 'Media Of', 'node', 'default', [], 2);

    $node = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => 'test_type',
      'title' => 'node',
    ]);
    $node->save();

    // Make an image for the Media.
    $file = $this->container->get('entity_type.manager')->getStorage('file')->create([
      'uid' => $account->id(),
      'uri' => "public://test.jpeg",
      'filename' => "test.jpeg",
      'filemime' => "image/jpeg",
      'status' => FILE_STATUS_PERMANENT,
    ]);
    $file->save();

    $this->drupalGet("node/1/delete");
    $assert_session->pageTextNotContains('Delete all associated medias and nodes');

    // Make the media, and associate it with the image and node.
    $media1 = $this->container->get('entity_type.manager')->getStorage('media')->create([
      'bundle' => $testImageMediaType->id(),
      'name' => 'Media1',
      'field_media_image' =>
        [
          'target_id' => $file->id(),
          'alt' => 'Some Alt',
          'title' => 'Some Title',
        ],
      'field_media_of' => ['target_id' => $node->id()],
    ]);
    $media1->save();

    $media2 = $this->container->get('entity_type.manager')->getStorage('media')->create([
      'bundle' => $testImageMediaType->id(),
      'name' => 'Media2',
      'field_media_image' =>
        [
          'target_id' => $file->id(),
          'alt' => 'Some Alt',
          'title' => 'Some Title',
        ],
      'field_media_of' => ['target_id' => $node->id()],
    ]);
    $media2->save();

    $this->drupalGet("admin/config/islandora/core");
    $assert_session->pageTextContains('Node Delete with Media and Files');
    \Drupal::configFactory()->getEditable('islandora.settings')->set('delete_media_and_files', TRUE)->save();

    $delete = ['delete_associated_content' => TRUE];

    $this->drupalGet("node/1/delete");
    $assert_session->pageTextContains('Media1');
    $assert_session->pageTextContains('Media2');
    $this->submitForm($delete, 'Delete');

    $assert_session->pageTextContains($media1->id());
    $assert_session->pageTextContains($media2->id());

    $this->drupalGet("media/1/delete");
    $assert_session->pageTextContains('Page not found');

    $this->drupalGet("media/2/delete");
    $assert_session->pageTextContains('Page not found');

    $this->drupalGet("/admin/content/files");
    $assert_session->pageTextNotContains('test.jpeg');

  }

}
