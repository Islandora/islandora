<?php

namespace Drupal\islandora\Plugin\ContextReaction;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\Plugin\Action\AbstractGenerateDerivativeMediaFile;
use Drupal\islandora\PresetReaction\PresetReaction;

/**
 * Derivative context reaction.
 *
 * @ContextReaction(
 *   id = "file_derivative",
 *   label = @Translation("Derive file For Existing Media")
 * )
 */
class DerivativeFileReaction extends PresetReaction {

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $actions = $this->actionStorage->loadByProperties(['type' => 'media']);

    foreach ($actions as $action) {
      $plugin = $action->getPlugin();
      if ($plugin instanceof AbstractGenerateDerivativeMediaFile) {
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

  /**
   * {@inheritdoc}
   */
  public function execute(EntityInterface $entity = NULL) {
    $config = $this->getConfiguration();
    $action_ids = $config['actions'];
    foreach ($action_ids as $action_id) {
      $action = $this->actionStorage->load($action_id);
      $action->execute([$entity]);
    }
  }

}
