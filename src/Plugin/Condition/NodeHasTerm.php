<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Entity\FieldableEntityInterface;
use Drupal\Core\Entity\Plugin\DataType\EntityReference;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a 'Term' condition for nodes.
 *
 * @Condition(
 *   id = "node_has_term",
 *   label = @Translation("Node has term with URI"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = TRUE , label = @Translation("node"))
 *   }
 * )
 */
class NodeHasTerm extends ConditionPluginBase implements ContainerFactoryPluginInterface {

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
   * URIs for which to test.
   *
   * @var string[]
   */
  protected array $uris;

  /**
   * Operand with which to combine matches.
   *
   * The string "and" or "or":
   * - if "and"; all $uris must be matched
   * - if "or"; only one of $uris much be matched.
   *
   * @var string
   */
  protected string $operand;

  /**
   * Flag, for how to enumerate candidate entities which might bear URIs.
   *
   * TRUE to use ::referencedEntities(); however, can be very inefficient with
   * many other referenced entities (such as paragraphs). FALSE to constrain
   * the fields considered to entity references fields bearing taxonomy terms
   * earlier.
   *
   * @var bool
   */
  protected bool $naiveReferences;

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
    EntityTypeManagerInterface $entity_type_manager
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->utils = $utils;
    $this->entityTypeManager = $entity_type_manager;
  }

  /**
   * Helper; unpack configuration to our member variables.
   */
  private function unpackConfig() : static {
    $this->uris = explode(',', $this->configuration['uri']);
    $this->operand = $this->configuration['logic'];
    $this->naiveReferences = $this->configuration['naive_references'];
    return $this;
  }

  /**
   * {@inheritDoc}
   */
  public function setConfiguration(array $configuration) {
    return parent::setConfiguration($configuration)
      ->unpackConfig();
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
  public function defaultConfiguration() {
    return array_merge(
      [
        'logic' => 'and',
        'uri' => '',
        'naive_references' => FALSE,
      ],
      parent::defaultConfiguration()
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $default = array_filter(array_map($this->utils->getTermForUri(...), $this->uris));

    $form['term'] = [
      '#type' => 'entity_autocomplete',
      '#title' => $this->t('Term'),
      '#description' => $this->t('Only terms that have external URIs/URLs will appear here.'),
      '#tags' => TRUE,
      '#default_value' => $default,
      '#target_type' => 'taxonomy_term',
      '#selection_handler' => 'islandora:external_uri',
    ];

    $form['logic'] = [
      '#type' => 'radios',
      '#title' => $this->t('Logic'),
      '#description' => $this->t('Whether to use AND or OR logic to evaluate multiple terms'),
      '#options' => [
        'and' => 'And',
        'or' => 'Or',
      ],
      '#default_value' => $this->operand,
    ];

    $form['naive_references'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Naive References'),
      '#description' => $this->t('Use naive reference enumeration. Uncheck for better performance when dealing with sufficiently complex node content definitions containing many entity reference fields (including paragraphs, dgi_image_discovery, etc.).'),

      '#default_value' => $this->naiveReferences,
    ];

    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    // Set URI for term if possible.
    $value = $form_state->getValue('term');
    $uris = [];
    if (!empty($value)) {
      foreach ($value as $target) {
        $tid = $target['target_id'];
        $term = $this->entityTypeManager->getStorage('taxonomy_term')->load($tid);
        $uri = $this->utils->getUriForTerm($term);
        if ($uri) {
          $uris[] = $uri;
        }
      }
    }

    $this->configuration['uri'] = implode(',', $uris);
    $this->configuration['logic'] = $form_state->getValue('logic');
    $this->configuration['naive_references'] = $form_state->getValue('naive_references');

    // XXX: Call to the parent has to be last, due to how the context definition
    // is added.
    parent::submitConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    if (empty($this->configuration['uri']) && !$this->isNegated()) {
      return TRUE;
    }

    $node = $this->getContextValue('node');
    if (!$node) {
      return FALSE;
    }
    return $this->evaluateEntity($node);
  }

  /**
   * Evaluates if an entity has the specified term(s).
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity to evalute.
   *
   * @return bool
   *   TRUE if entity has all the specified term(s), otherwise FALSE.
   */
  protected function evaluateEntity(EntityInterface $entity) : bool {
    // Find the terms on the node.
    $field_names = $this->utils->getUriFieldNamesForTerms();
    $unfiltered_terms = ($this->naiveReferences) ?
      $entity->referencedEntities() :
      $this->doSpecificReferenceLookup($entity);
    /** @var \Drupal\taxonomy\TermInterface[] $terms */
    $terms = array_filter($unfiltered_terms, static function ($entity) use ($field_names) {
      if ($entity->getEntityTypeId() !== 'taxonomy_term') {
        return FALSE;
      }
      if (!($entity instanceof FieldableEntityInterface)) {
        return FALSE;
      }

      foreach ($field_names as $field_name) {
        if ($entity->hasField($field_name) && !$entity->get($field_name)->isEmpty()) {
          return TRUE;
        }
      }
      return FALSE;
    });

    // Get their URIs.
    $haystack = array_filter(array_map($this->utils->getUriForTerm(...), $terms));

    // FALSE if there's no URIs on the node.
    if (empty($haystack)) {
      return FALSE;
    }

    return match ($this->operand) {
      'and' => count(array_intersect($this->uris, $haystack)) === count($this->uris),
      default => count(array_intersect($this->uris, $haystack)) > 0,
    };
  }

  /**
   * More targetedly discover references to the taxonomy fields we care about.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity from which to examine.
   *
   * @return \Drupal\taxonomy\TermInterface[]
   *   Taxonomy terms to check examine.
   *
   * @throws \Drupal\Core\TypedData\Exception\MissingDataException
   */
  private function doSpecificReferenceLookup(EntityInterface $entity) : array {
    assert($entity instanceof ContentEntityInterface);
    $field_generator = static function (FieldableEntityInterface $entity) {
      foreach ($entity->getFieldDefinitions() as $field_name => $field_definition) {
        if ($field_definition->getType() !== 'entity_reference' || $field_definition->getSetting('target_type') !== 'taxonomy_term') {
          continue;
        }
        yield $field_name;
      }
    };

    $terms = [];

    /** @var string $field_name */
    foreach ($field_generator($entity) as $field_name) {
      /** @var \Drupal\Core\Field\FieldItemInterface $field_item */
      foreach ($entity->get($field_name) as $field_item) {
        foreach ($field_item->getProperties(TRUE) as $property) {
          if ($property instanceof EntityReference &&
            $property->getTargetDefinition()->getEntityTypeId() === 'taxonomy_term' &&
            ($term = $property->getValue())) {
            $terms[] = $term;
          }
        }
      }
    }

    return $terms;
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    $context = [
      '@uri' => implode(',', $this->uris),
    ];
    return !empty($this->configuration['negate']) ?
      $this->t('The node is not associated with taxonomy term with uri @uri.', $context) :
      $this->t('The node is associated with taxonomy term with uri @uri.', $context);
  }

}
