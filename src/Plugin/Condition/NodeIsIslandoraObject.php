<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Islandora\IslandoraUtils;

/**
 * Checks whether node has fields that qualify it as an "Islandora" node.
 *
 * @Condition(
 *   id = "node_is_islandora_object",
 *   label = @Translation("Node is an Islandora node"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = TRUE , label = @Translation("node"))
 *   }
 * )
 */
class NodeIsIslandoraObject extends ConditionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * Islandora Utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Constructs a Node is Islandora Condition plugin.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\islandora\IslandoraUtils $islandora_utils
   *   Islandora utilities.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, IslandoraUtils $islandora_utils) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->utils = $islandora_utils;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('islandora.utils')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    $node = $this->getContextValue('node');
    if (!$node) {
      return FALSE;
    }
    // Determine if node is Islandora.
    if ($this->utils->isIslandoraType('node', $node->bundle())) {
      return TRUE;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (!empty($this->configuration['negate'])) {
      return $this->t('The node is not an Islandora node.');
    }
    else {
      return $this->t('The node is an Islandora node.');
    }
  }

}
