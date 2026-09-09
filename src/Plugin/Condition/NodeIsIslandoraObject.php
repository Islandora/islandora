<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Checks whether node has fields that qualify it as an "Islandora" node.
 *
 * @Condition(
 *   id = "node_is_islandora_object",
 *   label = @Translation("Node is an Islandora node"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node",
 *       required = FALSE,
 *       label = @Translation("Node source"),
 *       description = @Translation("The node source must be set for this condition to work as expected.")
 *     )
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
  public function defaultConfiguration() {
    $defaults = parent::defaultConfiguration();

    // XXX: There appear to be expectations in Drupal that there will be more
    // config to a plugin than just selecting the context mapping; however, it
    // is our only configuration. Due to these expectations, it would fail to
    // save our visibility settings. To work around, it seems to be sufficient
    // to dynamically declare our default configuration, such that the
    // difference from the default configuration can be detected upstream.
    // @see https://git.drupalcode.org/project/drupal/-/blob/d87ab76d397a2cfe0457997be4f2648c4760b2f5/core/lib/Drupal/Core/Condition/ConditionPluginCollection.php#L39-44
    if (!empty($this->configuration['context_mapping'])) {
      $defaults += [
        'context_mapping' => [],
      ];
    }

    return $defaults;
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
    return FALSE;
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
