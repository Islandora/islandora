<?php

namespace Drupal\islandora\Controller;

use Drupal\islandora\IslandoraUtils;
use Drupal\Core\Access\AccessResult;
use Drupal\Core\Routing\RouteMatch;
use Drupal\node\Entity\Node;
use Drupal\Core\Url;
use Drupal\node\NodeInterface;

/**
 * Page to select new media type to add.
 */
class ManageMediaController extends ManageMembersController {

  /**
   * Renders a list of media types to add.
   *
   * @param \Drupal\node\NodeInterface $node
   *   Node you want to add a media to.
   *
   * @return array
   *   Array of media types to add.
   */
  public function addToNodePage(NodeInterface $node) {
    $field = IslandoraUtils::MEDIA_OF_FIELD;

    $add_media_list = $this->generateTypeList(
      'media',
      'media_type',
      'entity.media.add_form',
      'entity.media_type.add_form',
      $field,
      ['query' => ["edit[$field][widget][0][target_id]" => $node->id()]]
    );

    $manage_link = Url::fromRoute('entity.media_type.collection')->toRenderArray();
    $manage_link['#title'] = $this->t('Manage media types');
    $manage_link['#type'] = 'link';
    $manage_link['#prefix'] = ' ';
    $manage_link['#suffix'] = '.';

    return [
      '#type' => 'markup',
      '#markup' => $this->t("The following media types can be added because they have the <code>@field</code> field.", [
        '@field' => $field,
      ]),
      'manage_link' => $manage_link,
      'add_media' => $add_media_list,
    ];
  }

  /**
   * Check if the object being displayed "is Islandora".
   *
   * @param \Drupal\Core\Routing\RouteMatch $route_match
   *   The current routing match.
   *
   * @return \Drupal\Core\Access\AccessResultAllowed|\Drupal\Core\Access\AccessResultForbidden
   *   Whether we can or can't show the "thing".
   */
  public function access(RouteMatch $route_match) {
    // Route match is being used as opposed to slugs as there are a few
    // admin routes being altered.
    // @see: \Drupal\islandora\EventSubscriber\AdminViewsRouteSubscriber::alterRoutes().
    if ($route_match->getParameters()->has('node')) {
      $node = $route_match->getParameter('node');
      if (!$node instanceof NodeInterface) {
        $node = Node::load($node);
      }
      // Ensure there's actually a node before referencing it.
      if ($node) {
        if ($this->utils->isIslandoraType($node->getEntityTypeId(), $node->bundle())) {
          return AccessResult::allowed();
        }
      }
    }
    return AccessResult::forbidden();
  }

}
