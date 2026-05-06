<?php

namespace Drupal\Tests\islandora\FunctionalJavascript;

use GuzzleHttp\Client;
use GuzzleHttp\Cookie\CookieJar;
use GuzzleHttp\Cookie\SetCookie;
use Drupal\Core\Config\FileStorage;
use Drupal\field\Entity\FieldConfig;
use Drupal\field\Entity\FieldStorageConfig;
use Drupal\FunctionalJavascriptTests\WebDriverTestBase;
use Drupal\link\LinkItemInterface;
use Drupal\Tests\field\Traits\EntityReferenceFieldCreationTrait;
use Drupal\Tests\media\Traits\MediaTypeCreationTrait;
use PHPUnit\Framework\Attributes\Group;

/**
 * Tests Jsonld Type Alter Reaction.
 */
#[Group('islandora')]
class JsonldTypeAlterReactionTest extends WebDriverTestBase {

  use EntityReferenceFieldCreationTrait;
  use MediaTypeCreationTrait;

  /**
   * {@inheritdoc}
   */
  protected static $modules = [
    'context_ui',
    'field_ui',
    'islandora',
    'menu_link_content',
  ];

  /**
   * {@inheritdoc}
   */
  protected $defaultTheme = 'stark';

  /**
   * {@inheritdoc}
   */
  protected static $configSchemaCheckerExclusions = [
    'jwt.config',
    'context.context.test',
    'context.context.node',
    'context.context.media',
    'context.context.file',
    'key.key.test',
    'media.settings',
  ];

  /**
   * Test node type.
   *
   * @var \Drupal\node\Entity\NodeType
   */
  protected $testType;

  /**
   * Test media type.
   *
   * @var \Drupal\media\Entity\MediaType
   */
  protected $testMediaType;

  /**
   * Test vocabulary.
   *
   * @var \Drupal\taxonomy\Entity\Vocabulary
   */
  protected $testVocabulary;

  /**
   * An RDF Mapping object.
   *
   * @var \Drupal\rdf\Entity\RdfMapping
   */
  protected $rdfMapping;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    // Delete the node rest config that's bootstrapped with Drupal.
    $this->container->get('entity_type.manager')
      ->getStorage('rest_resource_config')
      ->load('entity.node')
      ->delete();

