<?php

namespace Drupal\islandora\Plugin\views\filter;

use Drupal\Core\Form\FormStateInterface;
use Drupal\views\Plugin\views\filter\FilterPluginBase;
use Drupal\views\Plugin\ViewsHandlerManager;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\Core\Entity\EntityTypeBundleInfoInterface;

/**
 * Views Filter to show only Islandora nodes.
 *
 * @ingroup views_filter_handlers
 *
 * @ViewsFilter("islandora_node_is_islandora")
 */
class NodeIsIslandora extends FilterPluginBase implements ContainerFactoryPluginInterface {

  /**
   * Views Handler Plugin Manager.
   *
   * @var \Drupal\views\Plugin\ViewsHandlerManager
   */
  protected $joinHandler;

  /**
   * Islandora Utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * The entity type bundle info service.
   *
   * @var \Drupal\Core\Entity\EntityTypeBundleInfoInterface
   */
  protected $entityTypeBundleInfo;

  /**
   * Constructs a Node is Islandora views filter plugin.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\views\Plugin\ViewsHandlerManager $join_handler
   *   Views Handler Plugin Manager.
   * @param \Drupal\islandora\IslandoraUtils $islandora_utils
   *   Islandora utilities.
   * @param \Drupal\Core\Entity\EntityTypeBundleInfoInterface $entity_type_bundle_info
   *   The entity type bundle service.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, ViewsHandlerManager $join_handler, IslandoraUtils $islandora_utils, EntityTypeBundleInfoInterface $entity_type_bundle_info) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->joinHandler = $join_handler;
    $this->utils = $islandora_utils;
    $this->entityTypeBundleInfo = $entity_type_bundle_info;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration, $plugin_id, $plugin_definition,
      $container->get('plugin.manager.views.join'),
      $container->get('islandora.utils'),
      $container->get('entity_type.bundle.info')
    );
  }

  /**
   * {@inheritdoc}
   */
  protected function defineOptions() {
    return [
      'negated' => ['default' => FALSE],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function buildOptionsForm(&$form, FormStateInterface $form_state) {
    $types = [];
    foreach ($this->entityTypeBundleInfo->getBundleInfo('node') as $bundle_id => $bundle) {
      if ($this->utils->isIslandoraType('node', $bundle_id)) {
        $types[] = "{$bundle['label']} ($bundle_id)";
      }
    }
    $types_list = implode(', ', $types);
    $form['info'] = [
      '#type' => 'item',
      '#title' => 'Information',
      '#description' => t("Configured Islandora bundles: @types", ['@types' => $types_list]),
    ];
    $form['negated'] = [
      '#type' => 'checkbox',
      '#title' => 'Negated',
      '#description' => $this->t("Return nodes that <em>don't</em> have islandora fields"),
      '#default_value' => $this->options['negated'],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function adminSummary() {
    $operator = ($this->options['negated']) ? "is not" : "is";
    return "Node {$operator} an islandora node";
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $types = [];
    foreach (array_keys($this->entityTypeBundleInfo->getBundleInfo('node')) as $bundle_id) {
      if ($this->utils->isIslandoraType('node', $bundle_id)) {
        $types[] = $bundle_id;
      }
    }
    $condition = ($this->options['negated']) ? 'NOT IN' : 'IN';
    $query_base_table = $this->relationship ?: $this->view->storage->get('base_table');

    $definition = [
      'table' => 'node',
      'type' => 'LEFT',
      'field' => 'nid',
      'left_table' => $query_base_table,
      'left_field' => 'nid',
    ];
    $join = $this->joinHandler->createInstance('standard', $definition);
    $node_table_alias = $this->query->addTable('node', $this->relationship, $join);
    $this->query->addWhere($this->options['group'], "$node_table_alias.type", $types, $condition);
  }

}
