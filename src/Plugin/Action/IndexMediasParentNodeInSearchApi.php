<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Extension\ModuleHandler;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Indexes a media's parent node in Search API.
 *
 * @Action(
 *   id = "index_medias_parent_node_in_search_api",
 *   label = @Translation("Index a media's parent node in Search API"),
 *   type = "media"
 * )
 */
class IndexMediasParentNodeInSearchApi extends IndexNodeInSearchApi implements ContainerFactoryPluginInterface {
  use StringTranslationTrait;
  /**
   * Module handler.
   *
   * @var \Drupal\Core\Extension\ModuleHandler
   */
  protected ModuleHandler $moduleHandler;

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected IslandoraUtils $utils;

  /**
   * Constructor.
   *
   * @param array $configuration
   *   The plugin configuration, i.e. an array with configuration values keyed
   *    by configuration option name. The special key 'context' may be used to
   *    initialize the defined contexts by setting it to an array of context
   *    values keyed by context names.
   * @param mixed $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Extension\ModuleHandler $moduleHandler
   *   The Module Handler.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   The Islandora Utils.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, ModuleHandler $moduleHandler, IslandoraUtils $utils) {
    parent::__construct($configuration, $plugin_id, $plugin_definition, $moduleHandler);
    $this->utils = $utils;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('module_handler'),
      $container->get('islandora.utils')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function execute($media = NULL) {
    $media_revisions = [$media];
    // Get the original too, if it's an update.
    if (isset($media->original)) {
      $media_revisions[] = $media->original;
    }
    foreach ($media_revisions as $media) {
      $node = $this->utils->getParentNode($media);
      if ($node) {
        parent::execute($node);
      }
    }
  }

}
