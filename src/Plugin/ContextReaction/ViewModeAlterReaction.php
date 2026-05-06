<?php

namespace Drupal\islandora\Plugin\ContextReaction;

use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\ContextReaction\DisplayAlterReaction;

/**
 * Context reaction to alter view mode.
 *
 * @ContextReaction(
 *   id = "view_mode_alter",
 *   label = @Translation("Change view mode")
 * )
 */
class ViewModeAlterReaction extends DisplayAlterReaction {

  /**
   * {@inheritdoc}
   */
  public function summary() {
    return $this->t('Change the view mode for an entity');
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    // Build up options array.
    $entity_types = $this->entityTypeManager->getDefinitions();
    $view_modes = $this->entityTypeManager->getStorage('entity_view_mode')->loadMultiple();

    $options = [];

    foreach ($view_modes as $view_mode) {
      $exploded = explode('.', $view_mode->id());
      $entity_type_label = $entity_types[$exploded[0]]->getLabel()->render();
      $options[$entity_type_label][$view_mode->id()] = $view_mode->label();
    }

    // Provide a select to choose display mode.
    $config = $this->getConfiguration();
    $form[self::MODE] = [
      '#title' => $this->t('View Mode'),
      '#description' => $this->t("The selected view mode will be used if conditions are met."),
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $config[self::MODE] ?? '',
    ];
    return $form;
  }

}
