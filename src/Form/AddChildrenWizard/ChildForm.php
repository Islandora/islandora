<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

/**
 * Bulk children addition wizard base form.
 */
class ChildForm extends AbstractForm {

  const TEMPSTORE_ID = 'islandora.upload_children';
  const TYPE_SELECTION_FORM = ChildTypeSelectionForm::class;
  const FILE_SELECTION_FORM = ChildFileSelectionForm::class;

  /**
   * {@inheritdoc}
   */
  public function getMachineName() {
    return strtr("islandora_add_children_wizard__{userid}__{nodeid}", [
      '{userid}' => $this->currentUser->id(),
      '{nodeid}' => $this->nodeId,
    ]);
  }

}
