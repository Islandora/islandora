<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Form\FormStateInterface;

/**
 * Provides a 'term' condition for a node's parent.
 *
 * Like parent_node_has_term but applicable to a node instead of media.
 * Like node_has_term but applicable to parent node.
 * A typical use case is to test for a child node within a compound item.
 *
 * @Condition(
 *   id = "parent_node_of_node_has_term",
 *   label = @Translation("Parent node for node has term with URI"),
 *   context_definitions = {
 *     "node" = @ContextDefinition("entity:node", required = TRUE , label = @Translation("node"))
 *   }
 * )
 */
class ParentNodeOfNodeHasTerm extends NodeHasTerm {

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildConfigurationForm($form, $form_state);
    // Make term not required.
    $form['term']['#required'] = FALSE;
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    if (empty($this->configuration['uri']) && !$this->isNegated()) {
      return TRUE;
    }

    $node = $this->getContextValue('node');
    if (!$node) {
      return FALSE;
    }

    $fields = [$this->utils::MEMBER_OF_FIELD];
    $parentIds = $this->utils->findAncestors($node, $fields, 1);
    if (!$parentIds) {
      return FALSE;
    }
    $result = 0;
    foreach ($parentIds as $parentId) {
      $parent = $this->entityTypeManager->getStorage('node')->load($parentId);
      if ($parent) {
        $parentResult = $this->evaluateEntity($parent);
        $result = $result + (int) $parentResult;
      }
    }
    return (boolean) $result;
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (!empty($this->configuration['negate'])) {
      return $this->t('The parent node is not associated with taxonomy term with uri @uri.', ['@uri' => $this->configuration['uri']]);
    }
    else {
      return $this->t('The parent node is associated with taxonomy term with uri @uri.', ['@uri' => $this->configuration['uri']]);
    }
  }

}
