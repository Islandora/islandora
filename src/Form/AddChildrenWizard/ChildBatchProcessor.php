<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\islandora\IslandoraUtils;
use Drupal\node\NodeInterface;

/**
 * Children addition batch processor.
 */
class ChildBatchProcessor extends AbstractBatchProcessor {

  /**
   * {@inheritdoc}
   */
  protected function getNode($info, array $values) : NodeInterface {
    $taxonomy_term_storage = $this->entityTypeManager->getStorage('taxonomy_term');
    $node_storage = $this->entityTypeManager->getStorage('node');
    $parent = $node_storage->load($values['node']);

    // Create a node (with the filename?) (and also belonging to the target
    // node).
    /** @var \Drupal\node\NodeInterface $node */
    $node = $node_storage->create([
      'type' => $values['bundle'],
      'title' => $this->getName($info, $values),
      IslandoraUtils::MEMBER_OF_FIELD => $parent,
      'uid' => $this->currentUser->id(),
      'status' => NodeInterface::PUBLISHED,
      IslandoraUtils::MODEL_FIELD => ($values['model'] ?
        $taxonomy_term_storage->load($values['model']) :
        NULL),
    ]);

    if ($node->save() !== SAVED_NEW) {
      throw new \Exception("Failed to create node.");
    }

    return $node;
  }

  /**
   * {@inheritdoc}
   */
  public function batchProcessFinished($success, $results, $operations): void {
    if ($success) {
      $this->messenger->addMessage($this->formatPlural(
        $results['count'],
        'Added 1 child node.',
        'Added @count child nodes.'
      ));
    }

    parent::batchProcessFinished($success, $results, $operations);
  }

}