    // Set up JWT stuff.
    $key_value = <<<EOD
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEA6ZT5qNjI4WlXpXzXVuo69MQ0K11V1ZmwW7JaztX0Qsi87JCi
saDIhQps2dEBND2YYKG3AehNFd/a0+ttnKPOnqr13uCVewxpgpPD4lYD0XcCD/U1
pPpOmHYrSOoVtmJvZfr5gQQb0izNM/k0wrO5r5UZzsDPX343HQuiBXzFJtIKau3n
TKjjqs5ErdnftmqsnDhI28yUtlwfSjaRVBIevIT5LGmAboWDukHxf9/x1EemvgMG
E9TQL/+JdLs+LiZglJWWeGofkcThGRcTefHe9GqxoBPtwf/rs6CKN7n3MXGfaxjl
r/dKjJ8Lg5NCrINLUFcNNZippDWIUvj/8lLBXwIDAQABAoIBABmwsOTJMw7XrzQc
TvLYQDO7gKFkWpRrmuH689Hb5kmSGnVKUxqGPIelZeNvAVrli2TVZHNpQVEulbrJ
If0gZxE8bF5fBRHLg69A4UJ7g1/+XtOyfHvwq8RI+unCFTFCEk59FAQEl6q+ErOs
rQjdC4csNvJucmBmWVlwdhl0Z5qlOX3EN/ZXCDnTJsKz75mfa8LC+izXaSv+Gesp
h80wc2V/O9H32djCuz/Ct3WLdHCTQuTiZ32fZAILk/AlZHCHjki5PaLHxAySTmo6
FmJ09/ns0EGuaa1IZz98xLn0yAfAX+MGfsWTsKzAxTO1FcMWvj23mAbwD3Q65ayv
ieMWGwECgYEA/QNKuofgfXu95H+IQMn4l/8zXPt4SdGqGxD5/SlOSi292buoJOF/
eLLlDwsHjQ3+XeFXHHgRyGxD7ZyYe5urFxYrabXlNCIidNVhQAgu31i866cs/Sy4
z0UOzVk5ZCQdvx77/Av8Xe5SBVir54KGRa6h+QMnh7DZNHM3Yha+y+8CgYEA7Fb0
hDCA2YJ6Vb6PeqRPyzsKJP4bQNP1JSM8eThk6RZ/ecAuU9uQjjUuB/O/UeEBRt4w
KUCYoyHLTraPs98N8I000SCoejLjqpyf7SOB2LjGIYPjaTTiXlqJoewWPV5KOoeN
pd+PTTTWeRSpFGjnqkSXCpa8e933raxtkLHPsZECgYBhBl4l4e1osYdElNN/ZPR7
9VWRFq4uQMTm1D/JoYlwUNI5KQl1+zOS6aeFeUlQAknFXqC1PiYzobD68c5XuH6H
v+yuAR8AOwbTnvBISdsPs0vfYqCSBhBpC6Z9gPXNPTxbClq/cSk6LCYv/q0NfrRX
DHz4rQj/tAXXY0edyfMo6QKBgGgBqF+YHMwb4IxlbSzyrG7qj39SGFpCLOroA8/w
4m+1R+ojif+7a3U5sAUt3m9BDtfKJfWxiLqZv6fnLXxh1/eZnLm/noUQaiKGBNdO
PfFK915+dRCyhkAxpcoNZIgjO5VgXBS4Oo8mhpAIaJQjynei8blmNpJoT3wtmpYH
ujgRAoGALyTXD/v/kxkE+31rmga1JM2IyjVwzefmqcdzNo3e8KovtZ79FJNfgcEx
FZTd3w207YHqKu/CX/BF15kfIOh03t+0AEUyKUTY5JWS84oQPU6td1DOSA6P36xl
EOLIc/4JOdONrJKWYpWIjDhHLL8BacjLoh2bDY0KdYa69AfYvW4=
-----END RSA PRIVATE KEY-----
EOD;

    $key = $this->container->get('entity_type.manager')
      ->getStorage('key')
      ->create([
        'id' => 'test',
        'label' => 'Test',
        'key_type' => 'jwt_rs',
        'key_type_settings' => [
          'algorithm' => 'RS256',
        ],
        'key_provider' => 'config',
        'key_provider_settings' => [
          'key_value' => $key_value,
        ],
      ]);
    $key->save();

    $jwt_config = $this->container->get('config.factory')
      ->getEditable('jwt.config');
    $jwt_config->set('algorithm', 'RS256');
    $jwt_config->set('key_id', 'test');
    $jwt_config->save(TRUE);

    // Make some bundles and field by hand so hooks fire.
    // Create an action that dsm's "Hello World!".
    $hello_world = $this->container->get('entity_type.manager')
      ->getStorage('action')
      ->create([
        'id' => 'hello_world',
        'label' => 'Hello World',
        'type' => 'system',
        'plugin' => 'action_message_action',
        'configuration' => [
          'message' => 'Hello World!',
        ],
      ]);
    $hello_world->save();

    // Create a vocabulary.
    $this->testVocabulary = $this->container->get('entity_type.manager')
      ->getStorage('taxonomy_vocabulary')
      ->create([
        'name' => 'Test Vocabulary',
        'vid' => 'test_vocabulary',
      ]);
    $this->testVocabulary->save();

    // Create an external_uri field for taxonomy terms.
    $fieldStorage = $this->container->get('entity_type.manager')
      ->getStorage('field_storage_config')
      ->create([
        'field_name' => 'field_external_uri',
        'entity_type' => 'taxonomy_term',
        'type' => 'link',
      ]);
    $fieldStorage->save();
    $field = $this->container->get('entity_type.manager')
      ->getStorage('field_config')
      ->create([
        'field_storage' => $fieldStorage,
        'bundle' => $this->testVocabulary->id(),
        'settings' => [
          'title' => 'External URI',
          'link_type' => LinkItemInterface::LINK_EXTERNAL,
        ],
      ]);
    $field->save();

