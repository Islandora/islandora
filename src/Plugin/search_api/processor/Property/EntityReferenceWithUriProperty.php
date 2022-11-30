<?php

namespace Drupal\islandora\Plugin\search_api\processor\Property;

use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\search_api\Item\FieldInterface;
use Drupal\search_api\Processor\ConfigurablePropertyBase;
use Drupal\taxonomy\Entity\Term;

/**
 * Defines a "Entity Reference with URI" search property.
 *
 * @see \Drupal\islandora\Plugin\search_api\processor\EntityReferenceWithUri
 */
class EntityReferenceWithUriProperty extends ConfigurablePropertyBase {

  use StringTranslationTrait;

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return [
      'filter_terms' => [],
      'require_all' => TRUE,
      'target_type' => '',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(FieldInterface $field, array $form, FormStateInterface $form_state) {
    $configuration = $field->getConfiguration();
    $logger = \Drupal::logger('islandora');
    $logger->info('<pre>' . print_r($configuration['filter_terms'], TRUE) . '</pre>');
    $form['filter_terms'] = [
      '#type' => 'entity_autocomplete',
      '#target_type' => 'taxonomy_term',
      '#title' => $this->t('Related entities must have this URI to be included.'),
      '#tags' => TRUE,
      '#default_value' => array_map(function ($element) {
        return Term::load($element['target_id']);
      }, array_values($configuration['filter_terms'])),
      '#required' => TRUE,
    ];
    $form['require_all'] = [
      '#type' => 'checkbox',
      '#title' => 'Require all terms',
      '#description' => 'Only index related entities that have all the listed terms.',
      '#default_value' => $configuration['require_all'],
    ];
    // Store the target type of the field, it's a pain to get it when indexing.
    $form['target_type'] = [
      '#type' => 'hidden',
      '#value' => $field->getDataDefinition()->getSetting('target_type'),
    ];
    return $form;
  }

}
