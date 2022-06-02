<?php

namespace Drupal\islandora\Plugin\ContextReaction;

use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\Plugin\Action\AbstractGenerateDerivative;
use Drupal\islandora\PresetReaction\PresetReaction;

/**
 * Derivative context reaction.
 *
 * @ContextReaction(
 *   id = "derivative",
 *   label = @Translation("Derivative")
 * )
 */
class DerivativeReaction extends PresetReaction {

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $actions = $this->actionStorage->loadMultiple();
    $options = [];
    foreach ($actions as $action) {
      $plugin = $action->getPlugin();
      if ($plugin instanceof AbstractGenerateDerivative) {
        $options[ucfirst($action->getType())][$action->id()] = $action->label();
      }
    }
    $config = $this->getConfiguration();
    $form['actions'] = [
      '#title' => $this->t('Actions'),
      '#description' => $this->t('Pre-configured actions to execute. Multiple actions may be selected by shift or ctrl clicking.'),
      '#type' => 'select',
      '#multiple' => TRUE,
      '#options' => $options,
      '#default_value' => isset($config['actions']) ? $config['actions'] : '',
      '#size' => 15,
    ];

    return $form;
  }

}
