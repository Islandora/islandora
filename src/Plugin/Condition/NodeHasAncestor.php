<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Condition to see if a node has an ancestor.
 *
 * @Condition(
 *   id = "node_has_ancestor",
 *   label = @Translation("Node has ancestor"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = TRUE , label = @Translation("node"))
 *   }
 * )
 */
class NodeHasAncestor extends ConditionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * Drupal's entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected EntityTypeManagerInterface $entityTypeManager;

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected IslandoraUtils $utils;

  /**
   * Constructor for the ancestor condition.
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
   *   The Drupal entity type manager.
   * @param \Drupal\islandora\IslandoraUtils $islandora_utils
   *   Islandora utils service.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, EntityTypeManagerInterface $entity_type_manager, IslandoraUtils $islandora_utils) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->entityTypeManager = $entity_type_manager;
    $this->utils = $islandora_utils;
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
      $container->get('islandora.utils')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration(): array {
    return parent::defaultConfiguration() + [
      'ancestor_nids' => FALSE,
      'parent_reference_field' => IslandoraUtils::MEMBER_OF_FIELD,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $default_nids = FALSE;
    if ($this->configuration['ancestor_nids']) {
      $default_nids = array_map(function ($nid) {
        return $this->entityTypeManager->getStorage('node')->load($nid);
      }, $this->configuration['ancestor_nids']);
    }
    $form['ancestor_nids'] = [
      '#type' => 'entity_autocomplete',
      '#title' => $this->t('Parent node(s)'),
      '#default_value' => $default_nids,
      '#required' => TRUE,
      '#description' => $this->t("Can be a collection node, compound object or paged content. Accepts multiple values separated by a comma."),
      '#target_type' => 'node',
      '#tags' => TRUE,
    ];

    $options = [];
    $reference_fields = $this->entityTypeManager->getStorage('field_storage_config')->loadByProperties([
      'type' => 'entity_reference',
      'settings' => [
        'target_type' => 'node',
      ],
    ]);
    foreach ($reference_fields as $field) {
      $options[$field->get('field_name')] = $field->get('field_name');
    }
    $form['parent_reference_field'] = [
      '#type' => 'select',
      '#title' => $this->t('Direct parent reference'),
      '#options' => $options,
      '#default_value' => $this->configuration['parent_reference_field'],
      '#required' => TRUE,
      '#description' => $this->t('Field that contains the reference to its parent node.'),
    ];

    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    // Entity autocomplete store things with target IDs, for convenience just
    // store the plain nid.
    $this->configuration['ancestor_nids'] = array_map(function ($nid) {
      return $nid['target_id'];
    }, $form_state->getValue('ancestor_nids'));
    $this->configuration['parent_reference_field'] = $form_state->getValue('parent_reference_field');
    parent::submitConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    if (empty($this->configuration['ancestor_nids']) && !$this->isNegated()) {
      return TRUE;
    }

    $node = $this->getContextValue('node');
    if (!$node) {
      return FALSE;
    }

    $ancestors = $this->utils->findAncestors($node);
    return !empty(array_intersect($this->configuration['ancestor_nids'], $ancestors));
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (!empty($this->configuration['negate'])) {
      return $this->t('The node does not have node @nid as one of its ancestors.', ['@nid' => $this->configuration['ancestor_nids']]);
    }
    else {
      return $this->t('The node has node @nid as one of its ancestors.', ['@nid' => $this->configuration['ancestor_nids']]);
    }
  }

}
