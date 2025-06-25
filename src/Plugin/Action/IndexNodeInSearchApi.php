<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Access\AccessResult;
use Drupal\Core\Action\ConfigurableActionBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Indexes a node in Search API
 *
 * @Action(
 *   id = "index_node_in_search_api",
 *   label = @Translation("Index a node in Search API"),
 *   type = "node"
 * )
 */
class IndexNodeInSearchApi extends ConfigurableActionBase implements ContainerFactoryPluginInterface {
  use StringTranslationTrait;

  /**
   * @param array $configuration
   *   The plugin configuration, i.e. an array with configuration values keyed
   *    by configuration option name. The special key 'context' may be used to
   *    initialize the defined contexts by setting it to an array of context
   *    values keyed by context names.
   * @param mixed $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   The Islandora Utils.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition)
  {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
  }

  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition
  );
  }

  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form['index'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Index'),
      '#default_value' => $this->configuration['index'],
      '#required' => TRUE,
    ];
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->configuration['index'] = $form_state->getValue('index');
  }

  public function access($object, AccountInterface $account = NULL, $return_as_object = FALSE) {
    $result = AccessResult::allowed();
    return $return_as_object ? $result : $result->isAllowed();
  }

  public function execute($node = NULL) {
    $node->save(); # FIXME change to actual indexing calls.
  }

}
