<?php

namespace Drupal\islandora_breadcrumbs;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Breadcrumb\Breadcrumb;
use Drupal\Core\Breadcrumb\BreadcrumbBuilderInterface;
use Drupal\Core\Link;
use Drupal\Core\Routing\RouteMatchInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;

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
   * Storage to load nodes.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $nodeStorage;

  /**
   * Constructs a breadcrumb builder.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_manager
   *   Storage to load nodes.
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   The configuration factory.
   */
  public function __construct(EntityTypeManagerInterface $entity_manager, ConfigFactoryInterface $config_factory) {
    $this->nodeStorage = $entity_manager->getStorage('node');
    $this->config = $config_factory->get('islandora.breadcrumbs');
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
      $node = $this->nodeStorage->load($nid);
      return (!empty($node) && $node->hasField($this->config->get('referenceField')) && !$node->get($this->config->get('referenceField'))->isEmpty());
    }
  }

  /**
   * {@inheritdoc}
   */
  public function build(RouteMatchInterface $route_match) {

    $nid = $route_match->getRawParameters()->get('node');
    $node = $this->nodeStorage->load($nid);
    $breadcrumb = new Breadcrumb();
    $breadcrumb->addLink(Link::createFromRoute($this->t('Home'), '<front>'));

    $chain = [];
    $this->walkMembership($node, $chain);

    if (!$this->config->get('includeSelf')) {
      array_pop($chain);
    }
    $breadcrumb->addCacheableDependency($node);

    // Add membership chain to the breadcrumb.
    foreach ($chain as $chainlink) {
      $breadcrumb->addCacheableDependency($chainlink);
      $breadcrumb->addLink($chainlink->toLink());
    }
    $breadcrumb->addCacheContexts(['route']);
    return $breadcrumb;
  }

  /**
   * Follows chain of field_member_of links.
   *
   * We pass crumbs by reference to enable checking for looped chains.
   */
  protected function walkMembership(EntityInterface $entity, &$crumbs) {
    // Avoid infinate loops, return if we've seen this before.
    foreach ($crumbs as $crumb) {
      if ($crumb->uuid == $entity->uuid) {
        return;
      }
    }

    // Add this item onto the pile.
    array_unshift($crumbs, $entity);

    if ($this->config->get('maxDepth') > 0 && count($crumbs) >= $this->config->get('maxDepth')) {
      return;
    }

    // Find the next in the chain, if there are any.
    if ($entity->hasField($this->config->get('referenceField')) &&
      !$entity->get($this->config->get('referenceField'))->isEmpty()) {
      $this->walkMembership($entity->get($this->config->get('referenceField'))->entity, $crumbs);
    }
  }

}
