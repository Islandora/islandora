<?php

namespace Drupal\islandora\EventSubscriber;

use Drupal\Core\Access\AccessManagerInterface;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Routing\RouteMatchInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\Core\Entity\Exception\UndefinedLinkTemplateException;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;
use Symfony\Component\HttpFoundation\RequestStack;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\HttpKernel\Event\FilterResponseEvent;
use Symfony\Component\HttpKernel\KernelEvents;

/**
 * Abstract class to subscribe to LinkHeader Event.
 *
 * @package Drupal\islandora\EventSubscriber
 */
abstract class LinkHeaderSubscriber implements EventSubscriberInterface {

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
   * The access manager.
   *
   * @var \Drupal\Core\Access\AccessManagerInterface
   */
  protected $accessManager;

  /**
   * Current user.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $account;

  /**
   * The route match object.
   *
   * @var \Drupal\Core\Routing\RouteMatchInterface
   */
  protected $routeMatch;

  /**
   * Request stack (for current request).
   *
   * @var Symfony\Component\HttpFoundation\RequestStack
   */
  protected $requestStack;

  /**
   * Islandora utils.
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
   * @param \Drupal\Core\Access\AccessManagerInterface $access_manager
   *   The access manager.
   * @param \Drupal\Core\Session\AccountInterface $account
   *   The current user.
   * @param \Drupal\Core\Routing\RouteMatchInterface $route_match
   *   The route match object.
   * @param Symfony\Component\HttpFoundation\RequestStack $request_stack
   *   Request stack (for current request).
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   */
  public function __construct(
    EntityTypeManagerInterface $entity_type_manager,
    EntityFieldManagerInterface $entity_field_manager,
    AccessManagerInterface $access_manager,
    AccountInterface $account,
    RouteMatchInterface $route_match,
    RequestStack $request_stack,
    IslandoraUtils $utils
  ) {
    $this->entityTypeManager = $entity_type_manager;
    $this->entityFieldManager = $entity_field_manager;
    $this->accessManager = $access_manager;
    $this->account = $account;
    $this->routeMatch = $route_match;
    $this->accessManager = $access_manager;
    $this->account = $account;
    $this->requestStack = $request_stack;
    $this->utils = $utils;
  }

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents() {
    // Run this early so the headers get cached.
    $events[KernelEvents::RESPONSE][] = ['onResponse', 129];

    return $events;
  }

  /**
   * Get the Node | Media | File.
   *
   * @param \Symfony\Component\HttpFoundation\Response $response
   *   The current response object.
   * @param string $object_type
   *   The type of entity to look for.
   *
   * @return Drupal\Core\Entity\ContentEntityBase|bool
   *   A node or media entity or FALSE if we should skip out.
   */
  protected function getObject(Response $response, $object_type) {
    if ($object_type != 'node'
      && $object_type != 'media'
    ) {
      return FALSE;
    }

    // Exit early if the response is already cached.
    if ($response->headers->get('X-Drupal-Dynamic-Cache') == 'HIT') {
      return FALSE;
    }

    if (!$response->isOk()) {
      return FALSE;
    }

    // Hack the node out of the route.
    $route_object = $this->routeMatch->getRouteObject();
    if (!$route_object) {
      return FALSE;
    }

    $methods = $route_object->getMethods();
    $is_get = in_array('GET', $methods);
    $is_head = in_array('HEAD', $methods);
    if (!($is_get || $is_head)) {
      return FALSE;
    }

    $route_contexts = $route_object->getOption('parameters');
    if (!$route_contexts) {
      return FALSE;
    }
    if (!isset($route_contexts[$object_type])) {
      return FALSE;
    }

    $object = $this->routeMatch->getParameter($object_type);

    if (!$object) {
      return FALSE;
    }

    return $object;
  }

