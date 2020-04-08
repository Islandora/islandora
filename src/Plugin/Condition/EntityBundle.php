<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Provides a 'Entity Bundle' condition.
 *
 * Namespaced to avoid conflict with ctools entity_bundle plugin.
 *
 * @Condition(
 *   id = "islandora_entity_bundle",
 *   label = @Translation("Entity Bundle"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = FALSE, label = @Translation("Node")),
 *     "media" = @ContextDefinition("entity:media", required = FALSE, label = @Translation("Media")),
 *     "taxonomy_term" = @ContextDefinition("entity:taxonomy_term", required = FALSE, label = @Translation("Term"))
 *   }
 * )
 */
class IslandoraEntityBundle extends ConditionPluginBase {

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $options = [];
    foreach (['node', 'media', 'taxonomy_term'] as $content_entity) {
      $bundles = \Drupal::service('entity_type.bundle.info')->getBundleInfo($content_entity);
      foreach ($bundles as $bundle => $bundle_properties) {
        $options[$bundle] = $this->t('@bundle (@type)', [
          '@bundle' => $bundle_properties['label'],
          '@type' => $content_entity,
        ]);
      }
    }

    $form['bundles'] = [
      '#title' => $this->t('Bundles'),
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $this->configuration['bundles'],
    ];

    return parent::buildConfigurationForm($form, $form_state);;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->configuration['bundles'] = array_filter($form_state->getValue('bundles'));
    parent::submitConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    foreach ($this->getContexts() as $context) {
      if ($context->hasContextValue()) {
        $entity = $context->getContextValue();
        if (!empty($this->configuration['bundles'][$entity->bundle()])) {
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
    if (empty($this->configuration['bundles'])) {
      return $this->t('No bundles are selected.');
    }

    return $this->t(
      'Entity bundle in the list: @bundles',
      [
        '@bundles' => implode(', ', $this->configuration['field']),
      ]
    );
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return array_merge(
      ['bundles' => []],
      parent::defaultConfiguration()
    );
  }

}
