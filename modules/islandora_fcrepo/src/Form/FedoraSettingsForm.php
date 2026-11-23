<?php

namespace Drupal\islandora_fcrepo\Form;

use Drupal\Core\Cache\Cache;
use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Entity\EntityTypeBundleInfoInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Site\Settings;

/**
 * Configures Fedora URI display independently of core Islandora.
 */
class FedoraSettingsForm extends ConfigFormBase {

  const CONFIG_NAME = 'islandora_fcrepo.settings';
  const GEMINI_PSEUDO = 'gemini_pseudo_bundles';
  const GEMINI_PSEUDO_FIELD = 'field_gemini_uri';

  /**
   * Constructs the Fedora settings form.
   */
  public function __construct(
    ConfigFactoryInterface $config_factory,
    protected EntityTypeManagerInterface $entityTypeManager,
    protected EntityTypeBundleInfoInterface $bundleInfo,
  ) {
    $this->setConfigFactory($config_factory);
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static($container->get('config.factory'), $container->get('entity_type.manager'), $container->get('entity_type.bundle.info'));
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_fcrepo_settings';
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [self::CONFIG_NAME];
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $settings = Settings::get('flysystem', []);
    $roots = [];
    foreach ($settings as $scheme => $entry) {
      if (($entry['driver'] ?? NULL) === 'fedora') {
        $roots[] = $scheme . ': ' . ($entry['config']['root'] ?? '');
      }
    }
    $form['roots'] = [
      '#type' => 'item',
      '#title' => $this->t('Fedora roots (configured in settings.php)'),
      '#plain_text' => implode(', ', $roots),
    ];
    $options = [];
    foreach (['node', 'media', 'taxonomy_term'] as $type) {
      foreach ($this->bundleInfo->getBundleInfo($type) as $bundle => $info) {
        $options["$bundle:$type"] = $this->t('@label (@type)', ['@label' => $info['label'], '@type' => $type]);
      }
    }
    $form[self::GEMINI_PSEUDO] = [
      '#type' => 'checkboxes',
      '#title' => $this->t('Fedora URL display'),
      '#options' => $options,
      '#default_value' => $this->config(self::CONFIG_NAME)->get(self::GEMINI_PSEUDO) ?? [],
    ];
    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $config = $this->config(self::CONFIG_NAME);
    $selected = array_filter($form_state->getValue(self::GEMINI_PSEUDO));
    $this->updateEntityViewConfiguration($config->get(self::GEMINI_PSEUDO) ?? [], $selected);
    $config->set(self::GEMINI_PSEUDO, $selected)->save();
    parent::submitForm($form, $form_state);
  }

  /**
   * Removes the Fedora URI field from entity bundles that have be unselected.
   *
   * @param array $current_config
   *   The current set of entity types & bundles to have the pseudo field,
   *   format {bundle}:{entity_type}.
   * @param array $new_config
   *   The new set of entity types & bundles to have the pseudo field, format
   *   {bundle}:{entity_type}.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  private function updateEntityViewConfiguration(array $current_config, array $new_config) {
    $removed = array_diff($current_config, $new_config);
    $added = array_diff($new_config, $current_config);
    $entity_view_display = $this->entityTypeManager->getStorage('entity_view_display');
    foreach ($removed as $bundle_type) {
      [$bundle, $type_id] = explode(":", $bundle_type);
      $results = $entity_view_display->getQuery()
        ->condition('bundle', $bundle)
        ->condition('targetEntityType', $type_id)
        ->exists('content.' . self::GEMINI_PSEUDO_FIELD . '.region')
        ->execute();
      $entities = $entity_view_display->loadMultiple($results);
      foreach ($entities as $entity) {
        $entity->removeComponent(self::GEMINI_PSEUDO_FIELD);
        $entity->save();
      }
    }
    if (count($removed) > 0 || count($added) > 0) {
      // If we added or cleared a type then clear the extra_fields cache.
      // @see Drupal/Core/Entity/EntityFieldManager::getExtraFields
      Cache::invalidateTags(["entity_field_info"]);
    }
  }

}
