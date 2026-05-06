<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a Islandora 7.x namespace condition for nodes.
 *
 * @Condition(
 *   id = "node_had_namespace",
 *   label = @Translation("Node had 7.x namespace"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = TRUE , label = @Translation("node"))
 *   }
 * )
 */
class NodeHadNamespace extends ConditionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Term storage.
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
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   Entity type manager.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    IslandoraUtils $utils,
    EntityTypeManagerInterface $entity_type_manager,
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->utils = $utils;
    $this->entityTypeManager = $entity_type_manager;
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
      $container->get('entity_type.manager')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form['namespace'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Islandora 7.x Namespaces'),
      '#description' => $this->t('Comma-delimited list of 7.x PID namespaces, including the trailing colon (e.g., "islandora:,ir:").'),
      '#default_value' => $this->configuration['namespace'],
      '#maxlength' => 256,
    ];
    $field_map = \Drupal::service('entity_field.manager')->getFieldMapByFieldType('string');
    $node_fields = array_keys($field_map['node']);
    $options = array_combine($node_fields, $node_fields);
    $form['pid_field'] = [
      '#type' => 'select',
      '#title' => $this->t('Field that contains the PID'),
      '#options' => $options,
      '#default_value' => $this->configuration['pid_field'],
      '#required' => TRUE,
      '#description' => $this->t("Machine name of the field that contains the PID."),
    ];

    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->configuration['namespace'] = NULL;
    $namespace = $form_state->getValue('namespace');
    if (!empty($namespace)) {
      if ($namespace) {
        $this->configuration['namespace'] = $namespace;
      }
    }
    $this->configuration['pid_field'] = $form_state->getValue('pid_field');
    parent::submitConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    if (empty($this->configuration['namespace']) && !$this->isNegated()) {
      return TRUE;
    }

    $node = $this->getContextValue('node');
    if (!$node) {
      return FALSE;
    }
    return $this->evaluateEntity($node);
  }

  /**
   * Evaluates if the value of field_pid with a registered 7.x namespace.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity to evalute.
   *
   * @return bool
   *   TRUE if entity has the specified namespace, otherwise FALSE.
   */
  protected function evaluateEntity(EntityInterface $entity) {
    $pid_field = $this->configuration['pid_field'];
    if ($entity->hasField($pid_field)) {
      $pid_value = $entity->get($pid_field)->getValue();
      $pid = $pid_value[0]['value'];
      $namespace = strtok($pid, ':') . ':';
      $registered_namespaces = explode(',', $this->configuration['namespace']);
      foreach ($registered_namespaces as &$registered_namespace) {
        $registered_namespace = trim($registered_namespace);
        if (in_array($namespace, $registered_namespaces)) {
          return TRUE;
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
      return $this->t('The node does not have a value in its PID field with the namespace @namespace.', ['@namespace' => $this->configuration['namespace']]);
    }
    else {
      return $this->t('The node has a value in its PID field with the namespace @namespace.', ['@namespace' => $this->configuration['namespace']]);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return array_merge(
      ['namespace' => '', 'pid_field' => 'field_pid'],
      parent::defaultConfiguration()
    );
  }

}
