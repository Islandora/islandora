<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\node\NodeInterface;

/**
 * Media addition batch processor.
 */
class MediaBatchProcessor extends AbstractBatchProcessor {

  /**
   * {@inheritdoc}
   */
  protected function getNode($info, array $values) : NodeInterface {
    return $this->entityTypeManager->getStorage('node')->load($values['node']);
  }

  /**
   * {@inheritdoc}
   */
  public function batchProcessFinished($success, $results, $operations): void {
    if ($success) {
      $this->messenger->addMessage($this->formatPlural(
        $results['count'],
        'Added 1 media.',
        'Added @count media.'
      ));
    }

    parent::batchProcessFinished($success, $results, $operations);
  }

}
