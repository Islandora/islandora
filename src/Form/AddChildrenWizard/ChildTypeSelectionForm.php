<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Core\Cache\CacheableMetadata;
use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\IslandoraUtils;

/**
 * Children addition wizard's first step.
 */
class ChildTypeSelectionForm extends MediaTypeSelectionForm {

  /**
   * {@inheritdoc}
   */
  public function getFormId() : string {
    return 'islandora_add_children_type_selection';
  }

  /**
   * Memoization for ::getNodeBundleOptions().
   *
   * @var array|null
   */
  protected ?array $nodeBundleOptions = NULL;

  /**
   * Indicate presence of model field on node bundles.
   *
   * Populated as a side effect of ::getNodeBundleOptions().
   *
   * @var array|null
   */
  protected ?array $nodeBundleHasModelField = NULL;

  /**
   * Helper; get the node bundle options available to the current user.
   *
   * @return array
   *   An associative array mapping node bundle machine names to their human-
   *   readable labels.
   */
  protected function getNodeBundleOptions() : array {
    if ($this->nodeBundleOptions === NULL) {
      $this->nodeBundleOptions = [];
      $this->nodeBundleHasModelField = [];

      $access_handler = $this->entityTypeManager->getAccessControlHandler('node');
      foreach ($this->entityTypeBundleInfo->getBundleInfo('node') as $bundle => $info) {
        $access = $access_handler->createAccess(
          $bundle,
          NULL,
          [],
          TRUE
        );
        $this->cacheableMetadata->addCacheableDependency($access);
        if (!$access->isAllowed()) {
          continue;
        }
        $this->nodeBundleOptions[$bundle] = $info['label'];
        $fields = $this->entityFieldManager->getFieldDefinitions('node', $bundle);
        $this->nodeBundleHasModelField[$bundle] = array_key_exists(IslandoraUtils::MODEL_FIELD, $fields);
      }
    }

    return $this->nodeBundleOptions;
  }

  /**
   * Generates a mapping of taxonomy term IDs to their names.
   *
   * @return \Generator
   *   The mapping of taxonomy term IDs to their names.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  protected function getModelOptions() : \Generator {
    $terms = $this->entityTypeManager->getStorage('taxonomy_term')
      ->loadTree('islandora_models', 0, NULL, TRUE);
    foreach ($terms as $term) {
      yield $term->id() => $term->getName();
    }
  }

  /**
   * Helper; map node bundles supporting the "has model" field, for #states.
   *
   * @return \Generator
   *   Yields associative array mapping the string 'value' to the bundles which
   *   have the given field.
   */
  protected function mapModelStates() : \Generator {
    $this->getNodeBundleOptions();
    foreach (array_keys(array_filter($this->nodeBundleHasModelField)) as $bundle) {
      yield ['value' => $bundle];
    }
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $this->cacheableMetadata = CacheableMetadata::createFromRenderArray($form)
      ->addCacheContexts([
        'url',
        'url.query_args',
      ]);
    $cached_values = $form_state->getTemporaryValue('wizard');

    $form['bundle'] = [
      '#type' => 'select',
      '#title' => $this->t('Content Type'),
      '#description' => $this->t('Each child created will have this content type.'),
      '#empty_value' => '',
      '#default_value' => $cached_values['bundle'] ?? '',
      '#options' => $this->getNodeBundleOptions(),
      '#required' => TRUE,
    ];

    $model_states = iterator_to_array($this->mapModelStates());
    $form['model'] = [
      '#type' => 'select',
      '#title' => $this->t('Model'),
      '#description' => $this->t('Each child will be tagged with this model.'),
      '#options' => iterator_to_array($this->getModelOptions()),
      '#empty_value' => '',
      '#default_value' => $cached_values['model'] ?? '',
      '#states' => [
        'visible' => [
          ':input[name="bundle"]' => $model_states,
        ],
        'required' => [
          ':input[name="bundle"]' => $model_states,
        ],
      ],
    ];

    $this->cacheableMetadata->applyTo($form);
    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  protected static function keysToSave() : array {
    return array_merge(
      parent::keysToSave(),
      [
        'bundle',
        'model',
      ]
    );
  }

}