    // Create a test content type.
    $this->testType = $this->container->get('entity_type.manager')
      ->getStorage('node_type')
      ->create([
        'type' => 'test_type',
        'name' => 'Test Type',
      ]);
    $this->testType->save();
    $this->createEntityReferenceField('node', 'test_type', 'field_member_of', 'Member Of', 'node', 'default', [], 2);
    $this->createEntityReferenceField('node', 'test_type', 'field_model', 'Model', 'taxonomy_term', 'default', [], 2);

    // Create a media type.
    $this->testMediaType = $this->createMediaType('file', ['id' => 'test_media_type']);
    $this->testMediaType->save();
    $this->createEntityReferenceField('media', $this->testMediaType->id(), 'field_media_of', 'Media Of', 'node', 'default', [], 2);
    $this->createEntityReferenceField('media', $this->testMediaType->id(), 'field_media_use', 'Tags', 'taxonomy_term', 'default', [], 2);

    // Copy over the rest of the config from yml files.
    $source = new FileStorage(__DIR__ . '/../../fixtures/config');
    $destination = $this->container->get('config.storage');

    foreach ($source->listAll() as $name) {
      $destination->write($name, $source->read($name));
    }

    $media_settings = $this->container->get('config.factory')
      ->getEditable('media.settings');
    $media_settings->set('standalone_url', TRUE);
    $media_settings->save(TRUE);

    // Set up RDF mapping for JSON-LD.
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

