<?php

namespace Drupal\islandora\Plugin\views\filter;

use Drupal\Core\Form\FormStateInterface;
use Drupal\views\Plugin\views\filter\FilterPluginBase;

/**
 * Views Filter on Having Media of a Type.
 *
 * @ingroup views_field_handlers
 *
 * @ViewsFilter("islandora_node_has_media_use")
 */
class NodeHasMediaUse extends FilterPluginBase {

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
    $term = \Drupal::service('islandora.utils')->getTermForUri($uri);
    if (empty($term)) {
      $form_state->setError($form['use_uri'], $this->t('Could not find term with URI: "%uri"', ['%uri' => $uri]));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function buildOptionsForm(&$form, FormStateInterface $form_state) {
    $terms = \Drupal::entityTypeManager()->getStorage('taxonomy_term')->loadByProperties(['vid' => 'islandora_media_use']);
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
    $term = \Drupal::service('islandora.utils')->getTermForUri($this->options['use_uri']);
    $label = (empty($term)) ? 'BROKEN TERM URI' : $term->label();
    return "Node {$operator} a '{$label}' media";
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $condition = ($this->options['negated']) ? 'NOT IN' : 'IN';
    $utils = \Drupal::service('islandora.utils');
    $term = $utils->getTermForUri($this->options['use_uri']);
    if (empty($term)) {
      \Drupal::logger('islandora')->warning('Node Has Media Filter could not find term with URI: "%uri"', ['%uri' => $this->options['use_uri']]);
      return;
    }
    $sub_query = \Drupal::database()->select('media', 'm');
    $sub_query->join('media__field_media_use', 'use', 'm.mid = use.entity_id');
    $sub_query->join('media__field_media_of', 'of', 'm.mid = of.entity_id');
    $sub_query->fields('of', ['field_media_of_target_id'])
      ->condition('use.field_media_use_target_id', $term->id());
    $this->query->addWhere(0, 'nid', $sub_query, $condition);
  }

}
