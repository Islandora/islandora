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
        'uri' => NULL,
      ],
      parent::defaultConfiguration()
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $default = [];
    if (isset($this->configuration['uri']) && !empty($this->configuration['uri'])) {
      $uris = explode(',', $this->configuration['uri']);
      foreach ($uris as $uri) {
        $default[] = $this->utils->getTermForUri($uri);
      }
    }

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
      '#default_value' => $this->configuration['logic'],
    ];
    return parent::buildConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    // Set URI for term if possible.
    $this->configuration['uri'] = NULL;
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
      if (!empty($uris)) {
        $this->configuration['uri'] = implode(',', $uris);
      }
    }

    $this->configuration['logic'] = $form_state->getValue('logic');

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
  protected function evaluateEntity(EntityInterface $entity) {
    // Find the terms on the node.
    $field_names = $this->utils->getUriFieldNamesForTerms();
    $terms = array_filter($entity->referencedEntities(), function ($entity) use ($field_names) {
      if ($entity->getEntityTypeId() != 'taxonomy_term') {
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
    $haystack = array_map(function ($term) {
        return $this->utils->getUriForTerm($term);
    },
      $terms
    );

    // FALSE if there's no URIs on the node.
    if (empty($haystack)) {
      return FALSE;
    }

    // Get the URIs to look for.  It's a required field, so there
    // will always be one.
    $needles = explode(',', $this->configuration['uri']);

    // TRUE if every needle is in the haystack.
    if ($this->configuration['logic'] == 'and') {
      if (count(array_intersect($needles, $haystack)) == count($needles)) {
        return TRUE;
      }
      return FALSE;
    }
    // TRUE if any needle is in the haystack.
    else {
      if (count(array_intersect($needles, $haystack)) > 0) {
        return TRUE;
      }
      return FALSE;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (!empty($this->configuration['negate'])) {
      return $this->t('The node is not associated with taxonomy term with uri @uri.', ['@uri' => $this->configuration['uri']]);
    }
    else {
      return $this->t('The node is associated with taxonomy term with uri @uri.', ['@uri' => $this->configuration['uri']]);
    }
  }

}
