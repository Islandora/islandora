<?php

namespace Drupal\islandora\Plugin\Condition;

use Drupal\Core\Condition\ConditionPluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Checks if media entity has fields that qualify it as an "Islandora" media.
 *
 * @Condition(
 *   id = "media_is_islandora_media",
 *   label = @Translation("Media is an Islandora media"),
 *   context_definitions = {
 *     "media" = @ContextDefinition("entity:media", required = TRUE , label = @Translation("media"))
 *   }
 * )
 */
class MediaIsIslandoraMedia extends ConditionPluginBase implements ContainerFactoryPluginInterface {

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition
    );
  }

  /**
   * {@inheritdoc}
   */
  public function evaluate() {
    $media = $this->getContextValue('media');
    if (!$media) {
      return FALSE;
    }
    // Islandora Media have these two fields.
    if ($media->hasField('field_media_use') && $media->hasField('field_media_of')) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    if (!empty($this->configuration['negate'])) {
      return $this->t('The media is not an Islandora media.');
    }
    else {
      return $this->t('The media is an Islandora media.');
    }
  }

}
