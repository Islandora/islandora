<?php

namespace Drupal\islandora\ContextReaction;

use Drupal\context\ContextReactionPluginBase;
use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\jsonld\Form\JsonLdSettingsForm;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Base class to alter the normalizes Json-ld.
 *
 * Plugins must extend this class to be considered for execution.
 *
 * @package Drupal\islandora\ContextReaction
 */
abstract class NormalizerAlterReaction extends ContextReactionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * The configuration.
   *
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  protected $jsonldConfig;

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * {@inheritdoc}
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    ConfigFactoryInterface $config_factory,
    IslandoraUtils $utils,
  ) {

    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->jsonldConfig = $config_factory->get(JsonLdSettingsForm::CONFIG_NAME);
    $this->utils = $utils;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('config.factory'),
      $container->get('islandora.utils')
    );
  }

  /**
   * This reaction can alter the array of json-ld built from the entity.
   *
   * @param \Drupal\Core\Entity\EntityInterface|null $entity
   *   The entity we are normalizing.
   * @param array|null $normalized
   *   The normalized json-ld before encoding.
   * @param array|null $context
   *   The context used in the normalizer.
   */
  abstract public function execute(?EntityInterface $entity = NULL, ?array &$normalized = NULL, ?array $context = NULL);

  /**
   * Helper function to get the url for an entity that repsects jsonld config.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity.
   *
   * @return string
   *   The url.
   */
  protected function getSubjectUrl(EntityInterface $entity) {
    $format = '';
    if (!$this->jsonldConfig->get(JsonLdSettingsForm::REMOVE_JSONLD_FORMAT)) {
      $format = 'jsonld';
    }
    return $this->utils->getRestUrl($entity, $format);
  }

}
