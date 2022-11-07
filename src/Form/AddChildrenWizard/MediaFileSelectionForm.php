<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;

/**
 * Media addition wizard's second step.
 */
class MediaFileSelectionForm extends AbstractFileSelectionForm {

  public const BATCH_PROCESSOR = 'islandora.upload_media.batch_processor';

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_add_media_wizard_file_selection';
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);

    $cached_values = $form_state->getTemporaryValue('wizard');
    $form_state->setRedirectUrl(Url::fromUri("internal:/node/{$cached_values['node']}/media"));
  }

}
