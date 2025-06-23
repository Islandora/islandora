<?php

namespace Drupal\islandora\Plugin\search_api\processor;

use Drupal\search_api\Processor\ProcessorPluginBase;
use Drupal\search_api\Datasource\DatasourceInterface;
use Drupal\search_api\Item\ItemInterface;
use Drupal\search_api\Processor\ProcessorProperty;

/**
 * Adds a media count field to indexed nodes.
 *
 * @SearchApiProcessor(
 *   id = "media_count",
 *   label = @Translation("Media count"),
 *   description = @Translation("Adds a count of media referencing each node via field_media_of."),
 *   stages = {
 *     "add_properties" = 0,
 *   },
 *  locked = false,
 *  hidden = false,
 * )
 */
class MediaCount extends ProcessorPluginBase
{

  /**
   * {@inheritdoc}
   */
  public function getPropertyDefinitions(DatasourceInterface $datasource = NULL)
  {
    $properties = [];

    if (!$datasource || !$datasource->getPluginId()) {
      return $properties;
    }

    if ($datasource->getPluginId() == 'entity:node') {
      $definition = [
        'label' => $this->t('Media count'),
        'description' => $this->t('Count of media entities referencing this node via field_media_of.'),
        'type' => 'integer',
        'processor_id' => $this->getPluginId(),
      ];
      $properties['media_count'] = new ProcessorProperty($definition);
    }

    return $properties;
  }

  /**
   * {@inheritdoc}
   */
  public function addFieldValues(ItemInterface $item){
    $entity = $item->getOriginalObject()->getValue();

    // Only process if this is a node entity
    if (!$entity || $entity->getEntityTypeId() !== 'node') {
      return;
    }

    // Count media entities referencing this node via field_media_of
    $count = \Drupal::entityTypeManager()->getStorage('media')
      ->getQuery()
      ->accessCheck(FALSE)
      ->condition('field_media_of', $entity->id())
      ->count()
      ->execute();

// Add the value to all fields configured for this property
    $fields = $this->getFieldsHelper()
      ->filterForPropertyPath($item->getFields(), 'entity:node', 'media_count');

    foreach ($fields as $field) {
      $field->addValue((int)$count);
    }
  }
}
