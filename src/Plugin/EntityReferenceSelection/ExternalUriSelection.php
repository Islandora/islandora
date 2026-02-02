<?php

namespace Drupal\islandora\Plugin\EntityReferenceSelection;

use Drupal\Core\Entity\Attribute\EntityReferenceSelection;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityRepositoryInterface;
use Drupal\Core\Entity\EntityTypeBundleInfoInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\islandora\IslandoraUtils;
use Drupal\taxonomy\Plugin\EntityReferenceSelection\TermSelection;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Filters by looking for entities with Authority Links or External Uris.
 */
#[EntityReferenceSelection(
  id: "islandora:external_uri",
  label: new TranslatableMarkup("Taxonomy Term with external URI selection"),
  entity_types: ["taxonomy_term"],
  group: "islandora",
  weight: 1
)]
class ExternalUriSelection extends TermSelection {

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Constructs a new ExternalUriSelection object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity manager service.
   * @param \Drupal\Core\Extension\ModuleHandlerInterface $module_handler
   *   The module handler service.
   * @param \Drupal\Core\Session\AccountInterface $current_user
   *   The current user.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $entity_field_manager
   *   The entity field manager.
   * @param \Drupal\Core\Entity\EntityTypeBundleInfoInterface $entity_type_bundle_info
   *   The entity type bundle info service.
   * @param \Drupal\Core\Entity\EntityRepositoryInterface $entity_repository
   *   The entity repository.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    EntityTypeManagerInterface $entity_type_manager,
    ModuleHandlerInterface $module_handler,
    AccountInterface $current_user,
    IslandoraUtils $utils,
    ?EntityFieldManagerInterface $entity_field_manager = NULL,
    ?EntityTypeBundleInfoInterface $entity_type_bundle_info = NULL,
    ?EntityRepositoryInterface $entity_repository = NULL,
  ) {
    parent::__construct(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $entity_type_manager,
      $module_handler,
      $current_user,
      $entity_field_manager,
      $entity_type_bundle_info,
      $entity_repository
    );
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
      $container->get('entity_type.manager'),
      $container->get('module_handler'),
      $container->get('current_user'),
      $container->get('islandora.utils'),
      $container->get('entity_field.manager'),
      $container->get('entity_type.bundle.info'),
      $container->get('entity.repository')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getReferenceableEntities($match = NULL, $match_operator = 'CONTAINS', $limit = 0) {
    $options = parent::getReferenceableEntities($match, $match_operator, $limit);

    foreach (array_keys($options) as $vid) {
      foreach (array_keys($options[$vid]) as $tid) {
        $term = $this->entityTypeManager->getStorage('taxonomy_term')->load($tid);
        $uri = $this->utils->getUriForTerm($term);
        if (empty($uri)) {
          unset($options[$vid][$tid]);
        }
      }
      if (empty($options[$vid])) {
        unset($options[$vid]);
      }
    }

    return $options;
  }

}
