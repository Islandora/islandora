<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\field\Entity\FieldStorageConfig;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Condition for a node referenced by another node using the configured field.
 *
 * @Condition(
 *   id = "node_referenced_by_node",
 *   label = @Translation("Node is referenced by other nodes"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = TRUE , label = @Translation("node"))
 *   }
 * )
 */
class NodeReferencedByNode extends ConditionPluginBase implements ContainerFactoryPluginInterface {

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
      'reference_field' => 'field_member_of',
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
    $field_map = \Drupal::service('entity_field.manager')->getFieldMapByFieldType('entity_reference');
    $node_fields = array_keys($field_map['node']);
    $options = array_combine($node_fields, $node_fields);
    $form['reference_field'] = [
      '#type' => 'select',
      '#title' => $this->t('Field to check for reference to this node'),
      '#options' => $options,
      '#default_value' => $this->configuration['reference_field'],
      '#required' => TRUE,
      '#description' => $this->t("Machine name of field that should be checked for references to this node."),
    ];

    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->configuration['reference_field'] = $form_state->getValue('reference_field');
    parent::submitConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    $node = $this->getContextValue('node');
    if (!$node) {
      return FALSE;
    }
    return $this->evaluateEntity($node);
  }

  /**
   * Evaluates if an entity is referenced in the configured node field.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity to evaluate.
   *
   * @return bool
   *   TRUE if entity is referenced..
   */
  protected function evaluateEntity(EntityInterface $entity) {
    $reference_field = $this->configuration['reference_field'];
    $config = FieldStorageConfig::loadByName('node', $reference_field);
    if ($config) {
      $id_count = \Drupal::entityQuery('node')
        ->condition($reference_field, $entity->id())
        ->count()
        ->execute();
      return ($id_count > 0);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (!empty($this->configuration['negate'])) {
      return $this->t("The node is not referenced in another node's field `@field`.", ['@field' => $this->configuration['reference_field']]);
    }
    else {
      return $this->t("The node is referenced in another node's field `@field`.", ['@field' => $this->configuration['reference_field']]);
    }
  }

}
