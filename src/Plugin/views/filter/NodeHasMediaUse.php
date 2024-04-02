<?php

namespace Drupal\islandora\Plugin\views\filter;

use Drupal\Core\Database\Connection;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\views\Plugin\views\filter\FilterPluginBase;
use Psr\Log\LoggerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Views Filter on Having Media of a Type.
 *
 * @ingroup views_field_handlers
 *
 * @ViewsFilter("islandora_node_has_media_use")
 */
class NodeHasMediaUse extends FilterPluginBase {

  /**
   * Islandora's utility service.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected IslandoraUtils $utils;

  /**
   * Drupal's entity type manager service.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected EntityTypeManagerInterface $entityTypeManager;

  /**
   * Drupal's database connection service.
   *
   * @var \Drupal\Core\Database\Connection
   */
  protected Connection $connection;

  /**
   * Logger service.
   *
   * @var \Psr\Log\LoggerInterface
   */
  protected LoggerInterface $logger;

  /**
   * {@inheritDoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    $instance = parent::create($container, $configuration, $plugin_id, $plugin_definition);

    $instance->utils = $container->get('islandora.utils');
    $instance->entityTypeManager = $container->get('entity_type.manager');
    $instance->connection = $container->get('database');
    $instance->logger = $container->get('logger.factory')->get('islanodra');

    return $instance;
  }

  /**
   * {@inheritdoc}
   */
  protected function defineOptions() {
    $options = parent::defineOptions();
    $options['use_uri'] = ['default' => NULL];
    $options['negated'] = ['default' => FALSE];
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function validateOptionsForm(&$form, FormStateInterface $form_state) {
    $uri = $form_state->getValues()['options']['use_uri'];
    $term = $this->utils->getTermForUri($uri);
    if (empty($term)) {
      $form_state->setError($form['use_uri'], $this->t('Could not find term with URI: "%uri"', ['%uri' => $uri]));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function buildOptionsForm(&$form, FormStateInterface $form_state) {
    $terms = $this->entityTypeManager->getStorage('taxonomy_term')->loadByProperties(['vid' => 'islandora_media_use']);
    $uris = [];
    foreach ($terms as $term) {
      foreach ($term->get('field_external_uri')->getValue() as $uri) {
        $uris[$uri['uri']] = $term->label();
      }
    }

    $form['use_uri'] = [
      '#type' => 'select',
      '#title' => "Media Use Term",
      '#options' => $uris,
      '#default_value' => $this->options['use_uri'],
      '#required' => TRUE,
    ];
    $form['negated'] = [
      '#type' => 'checkbox',
      '#title' => 'Negated',
      '#description' => $this->t("Return nodes that <em>don't</em> have this use URI"),
      '#default_value' => $this->options['negated'],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function adminSummary() {
    $operator = ($this->options['negated']) ? "does not have" : "has";
    $term = $this->utils->getTermForUri($this->options['use_uri']);
    $label = (empty($term)) ? 'BROKEN TERM URI' : $term->label();
    return "Node {$operator} a '{$label}' media";
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $condition = ($this->options['negated']) ? 'NOT IN' : 'IN';
    $term = $this->utils->getTermForUri($this->options['use_uri']);
    if (empty($term)) {
      $this->logger->warning('Node Has Media Filter could not find term with URI: "%uri"', ['%uri' => $this->options['use_uri']]);
      return;
    }
    $sub_query = $this->connection->select('media', 'm');
    $use_alias = $sub_query->join('media__field_media_use', 'use', 'm.mid = %alias.entity_id');
    $of_alias = $sub_query->join('media__field_media_of', 'of', 'm.mid = %alias.entity_id');
    $sub_query->fields($of_alias, ['field_media_of_target_id'])
      ->condition("{$use_alias}.field_media_use_target_id", $term->id());

    /** @var \Drupal\views\Plugin\views\query\Sql $query */
    $query = $this->query;
    $alias = $query->ensureTable('node_field_data', $this->relationship);
    $query->addWhere(0, "{$alias}.nid", $sub_query, $condition);
  }

}
