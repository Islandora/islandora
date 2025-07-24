<?php

namespace Drupal\Tests\islandora\Functional;

/**
 * Tests the NodeHasTerm condition.
 *
 * @package Drupal\Tests\islandora\Functional
 * @group islandora
 */
class NodeHasTermTest extends IslandoraFunctionalTestBase {

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {

    parent::setUp();

    // Set up two tags so we can test the condition.
    // Doesn't really matter what they are or what
    // vocab they belong to.
    $this->createImageTag();
    $this->createPreservationMasterTag();
  }

  /**
   * Data providing method.
   *
   * @return array
   *   Arrays of test parameters.
   */
  public function dataProvider() {
    return [
      'naive references' => [TRUE],
      'specific references' => [FALSE],
    ];
  }

  /**
   * @covers \Drupal\islandora\Plugin\Condition\NodeHasTerm
   * @dataProvider dataProvider
   */
  public function testNodeHasTerm(bool $naive_references) {

    // Create a new node with the tag.
    $node = $this->container->get('entity_type.manager')
      ->getStorage('node')
      ->create([
        'type' => 'test_type',
        'title' => 'Test Node',
        'field_model' => [$this->imageTerm->id()],
      ]);

    // Create and execute the condition.
    $condition_manager = $this->container->get('plugin.manager.condition');
    $condition = $condition_manager->createInstance(
      'node_has_term',
      [
        'uri' => 'http://purl.org/coar/resource_type/c_c513',
        'naive_references' => $naive_references,
      ]
    );
    $condition->setContextValue('node', $node);
    $this->assertTrue($condition->execute(), "Condition should pass if node has the term");

    // Create a new node without the tag.
    $node = $this->container->get('entity_type.manager')
      ->getStorage('node')
      ->create([
        'type' => 'test_type',
        'title' => 'Test Node',
      ]);

    $condition->setContextValue('node', $node);
    $this->assertFalse($condition->execute(), "Condition should fail if the node does not have any terms");

    // Create a new node with the wrong tag.
    $node = $this->container->get('entity_type.manager')
      ->getStorage('node')
      ->create([
        'type' => 'test_type',
        'title' => 'Test Node',
        'field_model' => [$this->preservationMasterTerm->id()],
      ]);

    $condition->setContextValue('node', $node);
    $this->assertFalse($condition->execute(), "Condition should fail if the node has terms, but not the one we want.");

    // Check for two tags this time.
    // Node still only has one.
    $condition = $condition_manager->createInstance(
      'node_has_term',
      [
        'uri' => 'http://purl.org/coar/resource_type/c_c513,http://pcdm.org/use#PreservationMasterFile',
        'logic' => 'or',
        'naive_references' => $naive_references,
      ]
    );
    $condition->setContextValue('node', $node);
    $this->assertTrue($condition->execute(), "Condition should pass if has one of two terms using OR logic.");

    // Create a node with both tags and try it with OR.
    $node = $this->container->get('entity_type.manager')
      ->getStorage('node')
      ->create([
        'type' => 'test_type',
        'title' => 'Test Node',
        'field_model' => [
          $this->imageTerm->id(),
          $this->preservationMasterTerm->id(),
        ],
      ]);
    $condition->setContextValue('node', $node);
    $this->assertTrue($condition->execute(), "Condition should pass if node has both terms using OR logic");

    // Try it with AND.
    $condition = $condition_manager->createInstance(
      'node_has_term',
      [
        'uri' => 'http://purl.org/coar/resource_type/c_c513,http://pcdm.org/use#PreservationMasterFile',
        'naive_references' => $naive_references,
      ]
    );
    $condition->setContextValue('node', $node);
    $this->assertTrue($condition->execute(), "Condition should pass if node has both terms using AND logic");
  }

}
