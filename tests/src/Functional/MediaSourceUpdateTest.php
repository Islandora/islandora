<?php

namespace Drupal\Tests\islandora\Functional;

use Drupal\Core\Url;
use PHPUnit\Framework\Attributes\Group;

/**
 * Tests updating Media source File with PUT.
 */
#[Group('islandora')]
class MediaSourceUpdateTest extends IslandoraFunctionalTestBase {

  /**
   * Media to belong to the referencer.
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
   * User.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $account;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    // Make a user with appropriate permissions.
    $this->account = $this->drupalCreateUser([
      'view media',
      'create media',
      'update media',
    ]);
    $this->drupalLogin($this->account);

    // Make a file for the Media.
    $this->file = $this->container->get('entity_type.manager')->getStorage('file')->create([
      'uid' => $this->account->id(),
      'uri' => "public://test_file.txt",
      'filename' => "test_file.txt",
      'filemime' => "text/plain",
    ]);
    $this->file->setPermanent();
    $this->file->save();

    // Get the source field for the media.
    $type_configuration = $this->testMediaType->get('source_configuration');
    $source_field = $type_configuration['source_field'];

    // Make the media.
    $this->media = $this->container->get('entity_type.manager')->getStorage('media')->create([
      'bundle' => $this->testMediaType->id(),
      'name' => 'Media',
      "$source_field" => [$this->file->id()],
    ]);
    $this->media->save();
  }

  /**
   * Tests media source update via PUT request.
   */
  public function testMediaSourceUpdate() {
    // Hack out the guzzle client.
    $client = $this->getSession()->getDriver()->getClient()->getClient();

    $media_update_url = Url::fromRoute('islandora.media_source_update', ['media' => $this->media->id()])
      ->setAbsolute()
      ->toString();

    $file_contents = file_get_contents(__DIR__ . '/../../fixtures/test_file2.txt');

    // Update without Content-Type header should fail with 400.
    $options = [
      'auth' => [$this->account->getAccountName(), $this->account->pass_raw],
      'http_errors' => FALSE,
      'body' => $file_contents,
    ];
    $response = $client->request('PUT', $media_update_url, $options);
    $this->assertTrue($response->getStatusCode() == 400, "Expected 400, received {$response->getStatusCode()}");

    // Update without body should fail with 400.
    $options = [
      'auth' => [$this->account->getAccountName(), $this->account->pass_raw],
      'http_errors' => FALSE,
      'headers' => [
        'Content-Type' => 'text/plain',
      ],
    ];
    $response = $client->request('PUT', $media_update_url, $options);
    $this->assertTrue($response->getStatusCode() == 400, "Expected 400, received {$response->getStatusCode()}");

    // Should be successful.
    $options = [
      'auth' => [$this->account->getAccountName(), $this->account->pass_raw],
      'http_errors' => FALSE,
      'headers' => [
        'Content-Type' => 'text/plain',
      ],
      'body' => $file_contents,
    ];
    $response = $client->request('PUT', $media_update_url, $options);
    $this->assertTrue($response->getStatusCode() == 204, "Expected 204, received {$response->getStatusCode()}");

    // GET the media and compare file and metadata.
    $options = [
      'auth' => [$this->account->getAccountName(), $this->account->pass_raw],
      'http_errors' => FALSE,
    ];
    $url = $this->media->toUrl('canonical', ['absolute' => TRUE])->toString();
    $response = $client->request('GET', $url . '?_format=json', $options);
    $updated = json_decode($response->getBody(), TRUE);

    // Get the source field for the media.
    $type_configuration = $this->testMediaType->get('source_configuration');
    $source_field = $type_configuration['source_field'];

    $updated_file_contents = file_get_contents($updated[$source_field][0]['url']);

    $this->assertTrue($updated_file_contents == file_get_contents(__DIR__ . '/../../fixtures/test_file2.txt'), "Updated file not the same as PUT body.");
  }

}
