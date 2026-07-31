<?php

namespace Drupal\Tests\islandora\Functional;

use Stomp\Exception\StompException;

/**
 * Tests the GenerateDerivative action.
 */
abstract class GenerateDerivativeTestBase extends IslandoraFunctionalTestBase {

  /**
   * {@inheritdoc}
   */
  protected static $modules = ['context_ui'];

  /**
   * Node to hold the media.
   *
   * @var \Drupal\node\NodeInterface
   */
  protected $node;

  /**
   * Term to belong to the derivative media.
   *
   * @var \Drupal\taxonomy\TermInterface
   */
  protected $serviceFileTerm;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $this->createUserAndLogin();
    $this->createImageTag();
    $this->createPreservationMasterTag();

    // 'Service File' tag.
    $this->serviceFileTerm = $this->container->get('entity_type.manager')->getStorage('taxonomy_term')->create([
      'name' => 'Service File',
      'vid' => $this->testVocabulary->id(),
      'field_external_uri' => [['uri' => "http://pcdm.org/use#ServiceFile"]],
    ]);
    $this->serviceFileTerm->save();

    // Node to be referenced via media_of.
    $this->node = $this->container->get('entity_type.manager')->getStorage('node')->create([
      'type' => $this->testType->id(),
      'title' => 'Test Node',
      'field_model' => [$this->imageTerm->id()],
    ]);
    $this->node->save();
  }

  /**
   * Asserts a derivative event was delivered.
   *
   * @param array $expected
   *   The expected values.
   */
  protected function checkMessage(array $expected) {
    // Verify message is sent.
    $stomp = $this->container->get('islandora.stomp');
    try {
      $stomp->subscribe('generate-test-derivative');
      while ($msg = $stomp->read()) {
        $headers = $msg->getHeaders();
        $this->assertTrue(
          isset($headers['Authorization']),
          "Authorization header must be set"
        );
        $matches = [];
        $this->assertTrue(
          preg_match('/^Bearer (.*)/', $headers['Authorization'], $matches),
          "Authorization header must be a bearer token"
        );
        $this->assertTrue(
          count($matches) == 2 && !empty($matches[1]),
          "Bearer token must not be empty"
        );

        $body = $msg->getBody();
        $body = json_decode($body, TRUE);

        $type = $body['type'];
        $this->assertTrue($type == 'Activity', "Expected 'Activity', received $type");

        $summary = $body['summary'];
        $this->assertTrue($summary == 'Generate Derivative', "Expected 'Generate Derivative', received $summary");

        $content = $body['attachment']['content'];
        $this->assertTrue(
          strpos($content['source_uri'], $expected['source_uri']) !== FALSE,
          "Expected source uri should contain the file."
        );

        $this->assertTrue(
          strpos($content['destination_uri'], $expected['destination_uri']) !== FALSE,
          "Expected destination uri should reference both node and term"
        );
        $this->assertEquals($expected['file_upload_uri'],
          $content['file_upload_uri'],
          "Expected file upload uri should contain the scheme and path of the derivative"
        );

        $this->assertEquals($expected['mimetype'], $content['mimetype'], "Expected mimetype '{$expected['mimetype']}', received {$content['mimetype']}");

        $this->assertEquals($expected['args'], $content['args'], "Expected bundle '{$expected['args']}', received {$content['args']}");

      }
      $stomp->unsubscribe();
    }
    catch (StompException $e) {
      $this->assertTrue(FALSE, "There was an error connecting to the stomp broker");
    }
  }

}
