<?php

namespace Drupal\islandora_breadcrumbs;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Breadcrumb\Breadcrumb;
use Drupal\Core\Breadcrumb\BreadcrumbBuilderInterface;
use Drupal\Core\Link;
use Drupal\Core\Routing\RouteMatchInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\islandora\IslandoraUtils;

/**
 * Provides breadcrumbs for nodes using a configured entity reference field.
 */
class IslandoraBreadcrumbBuilder implements BreadcrumbBuilderInterface {
  use StringTranslationTrait;

  /**
   * The configuration.
   *
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  protected $config;

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Constructs a breadcrumb builder.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   The configuration factory.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils service.
   */
  public function __construct(EntityTypeManagerInterface $entity_type_manager, ConfigFactoryInterface $config_factory, IslandoraUtils $utils) {
    $this->entityTypeManager = $entity_type_manager;
    $this->config = $config_factory->get('islandora_breadcrumbs.breadcrumbs');
    $this->utils = $utils;
  }

  /**
   * {@inheritdoc}
   */
  public function applies(RouteMatchInterface $attributes) {
    // Using getRawParameters for consistency (always gives a
    // node ID string) because getParameters sometimes returns
    // a node ID string and sometimes returns a node object.
    $nid = $attributes->getRawParameters()->get('node');
    if (!empty($nid)) {
      $node = $this->entityTypeManager->getStorage('node')->load($nid);
      if (empty($node)) {
        return FALSE;
      }
      foreach ($this->config->get('referenceFields') as $field) {
        if ($node->hasField($field)) {
          return TRUE;
        }
      }
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function build(RouteMatchInterface $route_match) {

    $nid = $route_match->getRawParameters()->get('node');
    $node_storage = $this->entityTypeManager->getStorage('node');
    $node = $node_storage->load($nid);
    $breadcrumb = new Breadcrumb();
    $breadcrumb->addCacheableDependency($this->config);
    $breadcrumb->addLink(Link::createFromRoute($this->t('Home'), '<front>'));

    $chain = array_reverse($this->utils->findAncestors($node, $this->config->get('referenceFields'), $this->config->get('maxDepth')));

    // XXX: Handle a looping breadcrumb scenario by filtering the present
    // node out and then optionally re-adding it after if set to do so.
    $chain = array_filter($chain, function ($link) use ($nid) {
      return $link !== $nid;
    });
    if ($this->config->get('includeSelf')) {
      array_push($chain, $nid);
    }
    $breadcrumb->addCacheableDependency($node);

    // Add membership chain to the breadcrumb.
    foreach ($chain as $chainlink) {
      $node = $node_storage->load($chainlink);
      $breadcrumb->addCacheableDependency($node);
      $breadcrumb->addLink($node->toLink());
    }
    $breadcrumb->addCacheContexts(['route']);
    return $breadcrumb;
  }

}
