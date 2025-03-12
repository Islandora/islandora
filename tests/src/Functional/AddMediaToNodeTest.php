<?php

namespace Drupal\Tests\islandora\Functional;

use Drupal\Core\Url;
use Drupal\Tests\field\Traits\EntityReferenceFieldCreationTrait;

/**
 * Tests the RelatedLinkHeader view alter.
 *
 * @group islandora
 */
class AddMediaToNodeTest extends IslandoraFunctionalTestBase {

  use EntityReferenceFieldCreationTrait;

  /**
   * Node to hold the media.
   *
   * @var \Drupal\node\NodeInterface
   */
  protected $node;

  /**
   * Term to belong to the media.
   *
   * @var \Drupal\taxonomy\TermInterface
   */
  protected $preservationMasterTerm;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $this->node = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => 'test_type',
      'title' => 'Referencer',
    ]);
    $this->node->save();

    $this->preservationMasterTerm = $this->container->get('entity_type.manager')->getStorage('taxonomy_term')->create([
      'name' => 'Preservation Master',
      'vid' => $this->testVocabulary->id(),
      'field_external_uri' => [['uri' => "http://pcdm.org/use#PreservationMasterFile"]],
    ]);
    $this->preservationMasterTerm->save();
  }

  /**
   * @covers \Drupal\islandora\Controller\MediaSourceController::putToNode
   */
  public function testAddMediaToNode() {
    // Hack out the guzzle client.
    $client = $this->getSession()->getDriver()->getClient()->getClient();

    $add_to_node_url = Url::fromRoute(
      'islandora.media_source_put_to_node',
      [
        'node' => $this->node->id(),
        'media_type' => $this->testMediaType->id(),
        'taxonomy_term' => $this->preservationMasterTerm->id(),
      ]
    )
      ->setAbsolute()
      ->toString();

    $bad_node_url = Url::fromRoute(
      'islandora.media_source_put_to_node',
      [
        'node' => 123456,
        'media_type' => $this->testMediaType->id(),
        'taxonomy_term' => $this->preservationMasterTerm->id(),
      ]
    )
      ->setAbsolute()
      ->toString();

    $file_contents = file_get_contents(__DIR__ . '/../../fixtures/test_file.txt');

    // Test different permissions scenarios.
    $options = [
      'http_errors' => FALSE,
      'headers' => [
        'Content-Type' => 'text/plain',
        'Content-Location' => 'public://test_file.txt',
      ],
      'body' => $file_contents,
    ];

    // 403 if you don't have permissions to update the node.
    $account = $this->drupalCreateUser([
      'access content',
    ]);
    $this->drupalLogin($account);
    $options['auth'] = [$account->getDisplayName(), $account->pass_raw];
    $response = $client->request('PUT', $add_to_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 403, "Expected 403, received {$response->getStatusCode()}");

    // Bad node URL should return 404, regardless of permissions.
    // Just making sure our custom access function doesn't obfuscate responses.
    $response = $client->request('PUT', $bad_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 404, "Expected 404, received {$response->getStatusCode()}");

    // 403 if you don't have permissions to create Media.
    $account = $this->drupalCreateUser([
      'bypass node access',
    ]);
    $this->drupalLogin($account);
    $options['auth'] = [$account->getDisplayName(), $account->pass_raw];
    $response = $client->request('PUT', $add_to_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 403, "Expected 403, received {$response->getStatusCode()}");

    // Now with proper credentials, test responses given to malformed requests.
    $account = $this->drupalCreateUser([
      'bypass node access',
      'create media',
    ]);
    $this->drupalLogin($account);

    // Request without Content-Type header should fail with 400.
    $options = [
      'auth' => [$account->getDisplayName(), $account->pass_raw],
      'http_errors' => FALSE,
      'headers' => [
        'Content-Location' => 'public://test_file.txt',
      ],
      'body' => $file_contents,
    ];
    $response = $client->request('PUT', $add_to_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 400, "Expected 400, received {$response->getStatusCode()}");

    // Request without Content-Location header should fail with 400.
    $options = [
      'auth' => [$account->getDisplayName(), $account->pass_raw],
      'http_errors' => FALSE,
      'headers' => [
        'Content-Type' => 'text/plain',
      ],
      'body' => $file_contents,
    ];
    $response = $client->request('PUT', $add_to_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 400, "Expected 400, received {$response->getStatusCode()}");

    // Request without body should fail with 400.
    $options = [
      'auth' => [$account->getDisplayName(), $account->pass_raw],
      'http_errors' => FALSE,
      'headers' => [
        'Content-Type' => 'text/plain',
        'Content-Location' => 'public://test_file.txt',
      ],
    ];
    $response = $client->request('PUT', $add_to_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 400, "Expected 400, received {$response->getStatusCode()}");

    // Test properly formed requests with bad parameters in the route.
    $options = [
      'auth' => [$account->getDisplayName(), $account->pass_raw],
      'http_errors' => FALSE,
      'headers' => [
        'Content-Type' => 'text/plain',
        'Content-Location' => 'public://test_file.txt',
      ],
      'body' => $file_contents,
    ];

    // Bad node id should return 404 even with proper permissions.
    $response = $client->request('PUT', $bad_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 404, "Expected 404, received {$response->getStatusCode()}");

    // Bad media type in url should return 404.
    $bad_media_type_url = Url::fromRoute(
      'islandora.media_source_put_to_node',
      [
        'node' => $this->node->id(),
        'media_type' => 'derp',
        'taxonomy_term' => $this->preservationMasterTerm->id(),
      ]
    )
      ->setAbsolute()
      ->toString();
    $response = $client->request('PUT', $bad_media_type_url, $options);
    $this->assertTrue($response->getStatusCode() == 404, "Expected 404, received {$response->getStatusCode()}");

    // Bad taxonomy term in url should return 404.
    $bad_term_url = Url::fromRoute(
      'islandora.media_source_put_to_node',
      [
        'node' => $this->node->id(),
        'media_type' => $this->testMediaType->id(),
        'taxonomy_term' => 123456,
      ]
    )
      ->setAbsolute()
      ->toString();
    $response = $client->request('PUT', $bad_term_url, $options);
    $this->assertTrue($response->getStatusCode() == 404, "Expected 404, received {$response->getStatusCode()}");

    // Should be successful with proper url, options, and permissions.
    $options = [
      'auth' => [$account->getDisplayName(), $account->pass_raw],
      'http_errors' => FALSE,
      'headers' => [
        'Content-Type' => 'text/plain',
        'Content-Location' => 'public://test_file.txt',
      ],
      'body' => $file_contents,
    ];
    $response = $client->request('PUT', $add_to_node_url, $options);
    $this->assertTrue($response->getStatusCode() == 201, "Expected 201, received {$response->getStatusCode()}");
    $this->assertTrue(!empty($response->getHeader("Location")), "Response must include Location header");
  }

}
