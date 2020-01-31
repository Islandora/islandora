<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Class MappingUriPredicateReactionTest.
 *
 * @package Drupal\Tests\islandora\Functional
 * @group islandora
 */
class JsonldSelfReferenceReactionTest extends IslandoraFunctionalTestBase {

  /**
   * {@inheritdoc}
   */
  public function setUp() {
    parent::setUp();

    $types = ['schema:Thing'];
    $created_mapping = [
      'properties' => ['schema:dateCreated'],
      'datatype' => 'xsd:dateTime',
      'datatype_callback' => ['callable' => 'Drupal\rdf\CommonDataConverter::dateIso8601Value'],
    ];

    // Save bundle mapping config.
    $this->rdfMapping = rdf_get_mapping('node', 'test_type')
      ->setBundleMapping(['types' => $types])
      ->setFieldMapping('created', $created_mapping)
      ->setFieldMapping('title', [
        'properties' => ['dcterms:title'],
        'datatype' => 'xsd:string',
      ])
      ->save();

    $this->container->get('router.builder')->rebuildIfNeeded();
  }

  /**
   * @covers \Drupal\islandora\Plugin\ContextReaction\JsonldSelfReferenceReaction
   */
  public function testMappingReaction() {
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer contexts',
    ]);
    $this->drupalLogin($account);

    $context_name = 'test';
    $reaction_id = 'islandora_map_uri_predicate';

    $this->postNodeAddForm('test_type',
      ['title[0][value]' => 'Test Node'],
      t('Save'));
    $this->assertSession()->pageTextContains("Test Node");
    $url = $this->getUrl();

    // Make sure the node exists.
    $this->drupalGet($url);
    $this->assertSession()->statusCodeEquals(200);

    $contents = $this->drupalGet($url . '?_format=jsonld');
    $this->assertSession()->statusCodeEquals(200);
    $json = \GuzzleHttp\json_decode($contents, TRUE);
    $this->assertArrayHasKey('http://purl.org/dc/terms/title',
      $json['@graph'][0], 'Missing dcterms:title key');
    $this->assertEquals(
      'Test Node',
      $json['@graph'][0]['http://purl.org/dc/terms/title'][0]['@value'],
      'Missing title value'
    );
    $this->assertArrayNotHasKey('http://www.w3.org/2002/07/owl#sameAs',
      $json['@graph'][0], 'Has predicate when not configured');

    $this->createContext('Test', $context_name);
    $this->drupalGet("admin/structure/context/$context_name/reaction/add/$reaction_id");
    $this->assertSession()->statusCodeEquals(200);

    $this->drupalGet("admin/structure/context/$context_name");
    // Can't use an undefined prefix.
    $this->getSession()->getPage()
      ->fillField("Self-reference predicate", "bob:smith");
    $this->getSession()->getPage()->pressButton("Save and continue");
    $this->assertSession()
      ->pageTextContains("Namespace prefix bob is not registered");

    // Can't use a straight string.
    $this->getSession()->getPage()
      ->fillField("Self-reference predicate", "woohoo");
    $this->getSession()->getPage()->pressButton("Save and continue");
    $this->assertSession()
      ->pageTextContains("Predicate must use a defined prefix or be a full URI");

    // Use an existing prefix.
    $this->getSession()->getPage()
      ->fillField("Self-reference predicate", "owl:sameAs");
    $this->getSession()->getPage()->pressButton("Save and continue");
    $this->assertSession()
      ->pageTextContains("The context $context_name has been saved");

    // The first time a Context is saved, you need to clear the cache.
    // Subsequent changes to the context don't need a cache rebuild, though.
    drupal_flush_all_caches();

    $new_contents = $this->drupalGet($url . '?_format=jsonld');
    $json = \GuzzleHttp\json_decode($new_contents, TRUE);
    $this->assertEquals(
      'Test Node',
      $json['@graph'][0]['http://purl.org/dc/terms/title'][0]['@value'],
      'Missing title value'
    );
    $this->assertEquals(
      "$url?_format=jsonld",
      $json['@graph'][0]['http://www.w3.org/2002/07/owl#sameAs'][0]['@id'],
      'Missing alter added predicate.'
    );

    $this->drupalGet("admin/structure/context/$context_name");
    // Change to a random URL.
    $this->getSession()->getPage()
      ->fillField("Self-reference predicate", "http://example.org/first/second");
    $this->getSession()->getPage()->pressButton("Save and continue");
    $this->assertSession()
      ->pageTextContains("The context $context_name has been saved");
    $new_contents = $this->drupalGet($url . '?_format=jsonld');
    $json = \GuzzleHttp\json_decode($new_contents, TRUE);
    $this->assertEquals(
      'Test Node',
      $json['@graph'][0]['http://purl.org/dc/terms/title'][0]['@value'],
      'Missing title value'
    );
    $this->assertArrayNotHasKey('http://www.w3.org/2002/07/owl#sameAs',
      $json['@graph'][0], 'Still has old predicate');
    $this->assertEquals(
      "$url?_format=jsonld",
      $json['@graph'][0]['http://example.org/first/second'][0]['@id'],
      'Missing alter added predicate.'
    );
  }

  /**
   * @covers \Drupal\islandora\Plugin\ContextReaction\JsonldSelfReferenceReaction
   */
  public function testMappingReactionForMedia() {
    $account = $this->drupalCreateUser([
      'create media',
      'view media',
      'administer contexts',
    ]);
    $this->drupalLogin($account);

    $context_name = 'test';
    $reaction_id = 'islandora_map_uri_predicate';

    list($file, $media) = $this->makeMediaAndFile($account);
    $media_url = $media->url('canonical', ['absolute' => TRUE]);
    $file_url = $file->url('canonical', ['absolute' => TRUE]);

    $this->drupalGet($media_url);
    $this->assertSession()->statusCodeEquals(200);

    $contents = $this->drupalGet($media_url . '?_format=jsonld');
    $this->assertSession()->statusCodeEquals(200);
    $json = \GuzzleHttp\json_decode($contents, TRUE);
    $this->assertEquals(
      "$media_url?_format=jsonld",
      $json['@graph'][0]['@id'],
      'Swapped file and media urls when not configured'
    );
    $this->assertArrayNotHasKey('http://www.iana.org/assignments/relation/describedby',
      $json['@graph'][0], 'Has predicate when not configured');

    $this->createContext('Test', $context_name);
    $this->drupalGet("admin/structure/context/$context_name/reaction/add/$reaction_id");
    $this->assertSession()->statusCodeEquals(200);

    // Use an existing prefix.
    $this->getSession()->getPage()
      ->fillField("Self-reference predicate", "iana:describedby");
    $this->getSession()->getPage()->pressButton("Save and continue");
    $this->assertSession()
      ->pageTextContains("The context $context_name has been saved");

    // The first time a Context is saved, you need to clear the cache.
    // Subsequent changes to the context don't need a cache rebuild, though.
    drupal_flush_all_caches();

    $new_contents = $this->drupalGet($media_url . '?_format=jsonld');
    $json = \GuzzleHttp\json_decode($new_contents, TRUE);
    $this->assertEquals(
      "$media_url?_format=jsonld",
      $json['@graph'][0]['http://www.iana.org/assignments/relation/describedby'][0]['@id'],
      'Missing alter added predicate.'
    );
    $this->assertEquals(
      $file_url,
      $json['@graph'][0]['@id'],
      'Alter did not swap "@id" of media with file url.'
    );

  }

}
