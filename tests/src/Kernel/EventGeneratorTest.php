<?php

namespace Drupal\Tests\islandora\Kernel;

use Drupal\islandora\EventGenerator\EventGenerator;
use Drupal\node\Entity\Node;
use Drupal\node\Entity\NodeType;
use Drupal\Tests\user\Traits\UserCreationTrait;

/**
 * Tests the EventGenerator default implementation.
 *
 * @group islandora
 * @coversDefaultClass \Drupal\islandora\EventGenerator\EventGenerator
 */
class EventGeneratorTest extends IslandoraKernelTestBase {

  use UserCreationTrait;

  /**
   * The EventGenerator to test.
   *
   * @var \Drupal\islandora\EventGenerator\EventGeneratorInterface
   */
  protected $eventGenerator;

  /**
   * User entity.
   *
   * @var \Drupal\user\UserInterface
   */
  protected $user;

  /**
   * Fedora resource entity.
   *
   * @var \Drupal\node\Entity\NodeInterface
   */
  protected $entity;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    // Create a test user.
    $this->user = $this->createUser(['administer nodes']);

    $test_type = NodeType::create([
      'type' => 'test_type',
      'name' => 'Test Type',
    ]);
    $test_type->save();

    // Create a test entity.
    $this->entity = Node::create([
      "type" => "test_type",
      "uid" => $this->user->get('uid'),
      "title" => "Test Fixture",
      "langcode" => "und",
      "status" => 1,
    ]);
    $this->entity->save();

