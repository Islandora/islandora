<?php

namespace Drupal\Tests\islandora_video\Functional;

use Drupal\Tests\islandora\Functional\GenerateDerivativeTestBase;

/**
 * Tests the GenerateVideoDerivative action.
 *
 * @group islandora_video
 */
class GenerateVideoDerivativeTest extends GenerateDerivativeTestBase {

  /**
   * {@inheritdoc}
   */
  protected static $modules = ['context_ui', 'islandora_video'];

  /**
   * @covers \Drupal\islandora_video\Plugin\Action\GenerateVideoDerivative::defaultConfiguration
   * @covers \Drupal\islandora_video\Plugin\Action\GenerateVideoDerivative::buildConfigurationForm
   * @covers \Drupal\islandora_video\Plugin\Action\GenerateVideoDerivative::validateConfigurationForm
   */
  public function testGenerateVideoDerivativeFromScratch() {

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

    // Create an action to generate a jpeg thumbnail.
    $this->drupalGet('admin/config/system/actions');
    $this->getSession()->getPage()->findById("edit-action")->selectOption("Generate a video derivative");
    $this->getSession()->getPage()->pressButton(t('Create'));
    $this->assertSession()->statusCodeEquals(200);

    $this->getSession()->getPage()->fillField('edit-label', "Generate video test derivative");
    $this->getSession()->getPage()->fillField('edit-id', "generate_video_test_derivative");
    $this->getSession()->getPage()->fillField('edit-queue', "generate-video-test-derivative");
    $this->getSession()->getPage()->fillField('edit-destination-media-type', $this->testMediaType->label());
    $this->getSession()->getPage()->fillField("edit-source-term", $this->preservationMasterTerm->label());
    $this->getSession()->getPage()->fillField("edit-derivative-term", $this->serviceFileTerm->label());
    $this->getSession()->getPage()->fillField('edit-mimetype', "video/mp4");
    $this->getSession()->getPage()->fillField('edit-args', "-f mp4");
    $this->getSession()->getPage()->fillField('edit-scheme', "public");
    $this->getSession()->getPage()->fillField('edit-path', "derp.mov");
    $this->getSession()->getPage()->pressButton(t('Save'));
    $this->assertSession()->statusCodeEquals(200);

    // Create a context and add the action as a derivative reaction.
    $this->createContext('Test', 'test');
    $this->addPresetReaction('test', 'derivative', "generate_video_test_derivative");
    $this->assertSession()->statusCodeEquals(200);

    // Create a new preservation master belonging to the node.
    $values = [
      'name[0][value]' => 'Test Media',
      'files[field_media_file_0]' => __DIR__ . '/../../fixtures/test_file.txt',
      'field_media_of[0][target_id]' => 'Test Node',
      'field_tags[0][target_id]' => 'Preservation Master',
    ];
    $this->drupalPostForm('media/add/' . $this->testMediaType->id(), $values, t('Save'));

    $expected = [
      'source_uri' => 'test_file.txt',
      'destination_uri' => "node/1/media/{$this->testMediaType->id()}/3",
      'file_upload_uri' => 'public://derp.mov',
      'mimetype' => 'video/mp4',
      'args' => '-f mp4',
      'queue' => 'islandora-connector-homarus',
    ];

    // Check the message gets published and is of the right shape.
    $this->checkMessage($expected);
  }

}
