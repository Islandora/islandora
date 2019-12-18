<?php

namespace Drupal\Tests\islandora_image\Functional;

use Drupal\Tests\islandora\Functional\GenerateDerivativeTestBase;

/**
 * Tests the GenerateImageDerivative action.
 *
 * @group islandora_image
 */
class GenerateImageDerivativeTest extends GenerateDerivativeTestBase {

  /**
   * {@inheritdoc}
   */
  protected static $modules = ['context_ui', 'islandora_image'];

  /**
   * @covers \Drupal\islandora_image\Plugin\Action\GenerateImageDerivative::defaultConfiguration
   * @covers \Drupal\islandora_image\Plugin\Action\GenerateImageDerivative::buildConfigurationForm
   * @covers \Drupal\islandora_image\Plugin\Action\GenerateImageDerivative::validateConfigurationForm
   * @covers \Drupal\islandora\Plugin\Action\AbstractGenerateDerivative::defaultConfiguration
   * @covers \Drupal\islandora\Plugin\Action\AbstractGenerateDerivative::buildConfigurationForm
   * @covers \Drupal\islandora\Plugin\Action\AbstractGenerateDerivative::validateConfigurationForm
   * @covers \Drupal\islandora\Plugin\Action\AbstractGenerateDerivative::submitConfigurationForm
   * @covers \Drupal\islandora\Plugin\Action\AbstractGenerateDerivative::execute
   */
  public function testGenerateImageDerivativeFromScratch() {

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
    $this->getSession()->getPage()->findById("edit-action")->selectOption("Generate an image derivative");
    $this->getSession()->getPage()->pressButton(t('Create'));
    $this->assertSession()->statusCodeEquals(200);

    $this->getSession()->getPage()->fillField('edit-label', "Generate image test derivative");
    $this->getSession()->getPage()->fillField('edit-id', "generate_image_test_derivative");
    $this->getSession()->getPage()->fillField('edit-queue', "generate-image-test-derivative");
    $this->getSession()->getPage()->fillField('edit-destination-media-type', $this->testMediaType->label());
    $this->getSession()->getPage()->fillField("edit-source-term", $this->preservationMasterTerm->label());
    $this->getSession()->getPage()->fillField("edit-derivative-term", $this->serviceFileTerm->label());
    $this->getSession()->getPage()->fillField('edit-mimetype', "image/jpeg");
    $this->getSession()->getPage()->fillField('edit-args', "-thumbnail 20x20");
    $this->getSession()->getPage()->fillField('edit-scheme', "public");
    $this->getSession()->getPage()->fillField('edit-path', "derp.jpeg");
    $this->getSession()->getPage()->pressButton(t('Save'));
    $this->assertSession()->statusCodeEquals(200);

    // Create a context and add the action as a derivative reaction.
    $this->createContext('Test', 'test');
    $this->addPresetReaction('test', 'derivative', "generate_image_test_derivative");
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
      'file_upload_uri' => 'public://derp.jpeg',
      'mimetype' => 'image/jpeg',
      'args' => '-thumbnail 20x20',
      'queue' => 'islandora-connector-houdini',
    ];

    // Check the message gets published and is of the right shape.
    $this->checkMessage($expected);
  }

}
