<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a condition to detect node's parent.
 *
 * @Condition(
 *   id = "node_has_parent",
 *   label = @Translation("Node has parent"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = TRUE , label = @Translation("node"))
 *   }
 * )
 */
class NodeHasParent extends ConditionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * Node storage.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

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
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   Entity type manager.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    EntityTypeManagerInterface $entity_type_manager
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->entityTypeManager = $entity_type_manager;
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return parent::defaultConfiguration() + [
      'parent_reference_field' => 'field_member_of',
      'parent_nid' => '',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('entity_type.manager')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form['parent_nid'] = [
      '#type' => 'entity_autocomplete',
      '#title' => $this->t('Parent node'),
      '#default_value' => $this->entityTypeManager->getStorage('node')->load($this->configuration['parent_nid']),
      '#description' => $this->t("Can be a collection node or a compound object."),
      '#target_type' => 'node',
    ];
    $field_map = \Drupal::service('entity_field.manager')->getFieldMapByFieldType('entity_reference');
    $node_fields = array_keys($field_map['node']);
    $options = array_combine($node_fields, $node_fields);
    $form['parent_reference_field'] = [
      '#type' => 'select',
      '#title' => $this->t('Field that contains reference to parents'),
      '#options' => $options,
      '#default_value' => $this->configuration['parent_reference_field'],
      '#required' => TRUE,
      '#description' => $this->t("Machine name of field that contains references to parent node."),
    ];

    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->configuration['parent_nid'] = $form_state->getValue('parent_nid');
    $this->configuration['parent_reference_field'] = $form_state->getValue('parent_reference_field');
    parent::submitConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    if (empty($this->configuration['parent_nid']) && !$this->isNegated()) {
      return TRUE;
    }

    $node = $this->getContextValue('node');
    if (!$node) {
      return FALSE;
    }
    return $this->evaluateEntity($node);
  }

  /**
   * Evaluates if an entity has the specified node as its parent.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity to evalute.
   *
   * @return bool
   *   TRUE if entity references the specified parent.
   */
  protected function evaluateEntity(EntityInterface $entity) {
    $parent_reference_field = $this->configuration['parent_reference_field'];
    foreach ($entity->referencedEntities() as $referenced_entity) {
      // Check whether the entity and the referenced entity are nodes.
      // Also make sure that the field exists.
      if ($entity->getEntityTypeID() == 'node' && $entity->hasField($parent_reference_field) && $referenced_entity->getEntityTypeId() == 'node') {
        $field = $entity->get($parent_reference_field);
        if (!$field->isEmpty()) {
          $nids = $field->getValue();
          foreach ($nids as $nid) {
            if ($nid['target_id'] == $this->configuration['parent_nid']) {
              return TRUE;
            }
          }
        }
      }
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (!empty($this->configuration['negate'])) {
      return $this->t('The node does not have node @nid as its parent.', ['@nid' => $this->configuration['parent_nid']]);
    }
    else {
      return $this->t('The node has node @nid as its parent.', ['@nid' => $this->configuration['parent_nid']]);
    }
  }

}
