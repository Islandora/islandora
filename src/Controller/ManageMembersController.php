<?php

namespace Drupal\islandora\Controller;

use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Render\RendererInterface;
use Drupal\Core\Entity\Controller\EntityController;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Link;
use Drupal\Core\Url;
use Drupal\islandora\IslandoraUtils;
use Drupal\node\NodeInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Page to select new type to add as member.
 */
class ManageMembersController extends EntityController {

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * The entity field manager.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface
   */
  protected $entityFieldManager;

  /**
   * The renderer.
   *
   * @var \Drupal\Core\Render\RendererInterface
   */
  protected $renderer;

  /**
   * Islandora Utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Constructor.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $entity_field_manager
   *   The entity field manager.
   * @param \Drupal\Core\Render\RendererInterface $renderer
   *   The renderer.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   */
  public function __construct(
    EntityTypeManagerInterface $entity_type_manager,
    EntityFieldManagerInterface $entity_field_manager,
    RendererInterface $renderer,
    IslandoraUtils $utils
  ) {
    $this->entityTypeManager = $entity_type_manager;
    $this->entityFieldManager = $entity_field_manager;
    $this->renderer = $renderer;
    $this->utils = $utils;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity_type.manager'),
      $container->get('entity_field.manager'),
      $container->get('renderer'),
      $container->get('islandora.utils')
    );
  }

  /**
   * Renders a list of types to add as members.
   *
   * @param \Drupal\node\NodeInterface $node
   *   Node you want to add a member to.
   */
  public function addToNodePage(NodeInterface $node) {
    $field = IslandoraUtils::MEMBER_OF_FIELD;

    $add_node_list = $this->generateTypeList(
      'node',
      'node_type',
      'node.add',
      'node.type_add',
      $field,
      ['query' => ["edit[$field][widget][0][target_id]" => $node->id()]]
    );

    $manage_link = Url::fromRoute('entity.node_type.collection')->toRenderArray();
    $manage_link['#title'] = $this->t('Manage content types');
    $manage_link['#type'] = 'link';
    $manage_link['#prefix'] = ' ';
    $manage_link['#suffix'] = '.';

    return [
      '#type' => 'markup',
      '#markup' => $this->t("The following content types can be added because they have the <code>@field</code> field.", [
        '@field' => $field,
      ]),
      'manage_link' => $manage_link,
      'add_node' => $add_node_list,
    ];
  }

  /**
   * Renders a list of content types to add as members.
   */
  protected function generateTypeList($entity_type, $bundle_type, $entity_add_form, $bundle_add_form, $field, array $url_options) {
    $type_definition = $this->entityTypeManager->getDefinition($bundle_type);

    $build = [
      '#theme' => 'entity_add_list',
      '#bundles' => [],
      '#cache' => ['tags' => $type_definition->getListCacheTags()],
    ];

    $bundles = $this->entityTypeManager->getStorage($bundle_type)->loadMultiple();
    $access_control_handler = $this->entityTypeManager->getAccessControlHandler($entity_type);

    foreach (array_keys($bundles) as $bundle_id) {
      $bundle = $bundles[$bundle_id];

      // Skip bundles that don't have the specified field.
      $fields = $this->entityFieldManager->getFieldDefinitions($entity_type, $bundle_id);
      if (!isset($fields[$field])) {
        continue;
      }

      $build['#bundles'][$bundle_id] = [
        'label' => $bundle->label(),
        'description' => $bundle->getDescription(),
        'add_link' => Link::createFromRoute(
          $bundle->label(),
          $entity_add_form,
          [$bundle_type => $bundle->id()],
          $url_options
        ),
      ];
    }

    // Filter out bundles the user can't create.
    foreach (array_keys($bundles) as $bundle_id) {
      $access = $access_control_handler->createAccess($bundle_id, NULL, [], TRUE);
      if (!$access->isAllowed()) {
        unset($build['#bundles'][$bundle_id]);
      }
      $this->renderer->addCacheableDependency($build, $access);
    }

    // Build the message shown when there are no bundles.
    $type_label = $type_definition->getSingularLabel();
    $link_text = $this->t('Add a new @entity_type.', ['@entity_type' => $type_label]);
    $build['#add_bundle_message'] = $this->t('There is no @entity_type yet. @add_link', [
      '@entity_type' => $type_label,
      '@add_link' => Link::createFromRoute($link_text, $bundle_add_form)->toString(),
    ]);

    return $build;
  }

}
