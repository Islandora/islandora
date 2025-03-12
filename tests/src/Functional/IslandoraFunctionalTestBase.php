<?php

namespace Drupal\Tests\islandora\Functional;

use Drupal\Core\Config\FileStorage;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\link\LinkItemInterface;
use Drupal\Tests\BrowserTestBase;
use Drupal\Tests\field\Traits\EntityReferenceFieldCreationTrait;
use Drupal\Tests\media\Traits\MediaTypeCreationTrait;
use Drupal\Tests\TestFileCreationTrait;

/**
 * Base class for Functional tests.
 */
class IslandoraFunctionalTestBase extends BrowserTestBase {

  use EntityReferenceFieldCreationTrait;
  use TestFileCreationTrait;
  use MediaTypeCreationTrait;
  use StringTranslationTrait;

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
   * Term to belong to the node.
   *
   * @var \Drupal\taxonomy\TermInterface
   */
  protected $imageTerm;

  /**
   * Term to belong to the source media.
   *
   * @var \Drupal\taxonomy\TermInterface
   */
  protected $preservationMasterTerm;

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

    // Cache clear / rebuild.
    drupal_flush_all_caches();
    $this->container->get('router.builder')->rebuild();
  }

  /**
   * Create a new user and log them in.
   */
  protected function createUserAndLogin() {
    // Create a test user.
    $account = $this->drupalCreateUser();
    $this->drupalLogin($account);
    return $account;
  }

  /**
   * Create an Image tag.
   *
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  protected function createImageTag() {
    // 'Image' tag.
    $this->imageTerm = $this->container->get('entity_type.manager')
      ->getStorage('taxonomy_term')
      ->create([
        'name' => 'Image',
        'vid' => $this->testVocabulary->id(),
        'field_external_uri' => [['uri' => "http://purl.org/coar/resource_type/c_c513"]],
      ]);
    $this->imageTerm->save();
  }

  /**
   * Create a Preservation Master tag.
   *
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  protected function createPreservationMasterTag() {
    // 'Preservation Master' tag.
    $this->preservationMasterTerm = $this->container->get('entity_type.manager')
      ->getStorage('taxonomy_term')
      ->create([
        'name' => 'Preservation Master',
        'vid' => $this->testVocabulary->id(),
        'field_external_uri' => [['uri' => "http://pcdm.org/use#PreservationMasterFile"]],
      ]);
    $this->preservationMasterTerm->save();
  }

  /**
   * Creates a test context.
   */
  protected function createContext($label, $name) {
    $this->drupalGet('admin/structure/context/add');
    $values = [
      'label' => $label,
      'name' => $name,
    ];
    $this->submitForm($values, 'Save');

    $this->assertSession()->statusCodeEquals(200);
  }

  /**
   * Adds a condition to the test context.
   */
  protected function addCondition($context_id, $condition_id) {
    $this->drupalGet("admin/structure/context/$context_id/condition/add/$condition_id");
    $this->getSession()->getPage()->pressButton('Save and continue');
    $this->assertSession()->statusCodeEquals(200);
  }

  /**
   * Adds a reaction to the test context.
   */
  protected function addPresetReaction($context_id, $reaction_type, $action_id) {
    $this->drupalGet("admin/structure/context/$context_id/reaction/add/$reaction_type");
    $this->getSession()
      ->getPage()
      ->findById("edit-reactions-$reaction_type-actions")
      ->selectOption($action_id);
    $this->getSession()->getPage()->pressButton('Save and continue');
    $this->assertSession()->statusCodeEquals(200);
  }

  /**
   * Create a new node by posting its add form.
   */
  protected function postNodeAddForm($bundle_id, $values, $button_text) {
    $this->drupalGet("node/add/$bundle_id");
    $this->submitForm($values, $button_text);
    $this->assertSession()->statusCodeEquals(200);
  }

  /**
   * Create a new node by posting its add form.
   */
  protected function postTermAddForm($taxomony_id, $values, $button_text) {
    $this->drupalGet("admin/structure/taxonomy/manage/$taxomony_id/add");
    $this->submitForm($values, $button_text);
    $this->assertSession()->statusCodeEquals(200);
  }

  /**
   * Edits a node by posting its edit form.
   */
  protected function postEntityEditForm($entity_url, $values, $button_text) {
    $this->drupalGet("$entity_url/edit");
    $this->submitForm($values, $button_text);
    $this->assertSession()->statusCodeEquals(200);
  }

  /**
   * Utility function to check if a link header is included in the response.
   *
   * @param string $rel
   *   The relation to search for.
   *
   * @return bool
   *   TRUE if link header with relation is included in the response.
   */
  protected function doesNotHaveLinkHeader($rel) {
    $headers = $this->getSession()->getResponseHeaders();

    if (!isset($headers['Link'])) {
      return TRUE;
    }

    foreach ($headers['Link'] as $link_header) {
      if (strpos($link_header, "rel=\"$rel\"") !== FALSE) {
        return FALSE;
      }
    }

    return TRUE;
  }

  /**
   * Checks if the correct link header exists for an Entity.
   *
   * @param string $rel
   *   The expected relation type of the link header.
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity whose uri is expected in the link header.
   * @param string $title
   *   The expected title of the link header.
   * @param string $type
   *   The expected mimetype for the link header.
   *
   * @return int
   *   The number of times the correct header appears.
   */
  protected function validateLinkHeaderWithEntity($rel, EntityInterface $entity, $title = '', $type = '') {
    $entity_url = $entity->toUrl('canonical', ['absolute' => TRUE])
      ->toString();
    return $this->validateLinkHeaderWithUrl($rel, $entity_url, $title, $type);
  }

  /**
   * Checks if the correct link header exists for a string URI.
   *
   * @param string $rel
   *   The expected relation type of the link header.
   * @param string $url
   *   The uri is expected in the link header.
   * @param string $title
   *   The expected title of the link header.
   * @param string $type
   *   The expected mimetype for the link header.
   *
   * @return int
   *   The number of times the correct header appears.
   */
  protected function validateLinkHeaderWithUrl($rel, $url, $title = '', $type = '') {

    $regex = '/<(.*)>; rel="' . preg_quote($rel) . '"';
    if (!empty($title)) {
      $regex .= '; title="' . preg_quote($title) . '"';
    }
    if (!empty($type)) {
      $regex .= '; type="' . preg_quote($type, '/') . '"';
    }
    $regex .= '/';

    $count = 0;

    $headers = $this->getSession()->getResponseHeaders();

    foreach ($headers['Link'] as $link_headers) {
      $split = explode(',', $link_headers);
      foreach ($split as $link_header) {
        $matches = [];
        if (preg_match($regex, $link_header, $matches) && $matches[1] == $url) {
          $count++;
        }
      }
    }

    return $count;
  }

  /**
   * Utility function to make a media and an associated file.
   */
  protected function makeMediaAndFile(AccountInterface $account) {
    // Make a file for the Media.
    $file = $this->container->get('entity_type.manager')
      ->getStorage('file')
      ->create([
        'uid' => $account->id(),
        'uri' => "public://test_file.txt",
        'filename' => "test_file.txt",
        'filemime' => "text/plain",
      ]);
    $file->setPermanent();
    $file->save();

    // Get the source field for the media.
    $type_configuration = $this->testMediaType->get('source_configuration');
    $source_field = $type_configuration['source_field'];

    // Make the media for the referencer.
    $media = $this->container->get('entity_type.manager')
      ->getStorage('media')
      ->create([
        'bundle' => $this->testMediaType->id(),
        'name' => 'Media',
        "$source_field" => [$file->id()],
      ]);
    $media->save();

    return [$file, $media];
  }

}
