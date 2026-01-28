<?php

namespace Drupal\islandora\Plugin\Field\FieldFormatter;

use Drupal\file\Plugin\Field\FieldFormatter\FileMediaFormatterBase;
use Drupal\Core\Field\EntityReferenceFieldItemListInterface;
use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Cache\Cache;
use Drupal\Core\Template\Attribute;

/**
 * Extension of FileMediaFormatterBase that enables captions.
 */
abstract class IslandoraFileMediaFormatterBase extends FileMediaFormatterBase {

  /**
   * {@inheritdoc}
   */
  public function viewElements(FieldItemListInterface $items, $langcode) {
    $elements = [];

    $source_files = $this->getSourceFiles($items, $langcode);
    $track_files = $this->getTrackFiles($items, $langcode);
    if (!empty($source_files)) {
      $attributes = $this->prepareAttributes();
      foreach ($source_files as $delta => $files) {
        $elements[$delta] = [
          '#theme' => $this->getPluginId(),
          '#attributes' => $attributes,
          '#files' => $files,
          '#tracks' => $track_files[$delta] ?? [],
          '#cache' => ['tags' => []],
        ];

        $cache_tags = [];
        foreach ($files as $file) {
          $cache_tags = Cache::mergeTags($cache_tags, $file['file']->getCacheTags());
        }
        $elements[$delta]['#cache']['tags'] = $cache_tags;
      }
    }

    return $elements;
  }

  /**
   * Gets the track files with attributes.
   *
   * @param \Drupal\Core\Field\EntityReferenceFieldItemListInterface $items
   *   The items.
   * @param string $langcode
   *   The langcode.
   *
   * @return array
   *   Numerically indexed array, which again contains an associative array with
   *   the following key/values:
   *     - file => \Drupal\file\Entity\File
   *     - track_attributes => \Drupal\Core\Template\Attribute
   */
  protected function getTrackFiles(EntityReferenceFieldItemListInterface $items, $langcode) {
    $track_files = [];
    $media_entity = $items->getParent()->getEntity();
    $fields = $media_entity->getFields();
    foreach ($fields as $field) {
      $definition = $field->getFieldDefinition();
      if (method_exists($definition, 'get')) {
        if ($definition->get('field_type') == 'media_track') {
          // Extract the info for each track.
          $entities = $field->referencedEntities();
          $values = $field->getValue();
          foreach ($entities as $delta => $file) {
            $track_attributes = new Attribute();
            $track_attributes
              ->setAttribute('src', $file->createFileUrl())
              ->setAttribute('srclang', $values[$delta]['srclang'])
              ->setAttribute('label', $values[$delta]['label'])
              ->setAttribute('kind', $values[$delta]['kind']);
            if ($values[$delta]['default']) {
              $track_attributes->setAttribute('default', 'default');
            }
            $track_files[0][] = [
              'file' => $file,
              'track_attributes' => $track_attributes,
            ];
          }
        }
      }
    }
    return $track_files;
  }

}
