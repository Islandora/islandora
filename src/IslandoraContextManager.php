<?php

namespace Drupal\islandora;

use Drupal\context\ContextManager;
use Drupal\context\ContextInterface;
use Drupal\Core\Condition\ConditionPluginCollection;
use Drupal\Core\Plugin\ContextAwarePluginInterface;
use Drupal\Component\Plugin\Exception\ContextException;

/**
 * Threads in additional (core) Contexts to provide to Conditions.
 */
class IslandoraContextManager extends ContextManager {

  /**
   * Allow the contexts to be reset before evaluation.
   */
  protected function resetContextEvaluation() {
    $this->contexts = [];
    $this->contextConditionsEvaluated = FALSE;
  }

  /**
   * Evaluate all context conditions.
   *
   * @param \Drupal\Core\Plugin\Context\Context[] $provided
   *   Additional provided (core) contexts to apply to Conditions.
   */
  public function evaluateContexts(array $provided = []) {

    $this->activeContexts = [];
    // XXX: Ensure that no earlier executed contexts in the request are still
    // present when being triggered via Islandora's ContextProviders.
    if (!empty($provided)) {
      $this->resetContextEvaluation();
    }
    /** @var \Drupal\context\ContextInterface $context */
    foreach ($this->getContexts() as $context) {
      if ($this->evaluateContextConditions($context, $provided) && !$context->disabled()) {
        $this->activeContexts[$context->id()] = $context;
      }
    }

    $this->contextConditionsEvaluated = TRUE;
  }

  /**
   * Evaluate a contexts conditions.
   *
   * @param \Drupal\context\ContextInterface $context
   *   The context to evaluate conditions for.
   * @param \Drupal\Core\Plugin\Context\Context[] $provided
   *   Additional provided (core) contexts to apply to Conditions.
   *
   * @return bool
   *   TRUE if conditions pass
   */
  public function evaluateContextConditions(ContextInterface $context, array $provided = []) {
    $conditions = $context->getConditions();

    // Apply context to any context aware conditions.
    $this->applyContexts($conditions, $provided);

    // Set the logic to use when validating the conditions.
    $logic = $context->requiresAllConditions()
      ? 'and'
      : 'or';

    // Of there are no conditions then the context will be
    // applied as a site wide context.
    if (!count($conditions)) {
      $logic = 'and';
    }

    return $this->resolveConditions($conditions, $logic);
  }

  /**
   * Apply context to all the context aware conditions in the collection.
   *
   * @param \Drupal\Core\Condition\ConditionPluginCollection $conditions
   *   A collection of conditions to apply context to.
   * @param \Drupal\Core\Plugin\Context\Context[] $provided
   *   Additional provided (core) contexts to apply to Conditions.
   *
   * @return bool
   *   TRUE if conditions pass
   */
  protected function applyContexts(ConditionPluginCollection &$conditions, array $provided = []) {
    foreach ($conditions as $condition) {
      if ($condition instanceof ContextAwarePluginInterface) {
        try {
          if (empty($provided)) {
            $contexts = $this->contextRepository->getRuntimeContexts(array_values($condition->getContextMapping()));
          }
          else {
            $contexts = $provided;
          }
          $this->contextHandler->applyContextMapping($condition, $contexts);
        }
        catch (ContextException $e) {
          return FALSE;
        }
      }
    }

    return TRUE;
  }

}
