<?php

namespace Drupal\Tests\islandora\Functional;

use Drupal\islandora\Plugin\Field\FieldFormatter\IslandoraImageFormatter;
use PHPUnit\Framework\Attributes\CoversClass;
use PHPUnit\Framework\Attributes\Group;

/**
 * Tests the links for image fields with the islandora_image field formatter.
 */
#[Group('islandora')]
#[CoversClass(IslandoraImageFormatter::class)]
class IslandoraImageFormatterTest extends IslandoraFunctionalTestBase {

  /**
   * Tests the Islandora image formatter.
   */
  public function testIslandoraImageFormatter() {

    // Log in.
    $account = $this->drupalCreateUser([
      'bypass node access',
      'view media',
      'create media',
    ]);
    $this->drupalLogin($account);

    // Create an image media type.
    $testImageMediaType = $this->createMediaType('image', ['id' => 'test_image_media_type']);
    $testImageMediaType->save();
    $this->createEntityReferenceField('media', $testImageMediaType->id(), 'field_media_of', 'Media Of', 'node', 'default', [], 2);
    // Set the display mode to use the islandora_image formatter.
    // Also, only show the image on display to remove clutter.
    $display_options = [
      'type' => 'islandora_image',
      'settings' => [
        'image_style' => '',
        'image_link' => 'content',
        'image_loading' => [
          'attribute' => 'eager',
        ],
      ],
    ];

    $display = $this->container->get('entity_display.repository')->getViewDisplay('media', $testImageMediaType->id(), 'default');
    $display->setComponent('field_media_image', $display_options)
      ->removeComponent('created')
      ->removeComponent('uid')
      ->removeComponent('thumbnail')
      ->save();

    // Make a node.
    $node = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => $this->testType->id(),
      'title' => 'Test Node',
    ]);
    $node->save();
    // Make a image for the Media.
    $file = $this->container->get('entity_type.manager')->getStorage('file')->create([
      'uid' => $account->id(),
      'uri' => "public://test.jpeg",
      'filename' => "test.jpeg",
      'filemime' => "image/jpeg",
    ]);
    $file->setPermanent();
    $file->save();

    // Make the media, and associate it with the image and node.
    $media = $this->container->get('entity_type.manager')->getStorage('media')->create([
      'bundle' => $testImageMediaType->id(),
      'name' => 'Media',
      'field_media_image' =>
        [
          'target_id' => $file->id(),
          'alt' => 'Some Alt',
          'title' => 'Some Title',
        ],
      'field_media_of' => ['target_id' => $node->id()],
    ]);
    $media->save();

    // View the media.
    $this->drupalGet("media/1");

    // Assert that the image is rendered into html as a link pointing
    // to the Node, not the Media (that's what the islandora_image
    // formatter does).
    $elements = $this->xpath(
      '//a[@href=:path]/img[@src=:url and @alt=:alt and @title=:title]',
      [
        ':path' => $node->toUrl()->toString(),
        ':url' => $file->createFileUrl(),
        ':alt' => 'Some Alt',
        ':title' => 'Some Title',
      ]
    );
    $this->assertEquals(count($elements), 1, 'Image linked to content formatter displaying points to Node and not Media.');
  }

}
