<?php

namespace Drupal\Tests\islandora_microservice_rewrite\Kernel;

use Drupal\node\Entity\Node;
use Drupal\node\Entity\NodeType;
use Drupal\Tests\islandora\Kernel\IslandoraKernelTestBase;
use Drupal\Tests\user\Traits\UserCreationTrait;

/**
 * Tests submodule-owned microservice URL rewrites.
 *
 * @group islandora_microservice_rewrite
 */
class MicroserviceRewriteSubscriberTest extends IslandoraKernelTestBase {

  use UserCreationTrait;

  /**
   * {@inheritdoc}
   */
  protected static $modules = [
    'islandora_microservice_rewrite',
  ];

  /**
   * The test user.
   *
   * @var \Drupal\user\UserInterface
   */
  protected $user;

  /**
   * The test entity.
   *
   * @var \Drupal\node\NodeInterface
   */
  protected $entity;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $this->installConfig('islandora_microservice_rewrite');

    $this->user = $this->createUser(['administer nodes']);

    $type = NodeType::create([
      'type' => 'test_type',
      'name' => 'Test Type',
    ]);
    $type->save();

    $this->entity = Node::create([
      'type' => 'test_type',
      'uid' => $this->user->id(),
      'title' => 'Rewrite test',
      'status' => 1,
    ]);
    $this->entity->save();
  }

  /**
   * Tests that configured rewrite rules are applied.
   */
  public function testRewriteRulesAreApplied() {
    $this->setRewriteRules("https://example.com|http://localhost");

    $message = $this->generateDerivativeMessage([
      'source_uri' => 'https://example.com/file.jpg',
      'destination_uri' => 'https://example.com/media/1/source',
      'file_upload_uri' => 'public://derivatives/test.mp4',
    ]);

    $this->assertEquals(
      'http://localhost/file.jpg',
      $message['attachment']['content']['source_uri']
    );
    $this->assertEquals(
      'http://localhost/media/1/source',
      $message['attachment']['content']['destination_uri']
    );
    $this->assertEquals(
      'public://derivatives/test.mp4',
      $message['attachment']['content']['file_upload_uri']
    );
  }

  /**
   * Tests that multiple rewrite rules are applied.
   */
  public function testMultipleRulesAreApplied() {
    $this->setRewriteRules("islandora-test.lib|islandora-stage.lib\nislandora-prod.lib|preserve.lib");

    $message = $this->generateDerivativeMessage([
      'source_uri' => 'https://islandora-test.lib/file.jpg',
      'destination_uri' => 'https://islandora-prod.lib/media/1/source',
    ]);

    $this->assertEquals(
      'https://islandora-stage.lib/file.jpg',
      $message['attachment']['content']['source_uri']
    );
    $this->assertEquals(
      'https://preserve.lib/media/1/source',
      $message['attachment']['content']['destination_uri']
    );
  }

  /**
   * Tests that URI values stay untouched when no rules are configured.
   */
  public function testNoRulesLeaveUrisUntouched() {
    $this->setRewriteRules('');

    $message = $this->generateDerivativeMessage([
      'source_uri' => 'https://example.com/_flysystem/fedora/file.jpg',
      'destination_uri' => 'https://example.com/media/1/source',
    ]);

    $this->assertEquals(
      'https://example.com/_flysystem/fedora/file.jpg',
      $message['attachment']['content']['source_uri']
    );
    $this->assertEquals(
      'https://example.com/media/1/source',
      $message['attachment']['content']['destination_uri']
    );
  }

  /**
   * Tests that malformed rules are ignored while valid ones still apply.
   */
  public function testMalformedRulesAreIgnored() {
    $this->setRewriteRules("invalid-line\nhttps://example.com|http://localhost");

    $message = $this->generateDerivativeMessage([
      'source_uri' => 'https://example.com/_flysystem/fedora/file.jpg',
      'destination_uri' => 'https://example.com/media/1/source',
    ]);

    $this->assertEquals(
      'http://localhost/_flysystem/fedora/file.jpg',
      $message['attachment']['content']['source_uri']
    );
    $this->assertEquals(
      'http://localhost/media/1/source',
      $message['attachment']['content']['destination_uri']
    );
  }

  /**
   * Saves rewrite rules to the submodule config.
   *
   * @param string $rules
   *   Newline-delimited rewrite rules.
   */
  protected function setRewriteRules($rules) {
    $this->config('islandora_microservice_rewrite.settings')
      ->set('rewrite_rules', $rules)
      ->save();
  }

  /**
   * Generates a derivative event message with the provided attachment data.
   *
   * @param array $data
   *   Attachment content fields for the generated event.
   *
   * @return array
   *   The decoded generated message.
   */
  protected function generateDerivativeMessage(array $data) {
    $json = $this->container->get('islandora.eventgenerator')->generateEvent(
      $this->entity,
      $this->user,
      ['event' => 'Generate Derivative'] + $data
    );

    return json_decode($json, TRUE);
  }

}
