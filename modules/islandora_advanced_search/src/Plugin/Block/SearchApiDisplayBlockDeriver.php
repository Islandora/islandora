<?php

namespace Drupal\islandora_advanced_search\Plugin\Block;

use Drupal\Component\Plugin\PluginBase;
use Drupal\Core\Plugin\Discovery\ContainerDeriverInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * This deriver creates a block for every search_api.display.
 */
abstract class SearchApiDisplayBlockDeriver implements ContainerDeriverInterface {

  use StringTranslationTrait;

  /**
   * List of derivative definitions.
   *
   * @var array
   */
  protected $derivatives = [];

  /**
   * The entity storage for the view.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $storage;

  /**
   * The display manager for the search_api.
   *
   * @var \Drupal\search_api\Display\DisplayPluginManager
   */
  protected $displayPluginManager;

  /**
   * Label for the SearchApiDisplayBlockDriver.
   */
  abstract protected function label();

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, $base_plugin_id) {
    $deriver = new static($container, $base_plugin_id);
    $deriver->storage = $container->get('entity_type.manager')->getStorage('view');
    $deriver->displayPluginManager = $container->get('plugin.manager.search_api.display');
    return $deriver;
  }

  /**
   * {@inheritdoc}
   */
  public function getDerivativeDefinition($derivative_id, $base_plugin_definition) {
    $derivatives = $this->getDerivativeDefinitions($base_plugin_definition);
    return $derivatives[$derivative_id] ?? NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getDerivativeDefinitions($base_plugin_definition) {
    $base_plugin_id = $base_plugin_definition['id'];

    if (!isset($this->derivatives[$base_plugin_id])) {
      $plugin_derivatives = [];

      foreach ($this->displayPluginManager->getDefinitions() as $display_definition) {
        $view_id = $display_definition['view_id'];
        $view_display = $display_definition['view_display'];
        // The derived block needs both the view / display identifiers to
        // construct the pager.
        $machine_name = "${view_id}__${view_display}";

        /** @var \Drupal\views\ViewEntityInterface $view */
        $view = $this->storage->load($view_id);
        $display = $view->getDisplay($view_display);

        $plugin_derivatives[$machine_name] = [
          'id' => $base_plugin_id . PluginBase::DERIVATIVE_SEPARATOR . $machine_name,
          'label' => $this->label(),
          'admin_label' => $this->t(':view: :label for :display', [
            ':view' => $view->label(),
            ':label' => $this->label(),
            ':display' => $display['display_title'],
          ]),
        ] + $base_plugin_definition;
      }

      $this->derivatives[$base_plugin_id] = $plugin_derivatives;
    }
    return $this->derivatives[$base_plugin_id];
  }

}
