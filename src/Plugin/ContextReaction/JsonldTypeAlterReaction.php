<?php

namespace Drupal\islandora\Plugin\ContextReaction;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\ContextReaction\NormalizerAlterReaction;
use Drupal\jsonld\Normalizer\NormalizerBase;
use Drupal\islandora\IslandoraUtils;

/**
 * Alter JSON-LD Type context reaction.
 *
 * @ContextReaction(
 *   id = "alter_jsonld_type",
 *   label = @Translation("Alter JSON-LD Type")
 * )
 */
class JsonldTypeAlterReaction extends NormalizerAlterReaction {

  /**
   * {@inheritdoc}
   */
  public function summary() {
    return $this->t('Alter JSON-LD Type context reaction.');
  }

  /**
   * {@inheritdoc}
   */
  public function execute(EntityInterface $entity = NULL, array &$normalized = NULL, array $context = NULL) {
    // Check that the source field exists and there's some RDF
    // to manipulate.
    $config = $this->getConfiguration();
    $ok = $entity->hasField($config['source_field']) &&
        !empty($entity->get($config['source_field'])->getValue()) &&
        isset($normalized['@graph']) &&
        is_array($normalized['@graph']) &&
        !empty($normalized['@graph']);

    if (!$ok) {
      return;
    }

    // Search for the entity in the graph.
    foreach ($normalized['@graph'] as &$elem) {
      if (!is_array($elem['@type'])) {
        $elem['@type'] = [$elem['@type']];
      }
      if ($elem['@id'] === $this->getSubjectUrl($entity)) {
        foreach ($entity->get($config['source_field'])->getValue() as $type) {
          // If the configured field is using an entity reference,
          // we will see if it uses the core config's
          // IslandoraUtils::EXTERNAL_URI_FIELD.
          if (array_key_exists('target_id', $type)) {
            $target_type = $entity->get($config['source_field'])->getFieldDefinition()->getSetting('target_type');
            $referenced_entity = \Drupal::entityTypeManager()->getStorage($target_type)->load($type['target_id']);
            if ($referenced_entity->hasField(IslandoraUtils::EXTERNAL_URI_FIELD) &&
                !empty($referenced_entity->get(IslandoraUtils::EXTERNAL_URI_FIELD)->getValue())) {
              foreach ($referenced_entity->get(IslandoraUtils::EXTERNAL_URI_FIELD)->getValue() as $value) {
                $elem['@type'][] = $value['uri'];
              }
            }
          }
          else {
            $elem['@type'][] = NormalizerBase::escapePrefix($type['value'], $context['namespaces']);
          }
        }
        return;
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $options = [];
    $fieldsArray = \Drupal::service('entity_field.manager')->getFieldMap();
    foreach ($fieldsArray as $entity_type => $entity_fields) {
      foreach ($entity_fields as $field => $field_properties) {
        $options[$field] = $this->t('@field (@bundles)', [
          '@field' => $field,
          '@bundles' => implode(', ', array_keys($field_properties['bundles'])),
        ]);
      }
    }

    $config = $this->getConfiguration();
    $form['source_field'] = [
      '#type' => 'select',
      '#title' => $this->t('Source Field'),
      '#options' => $options,
      '#description' => $this->t("Select the field containing the type predicates."),
      '#default_value' => isset($config['source_field']) ? $config['source_field'] : '',
    ];
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->setConfiguration(['source_field' => $form_state->getValue('source_field')]);
  }

}
