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
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition
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
    // Islandora objects are determined by Islandora Utils.
    $utils = \Drupal::service('islandora.utils');
    if ($utils->isIslandoraType('node',$node->bundle())) {
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
