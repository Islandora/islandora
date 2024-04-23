<?php

namespace Drupal\Tests\islandora\Functional;

use Drupal\Core\Url;

/**
 * Tests link headers get added to GET requests.
 *
 * @group islandora
 */
class LinkHeaderTest extends IslandoraFunctionalTestBase {

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
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $account = $this->createUserAndLogin();
    $this->createImageTag();
    $this->createPreservationMasterTag();

    // Node to be referenced via member of.
    $this->referenced = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => $this->testType->id(),
      'title' => 'Referenced',
    ]);
    $this->referenced->save();

    // Node that is member of something, with an Image tag.
    $this->referencer = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => $this->testType->id(),
      'title' => 'Referencer',
      'field_member_of' => [$this->referenced->id()],
      'field_model' => [$this->imageTerm->id()],
    ]);
    $this->referencer->save();

    list($this->file, $this->media) = $this->makeMediaAndFile($account);
    $this->media->set('field_media_of', $this->referencer);
    $this->media->set('field_media_use', $this->preservationMasterTerm);
    $this->media->save();
  }

  /**
   * @covers \Drupal\islandora\EventSubscriber\NodeLinkHeaderSubscriber::onResponse
   */
  public function testNodeLinkHeaders() {
    // Visit the referenced node, there should not be a related header since
    // its entity reference field is empty.
    $this->drupalGet('node/' . $this->referenced->id());
    $this->assertTrue(
      $this->doesNotHaveLinkHeader('related'),
      "Node that has empty entity reference field must not return link header."
    );

    // Visit the referencer. It should return a rel="related" link header
    // for referenced node and the referencing media, plus one for the tag.
    $this->drupalGet('node/' . $this->referencer->id());
    $this->assertTrue(
      $this->validateLinkHeaderWithEntity('related', $this->referenced, 'Member Of') == 1,
      "Malformed related node link header"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithEntity('related', $this->media, $this->preservationMasterTerm->label()) == 1,
      "Malformed related media link header"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('tag', $this->imageTerm->get('field_external_uri')->first()->getValue()['uri'], $this->imageTerm->label()) == 1,
      "Malformed tag link header"
    );

    // Check for links to REST endpoints for metadata.
    $entity_url = $this->referencer->toUrl('canonical', ['absolute' => TRUE])
      ->toString();
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', "$entity_url?_format=json", NULL, 'application/json') == 1,
      "Node must have link header pointing to json REST endpoint."
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', "$entity_url?_format=jsonld", NULL, 'application/ld+json') == 1,
      "Node must have link header pointing to jsonld REST endpoint."
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', "$entity_url?_format=xml", NULL, 'application/xml') == 0,
      "Node must not have link header pointing to disabled xml REST endpoint."
    );

    // Check that the current representation is not advertised when visitng
    // a REST endpoint (e.g. the json link header doesn't appear when you're
    // visiting the ?_format=json endpoint).
    $this->drupalGet('node/' . $this->referencer->id(), ['query' => ['_format' => 'json']]);
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', "$entity_url?_format=json", NULL, 'application/json') == 0,
      "Node must not have link header pointing to json REST endpoint when vising the json REST endpoint."
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', "$entity_url?_format=jsonld", NULL, 'application/ld+json') == 1,
      "Node must have link header pointing to jsonld REST endpoint when visiting the json REST endpoint."
    );
  }

  /**
   * @covers \Drupal\islandora\EventSubscriber\MediaLinkHeaderSubscriber
   */
  public function testMediaLinkHeaders() {

    // Get the file to check its url in the response headers.
    $file_url = $this->file->createFileUrl(FALSE);
    $rest_url = Url::fromRoute('islandora.media_source_update', ['media' => $this->media->id()])
      ->setAbsolute()
      ->toString();
    $media_url = $this->media->toUrl('canonical', ['absolute' => TRUE])->toString();

    // Perform a GET request as anonymous.
    $this->drupalGet($media_url, [], ['Cache-Control' => 'no-cache']);
    // Check link headers.
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('describes', $file_url, '', 'text/plain') == 1,
      "Malformed 'describes' link header, expecting $file_url"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('edit-media', $rest_url, '', '') == 0,
      "Anonymous should not be able to see the edit-media link"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('tag', $this->preservationMasterTerm->get('field_external_uri')->first()->getValue()['uri'], $this->preservationMasterTerm->label()) == 1,
      "Malformed tag link header"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', $media_url . "?_format=json", NULL, 'application/json') == 1,
      "Media must have link header pointing to json REST endpoint."
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', $media_url . "?_format=jsonld", NULL, 'application/ld+json') == 1,
      "Media must have link header pointing to jsonld REST endpoint."
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', $media_url . "?_format=xml", NULL, 'application/xml') == 0,
      "Media must not have link header pointing to disabled xml REST endpoint."
    );

    // Create a test user with edit media permissions.
    $account = $this->drupalCreateUser(['update media']);
    $this->drupalLogin($account);

    // Perform a GET request with update media permissions.
    $this->drupalGet($media_url, [], ['Cache-Control' => 'no-cache']);

    // Check link headers again, the edit-media link header should be present.
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('describes', $file_url, '', 'text/plain') == 1,
      "Malformed 'describes' link header, expecting $file_url"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('edit-media', $rest_url, '', '') == 1,
      "Malformed 'edit-media' link, expecting $rest_url"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('tag', $this->preservationMasterTerm->get('field_external_uri')->first()->getValue()['uri'], $this->preservationMasterTerm->label()) == 1,
      "Malformed tag link header"
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', $media_url . "?_format=json", NULL, 'application/json') == 1,
      "Media must have link header pointing to json REST endpoint."
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', $media_url . "?_format=jsonld", NULL, 'application/ld+json') == 1,
      "Media must have link header pointing to jsonld REST endpoint."
    );
    $this->assertTrue(
      $this->validateLinkHeaderWithUrl('alternate', $media_url . "?_format=xml", NULL, 'application/xml') == 0,
      "Media must not have link header pointing to disabled xml REST endpoint."
    );
  }

}
