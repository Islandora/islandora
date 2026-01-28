<?php

namespace Drupal\islandora\Plugin\ContextReaction;

use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\ContextReaction\DisplayAlterReaction;

/**
 * Context reaction to alter form mode.
 *
 * @ContextReaction(
 *   id = "form_display_alter",
 *   label = @Translation("Change form mode")
 * )
 */
class FormDisplayAlterReaction extends DisplayAlterReaction {

  /**
   * {@inheritdoc}
   */
  public function summary() {
    return $this->t('Change the form display for an entity');
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    // Build up options array.
    $entity_types = $this->entityTypeManager->getDefinitions();
    $form_displays = $this->entityTypeManager->getStorage('entity_form_display')->loadMultiple();

    $options = [];

    foreach ($form_displays as $form_display) {
      $entity_type = $form_display->getTargetEntityTypeId();
      $entity_type_label = $entity_types[$entity_type]->getLabel()->render();
      $mode = $form_display->getMode();
      $options[$entity_type_label]["$entity_type.$mode"] = ucfirst($mode);
    }

    // Provide a select to choose display mode.
    $config = $this->getConfiguration();
    $form[self::MODE] = [
      '#title' => $this->t('Form Mode'),
      '#description' => $this->t("The selected form mode will be used if conditions are met."),
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $config[self::MODE] ?? '',
    ];
    return $form;
  }

}