    // Cache clear / rebuild.
    drupal_flush_all_caches();
    $this->container->get('router.builder')->rebuild();
  }

  /**
   * Creates a test context.
   */
  protected function createContext($label, $name) {
    $this->drupalGet('admin/structure/context/add');
    $this->assertSession()->waitForField('label');

    $page = $this->getSession()->getPage();
    $page->fillField('label', $label);
    // Wait for machine name to be auto-generated via AJAX.
    $this->assertSession()->waitForElementVisible('css', '.machine-name-value');

    // Fill the name field if it's visible, otherwise let it auto-generate.
    $nameField = $page->findField('name');
    if ($nameField && $nameField->isVisible()) {
      $nameField->setValue($name);
    }

    $page->pressButton('Save');
    $this->assertSession()->pageTextContains("The context $name has been added");
  }

  /**
   * Adds a condition to the test context.
   */
  protected function addCondition($context_id, $condition_id) {
    $this->drupalGet("admin/structure/context/$context_id/condition/add/$condition_id");
    $this->assertSession()->waitForButton('Save and continue');
    $this->getSession()->getPage()->pressButton('Save and continue');
    $this->assertSession()->waitForText("The context $context_id has been saved");
  }

  /**
   * Create a new node by posting its add form.
   */
  protected function postNodeAddForm($bundle_id, $values, $button_text) {
    $this->drupalGet("node/add/$bundle_id");
    $this->assertSession()->waitForButton($button_text);
    $this->submitForm($values, $button_text);
  }

  /**
   * Fetches JSON-LD content using HTTP client.
   *
   * FunctionalJavascript tests use Selenium which renders pages in a browser,
   * so we need to use Guzzle directly to fetch raw JSON responses.
   *
   * @param string $url
   *   The URL to fetch.
   *
   * @return array
   *   The decoded JSON response.
   */
  protected function getJsonLd($url) {
    // Get session cookies to maintain authentication.
    $session = $this->getSession();
    $cookies = $session->getDriver()->getWebDriverSession()->getAllCookies();
    $cookieJar = new CookieJar();
    foreach ($cookies as $cookie) {
      $cookieJar->setCookie(new SetCookie([
        'Name' => $cookie['name'],
        'Value' => $cookie['value'],
        'Domain' => $cookie['domain'] ?? parse_url($url, PHP_URL_HOST),
        'Path' => $cookie['path'] ?? '/',
      ]));
    }

    $client = new Client([
      'verify' => FALSE,
      'timeout' => 30,
    ]);
    $response = $client->get($url, [
      'cookies' => $cookieJar,
      'http_errors' => FALSE,
    ]);

    $statusCode = $response->getStatusCode();
    $contents = (string) $response->getBody();

    $json = json_decode($contents, TRUE);
    if (json_last_error() !== JSON_ERROR_NONE) {
      $this->fail(sprintf(
        "Failed to decode JSON-LD from %s (HTTP %d): %s\nResponse body (first 1000 chars): %s",
        $url,
        $statusCode,
        json_last_error_msg(),
        substr($contents, 0, 1000)
      ));
    }

    return $json;
  }

  /**
   * Tests the JSON-LD type alter reaction.
   */
  public function testMappingReaction() {
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer contexts',
    ]);
    $this->drupalLogin($account);

    // Create the typed predicate field programmatically.
    $fieldStorage = FieldStorageConfig::create([
      'field_name' => 'field_type_predicate',
      'entity_type' => 'node',
      'type' => 'string',
    ]);
    $fieldStorage->save();

    $field = FieldConfig::create([
      'field_storage' => $fieldStorage,
      'bundle' => 'test_type',
      'label' => 'Typed Predicate',
      'required' => FALSE,
    ]);
    $field->save();

    // Configure the form display to show the field.
    /** @var \Drupal\Core\Entity\EntityDisplayRepositoryInterface $display_repository */
    $display_repository = \Drupal::service('entity_display.repository');
    $display_repository->getFormDisplay('node', 'test_type')
      ->setComponent('field_type_predicate', ['type' => 'string_textfield'])
      ->save();

    // Add the test node.
    $this->postNodeAddForm('test_type', [
      'title[0][value]' => 'Test Node',
      'field_type_predicate[0][value]' => 'schema:Organization',
    ], 'Save');
    $this->assertSession()->pageTextContains("Test Node");
    $url = $this->getUrl();

    // Get the JSON-LD representation of the node.
    $json = $this->getJsonLd($url . '?_format=jsonld');
    $this->assertArrayHasKey('@type',
      $json['@graph'][0], 'Missing @type');
    $this->assertEquals(
      'http://schema.org/Thing',
      $json['@graph'][0]['@type'][0],
      'Missing @type value of http://schema.org/Thing'
    );

    // Add the test context.
    $context_name = 'test';
    $reaction_id = 'alter_jsonld_type';

    $this->createContext('Test', $context_name);
    $this->drupalGet("admin/structure/context/$context_name/reaction/add/$reaction_id");
    $this->assertSession()
      ->pageTextContains("Field containing RDF type information");

    $this->drupalGet("admin/structure/context/$context_name");
    $this->getSession()->getPage()
      ->fillField("Field containing RDF type information", "field_type_predicate");
    $this->getSession()->getPage()->pressButton("Save and continue");
    $this->assertSession()
      ->pageTextContains("The context $context_name has been saved");

    $this->addCondition('test', 'islandora_entity_bundle');
    $this->getSession()->getPage()->checkField("edit-conditions-islandora-entity-bundle-bundles-test-type");
    $this->getSession()->getPage()->findById("edit-conditions-islandora-entity-bundle-context-mapping-node")->selectOption("@node.node_route_context:node");
    $this->getSession()->getPage()->pressButton('Save and continue');

    // The first time a Context is saved, you need to clear the cache.
    // Subsequent changes to the context don't need a cache rebuild, though.
    drupal_flush_all_caches();

    // Check for the new @type from the field_type_predicate value.
    $json = $this->getJsonLd($url . '?_format=jsonld');
    $this->assertTrue(
      in_array('http://schema.org/Organization', $json['@graph'][0]['@type']),
      'Missing altered @type value of http://schema.org/Organization'
    );
  }

}