    // Create the event generator so we can test it.
    $this->eventGenerator = new EventGenerator(
      $this->container->get('islandora.utils'),
      $this->container->get('islandora.media_source_service'),
      $this->container->get('config.factory')
    );
  }

  /**
   * Tests the generateCreateEvent() method.
   *
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::generateEvent
   */
  public function testGenerateCreateEvent() {
    $json = $this->eventGenerator->generateEvent(
      $this->entity,
      $this->user,
      ['event' => 'create', 'queue' => 'islandora-indexing-fcrepo-content']
    );
    $msg = json_decode($json, TRUE);

    $this->assertBasicStructure($msg);
    $this->assertTrue($msg["type"] == "Create", "Event must be of type 'Create'.");
  }

  /**
   * Tests the generateUpdateEvent() method.
   *
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::generateEvent
   */
  public function testGenerateUpdateEvent() {
    $json = $this->eventGenerator->generateEvent(
      $this->entity,
      $this->user,
      ['event' => 'update', 'queue' => 'islandora-indexing-fcrepo-content']
    );
    $msg = json_decode($json, TRUE);

    $this->assertBasicStructure($msg);
    $this->assertTrue($msg["type"] == "Update", "Event must be of type 'Update'.");
  }

  /**
   * Tests the generateDeleteEvent() method.
   *
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::generateEvent
   */
  public function testGenerateDeleteEvent() {
    $json = $this->eventGenerator->generateEvent(
      $this->entity,
      $this->user,
      ['event' => 'delete', 'queue' => 'islandora-indexing-fcrepo-delete']
    );
    $msg = json_decode($json, TRUE);

    $this->assertBasicStructure($msg);
    $this->assertTrue($msg["type"] == "Delete", "Event must be of type 'Delete'.");
  }

  /**
   * Util function for repeated checks.
   *
   * @param array $msg
   *   The message parsed as an array.
   */
  protected function assertBasicStructure(array $msg) {
    // Looking for @context.
    $this->assertTrue(array_key_exists('@context', $msg), "Expected @context entry");
    $this->assertTrue($msg["@context"] == "https://www.w3.org/ns/activitystreams", "@context must be activity stream.");

    // Make sure it has a type.
    $this->assertTrue(array_key_exists('type', $msg), "Message must have 'type' key.");

    // Make sure the actor exists, is a person, and has a uri.
    $this->assertTrue(array_key_exists('actor', $msg), "Message must have 'actor' key.");
    $this->assertTrue(array_key_exists("type", $msg["actor"]), "Actor must have 'type' key.");
    $this->assertTrue($msg["actor"]["type"] == "Person", "Actor must be a 'Person'.");
    $this->assertTrue(array_key_exists("id", $msg["actor"]), "Actor must have 'id' key.");
    $this->assertTrue(
        $msg["actor"]["id"] == "urn:uuid:{$this->user->uuid()}",
        "Id must be an URN with user's UUID"
    );
    $this->assertTrue(array_key_exists("url", $msg["actor"]), "Actor must have 'url' key.");
    foreach ($msg['actor']['url'] as $url) {
      $this->assertTrue($url['type'] == 'Link', "'url' entries must have type 'Link'");
      $this->assertTrue(
        in_array(
          $url['mediaType'],
          ['application/json', 'application/ld+json', 'text/html']
        ),
        "'url' entries must be either html, json, or jsonld"
      );
    }

    // Make sure the object exists and is a uri.
    $this->assertTrue(array_key_exists('object', $msg), "Message must have 'object' key.");
    $this->assertTrue(array_key_exists("id", $msg["object"]), "Object must have 'id' key.");
    $this->assertTrue(
        $msg["object"]["id"] == "urn:uuid:{$this->entity->uuid()}",
        "Id must be an URN with entity's UUID"
    );
    $this->assertTrue(array_key_exists("url", $msg["object"]), "Object must have 'url' key.");
    foreach ($msg['object']['url'] as $url) {
      $this->assertTrue($url['type'] == 'Link', "'url' entries must have type 'Link'");
      $this->assertTrue(
        in_array(
          $url['mediaType'],
          ['application/json', 'application/ld+json', 'text/html']
        ),
        "'url' entries must be either html, json, or jsonld"
      );
    }
  }

  /**
   * Tests URL rewriting in microservice events.
   *
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::generateEvent
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::applyUrlRewrites
   */
  public function testMicroserviceUrlRewrites() {
    // Configure URL rewrites.
    $config = $this->container->get('config.factory')->getEditable('islandora.settings');
    $config->set('microservice_url_rewrites', "https://example.com|http://localhost\nhttps://test.org|http://internal.local");
    $config->save();

    // Create a new event generator with the updated config.
    $this->eventGenerator = new EventGenerator(
      $this->container->get('islandora.utils'),
      $this->container->get('islandora.media_source_service'),
      $this->container->get('config.factory')
    );

    // Generate an event with URIs that should be rewritten.
    $json = $this->eventGenerator->generateEvent(
      $this->entity,
      $this->user,
      [
        'event' => 'Generate Derivative',
        'source_uri' => 'https://example.com/_flysystem/fedora/file.jpg',
        'destination_uri' => 'https://example.com/media/1/source',
        'file_upload_uri' => 'public://derivatives/test.mp4',
      ]
    );
    $msg = json_decode($json, TRUE);

    // Assert URIs were rewritten.
    $this->assertTrue(
      isset($msg['attachment']['content']['source_uri']),
      "Event should contain source_uri in attachment content"
    );
    $this->assertEquals(
      'http://localhost/_flysystem/fedora/file.jpg',
      $msg['attachment']['content']['source_uri'],
      "source_uri should be rewritten from https://example.com to http://localhost"
    );
    $this->assertEquals(
      'http://localhost/media/1/source',
      $msg['attachment']['content']['destination_uri'],
      "destination_uri should be rewritten from https://example.com to http://localhost"
    );
    $this->assertEquals(
      'public://derivatives/test.mp4',
      $msg['attachment']['content']['file_upload_uri'],
      "file_upload_uri should not be rewritten when it doesn't match any pattern"
    );
  }

  /**
   * Tests URL rewriting with multiple patterns.
   *
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::generateEvent
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::applyUrlRewrites
   */
  public function testMicroserviceUrlRewritesMultiplePatterns() {
    // Configure URL rewrites with multiple patterns.
    $config = $this->container->get('config.factory')->getEditable('islandora.settings');
    $config->set('microservice_url_rewrites', "islandora-test.lib|islandora-stage.lib\nislandora-prod.lib|preserve.lib");
    $config->save();

    // Create a new event generator with the updated config.
    $this->eventGenerator = new EventGenerator(
      $this->container->get('islandora.utils'),
      $this->container->get('islandora.media_source_service'),
      $this->container->get('config.factory')
    );

    // Generate an event with URIs that match different patterns.
    $json = $this->eventGenerator->generateEvent(
      $this->entity,
      $this->user,
      [
        'event' => 'Generate Derivative',
        'source_uri' => 'https://islandora-test.lib/file.jpg',
        'destination_uri' => 'https://islandora-prod.lib/media/1/source',
      ]
    );
    $msg = json_decode($json, TRUE);

    // Assert both patterns were applied.
    $this->assertEquals(
      'https://islandora-stage.lib/file.jpg',
      $msg['attachment']['content']['source_uri'],
      "source_uri should be rewritten from islandora-test.lib to islandora-stage.lib"
    );
    $this->assertEquals(
      'https://preserve.lib/media/1/source',
      $msg['attachment']['content']['destination_uri'],
      "destination_uri should be rewritten from islandora-prod.lib to preserve.lib"
    );
  }

  /**
   * Tests that events work correctly with no URL rewrites configured.
   *
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::generateEvent
   * @covers \Drupal\islandora\EventGenerator\EventGenerator::applyUrlRewrites
   */
  public function testNoUrlRewrites() {
    // Ensure no rewrites are configured.
    $config = $this->container->get('config.factory')->getEditable('islandora.settings');
    $config->set('microservice_url_rewrites', '');
    $config->save();

    // Create a new event generator with the updated config.
    $this->eventGenerator = new EventGenerator(
      $this->container->get('islandora.utils'),
      $this->container->get('islandora.media_source_service'),
      $this->container->get('config.factory')
    );

    // Generate an event.
    $json = $this->eventGenerator->generateEvent(
      $this->entity,
      $this->user,
      [
        'event' => 'Generate Derivative',
        'source_uri' => 'https://example.com/_flysystem/fedora/file.jpg',
        'destination_uri' => 'https://example.com/media/1/source',
      ]
    );
    $msg = json_decode($json, TRUE);

    // Assert URIs were NOT rewritten.
    $this->assertEquals(
      'https://example.com/_flysystem/fedora/file.jpg',
      $msg['attachment']['content']['source_uri'],
      "source_uri should not be rewritten when no rewrites are configured"
    );
    $this->assertEquals(
      'https://example.com/media/1/source',
      $msg['attachment']['content']['destination_uri'],
      "destination_uri should not be rewritten when no rewrites are configured"
    );
  }

}