  /**
   * Generates link headers for each referenced entity.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   Entity that has reference fields.
   *
   * @return string[]
   *   Array of link headers
   */
  protected function generateEntityReferenceLinks(EntityInterface $entity) {
    // Use the node to add link headers for each entity reference.
    $entity_type = $entity->getEntityType()->id();
    $bundle = $entity->bundle();

    // Get all fields for the entity.
    $fields = $this->entityFieldManager->getFieldDefinitions($entity_type, $bundle);

    // Strip out everything but entity references that are not base fields.
    $entity_reference_fields = array_filter($fields, function ($field) {
      return $field->getFieldStorageDefinition()->isBaseField() == FALSE && $field->getType() == "entity_reference";
    });

    // Collect links for referenced entities.
    $links = [];
    foreach ($entity_reference_fields as $field_name => $field_definition) {
      foreach ($entity->get($field_name)->referencedEntities() as $referencedEntity) {
        // Headers are subject to an access check.
        if ($referencedEntity->access('view')) {

          try {
            $entity_url = $this->utils->getEntityUrl($referencedEntity);
          }
          catch (UndefinedLinkTemplateException $e) {
            // Not all referencable entities can generate canonical URLs, for
            // example: block entities.
            continue;
          }

          // Taxonomy terms are written out as
          // <url>; rel="tag"; title="Tag Name"
          // where url is defined in field_same_as.
          // If field_same_as doesn't exist or is empty,
          // it becomes the taxonomy term's local uri.
          if ($referencedEntity->getEntityTypeId() == 'taxonomy_term') {
            $rel = "tag";
            if ($referencedEntity->hasField(IslandoraUtils::EXTERNAL_URI_FIELD)) {
              $external_uri = $referencedEntity->get(IslandoraUtils::EXTERNAL_URI_FIELD)->getValue();
              if (!empty($external_uri) && isset($external_uri[0]['uri'])) {
                $entity_url = $external_uri[0]['uri'];
              }
            }
            $title = $referencedEntity->label();
          }
          else {
            // If it's not a taxonomy term, referenced entity link
            // headers take the form
            // <url>; rel="related"; title="Field Label"
            // and the url is the local uri.
            $rel = "related";
            $title = $field_definition->label();
          }
          $links[] = "<$entity_url>; rel=\"$rel\"; title=\"$title\"";
        }
      }
    }

    return $links;
  }

  /**
   * Generates link headers for REST endpoints.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   Entity that has reference fields.
   *
   * @return string[]
   *   Array of link headers
   */
  protected function generateRestLinks(EntityInterface $entity) {
    $rest_resource_config_storage = $this->entityTypeManager->getStorage('rest_resource_config');
    $entity_type = $entity->getEntityType()->id();
    $rest_resource_config = $rest_resource_config_storage->load("entity.$entity_type");

    $current_format = $this->requestStack->getCurrentRequest()->query->get('_format');

    $links = [];
    $route_name = $this->routeMatch->getRouteName();

    if ($rest_resource_config) {
      $formats = $rest_resource_config->getFormats("GET");

      foreach ($formats as $format) {
        if ($format == $current_format) {
          continue;
        }

        switch ($format) {
          case 'json':
            $mime = 'application/json';
            break;

          case 'jsonld':
            $mime = 'application/ld+json';
            break;

          case 'hal_json':
            $mime = 'application/hal+json';
            break;

          case 'xml':
            $mime = 'application/xml';
            break;

          default:
            continue 2;
        }

        // Skip route if the user doesn't have access.
        $meta_route_name = "rest.entity.$entity_type.GET";
        $route_params = [$entity_type => $entity->id()];
        if (!$this->accessManager->checkNamedRoute($meta_route_name, $route_params, $this->account)) {
          continue;
        }

        $meta_url = $this->utils->getRestUrl($entity, $format);

        $links[] = "<$meta_url>; rel=\"alternate\"; type=\"$mime\"";
      }
    }

    return $links;
  }

  /**
   * Adds resource-specific link headers to appropriate responses.
   *
   * @param \Symfony\Component\HttpKernel\Event\FilterResponseEvent $event
   *   Event containing the response.
   */
  abstract public function onResponse(FilterResponseEvent $event);

}
