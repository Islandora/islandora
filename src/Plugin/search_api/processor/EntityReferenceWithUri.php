<?php

namespace Drupal\islandora\Plugin\search_api\processor;

use Drupal\islandora\Plugin\search_api\processor\Property\EntityReferenceWithUriProperty;
use Drupal\search_api\Datasource\DatasourceInterface;
use Drupal\search_api\Item\ItemInterface;
use Drupal\search_api\Processor\ProcessorPluginBase;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Add a filterable search api field for entity reference fields.
 *
 * @SearchApiProcessor(
 *   id = "entity_reference_with_term",
 *   label = @Translation("Entity Reference, where the target has a given taxonomy term"),
 *   description = @Translation("Indexes the titles of Entity Reference field targets, where the targets have a taxonomy term or terms. Does not apply to Taxonomy Reference fields."),
 *   stages = {
 *     "add_properties" = 0,
 *   },
 *   locked = false,
 *   hidden = false,
 * )
 */
class EntityReferenceWithUri extends ProcessorPluginBase {

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
   *   by configuration option name. The special key 'context' may be used to
   *   initialize the defined contexts by setting it to an array of context
   *   values keyed by context names.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   */
  public function __construct(
    array $configuration,
          $plugin_id,
          $plugin_definition,
    IslandoraUtils $utils
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
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
      $container->get('islandora.utils'),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getPropertyDefinitions(DatasourceInterface $datasource = NULL) {
    $properties = [];
    if (!$datasource || !$datasource->getEntityTypeId()) {
      return $properties;
    }

    $entity_type = $datasource->getEntityTypeId();

    // Get all configured Entity Relation fields for this entity type.
    $fields = \Drupal::entityTypeManager()->getStorage('field_config')->loadByProperties([
      'entity_type' => $entity_type,
      'field_type' => 'entity_reference',
    ]);

    foreach ($fields as $field) {
      // Skip over fields that point to taxonomy term fields themselves.
      if ($field->getSetting('target_type') == 'taxonomy_term') {
        continue;
      }

      $definition = [
        'label' => $this->t('@label (by term URI) [@bundle]', [
          '@label' => $field->label(),
          '@bundle' => $field->getTargetBundle(),
        ]),
        'description' => $this->t('Index the target entity, but only if the target entity has a taxonomy term.'),
        'type' => 'string',
        'processor_id' => $this->getPluginId(),
        'settings' => [
          'filter_terms' => [],
          'target_type' => $field->getSetting('target_type'),
        ],
        'is_list' => TRUE,
      ];
      $fieldname = 'entity_reference_with_term__' . str_replace('.', '__', $field->id());
      $properties[$fieldname] = new EntityReferenceWithUriProperty($definition);
    }
    return $properties;
  }

  /**
   * {@inheritdoc}
   */
  public function addFieldValues(ItemInterface $item) {
    // Skip if no Entity Reference with URI fields are configured.
    $relevant_search_api_fields = [];
    $search_api_fields = $item->getFields(FALSE);
    foreach ($search_api_fields as $field) {
      if (substr($field->getPropertyPath(), 0, 28) == 'entity_reference_with_term__') {
        $relevant_search_api_fields[] = $field;
      }
    }
    if (empty($relevant_search_api_fields)) {
      return;
    }

    $content_entity = $item->getOriginalObject()->getValue();
    $type_and_bundle_prefix = $content_entity->getEntityTypeId() . '__' . $content_entity->bundle() . '__';
    foreach ($relevant_search_api_fields as $search_api_field) {
      $entity_type = $search_api_field->getConfiguration()['target_type'];
      $filter_terms = $search_api_field->getConfiguration()['filter_terms'];
      $filter_tids = array_map(function ($element) {
        return $element['target_id'];
      }, $filter_terms);
      $require_all = $search_api_field->getConfiguration()['require_all'];
      $field_name = substr($search_api_field->getPropertyPath(), 28);
      $field_name = str_replace($type_and_bundle_prefix, '', $field_name);
      if ($content_entity->hasField($field_name)) {
        $referenced_terms = [];
        foreach ($content_entity->get($field_name)->getValue() as $values) {
          foreach ($values as $value) {
            // Load the entity stored in our field.
            $target_entity = \Drupal::entityTypeManager()
              ->getStorage($entity_type)
              ->load($value);
            // Load the taxonomy terms on the entity stored in our field.
            $referenced_terms = array_merge($referenced_terms, array_filter($target_entity->referencedEntities(), function ($entity) {
              if ($entity->getEntityTypeId() != 'taxonomy_term') {
                return FALSE;
              }
              else {
                return TRUE;
              }
            }));
          }

          $referenced_tids = array_map(function ($element) {
            return $element->id();
          },
            $referenced_terms
          );
          if ($require_all) {
            if (count(array_intersect($referenced_tids, $filter_tids)) == count($filter_tids)) {
              // "All" and all the terms specified are present.
              $label = $target_entity->label();
              $search_api_field->addValue($label);
            }
          }
          else {
            if (count(array_intersect($referenced_tids, $filter_tids)) > 0) {
              // "Any" and at least one term specified is present.
              $label = $target_entity->label();
              $search_api_field->addValue($label);
            }
          }
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function requiresReindexing(array $old_settings = NULL, array $new_settings = NULL) {
    if ($new_settings != $old_settings) {
      return TRUE;
    }
    return FALSE;
  }

}
