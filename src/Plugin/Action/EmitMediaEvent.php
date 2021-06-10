<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Entity\EntityInterface;
use Drupal\islandora\EventGenerator\EmitEvent;
use Drupal\islandora\MediaSource\MediaSourceService;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Emits a Media event.
 *
 * @Action(
 *   id = "emit_media_event",
 *   label = @Translation("Emit a media event to a queue/topic"),
 *   type = "media"
 * )
 */
class EmitMediaEvent extends EmitEvent {

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSource;

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    $instance = parent::create($container, $configuration, $plugin_id, $plugin_definition);

    $instance->setMediaSourceService($container->get('islandora.media_source_service'));

    return $instance;
  }

  /**
   * {@inheritdoc}
   */
  protected function generateData(EntityInterface $entity) {
    $data = parent::generateData($entity);
    $data['source_field'] = $this->mediaSource->getSourceFieldName($entity->bundle());
    return $data;
  }

  /**
   * Setter for the media source service.
   */
  public function setMediaSourceService(MediaSourceService $media_source) {
    $this->mediaSource = $media_source;
  }

}
