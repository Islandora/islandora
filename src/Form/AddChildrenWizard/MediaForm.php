<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

/**
 * Bulk children addition wizard base form.
 */
class MediaForm extends AbstractForm {

  const TEMPSTORE_ID = 'islandora.upload_media';
  const TYPE_SELECTION_FORM = MediaTypeSelectionForm::class;
  const FILE_SELECTION_FORM = MediaFileSelectionForm::class;

  /**
   * {@inheritdoc}
   */
  public function getMachineName() {
    return strtr("islandora_add_media_wizard__{userid}__{nodeid}", [
      '{userid}' => $this->currentUser->id(),
      '{nodeid}' => $this->nodeId,
    ]);
  }

}
