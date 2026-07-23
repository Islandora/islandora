<?php

namespace Drupal\islandora\Plugin\Field\FieldFormatter;

use Drupal\Core\Field\Attribute\FieldFormatter;
use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\image\Plugin\Field\FieldFormatter\ImageFormatter;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Plugin implementation of the 'image' formatter for a media.
 */
#[FieldFormatter(
  id: "islandora_image",
  label: new TranslatableMarkup("Islandora Image"),
  field_types: ["image"]
)]
class IslandoraImageFormatter extends ImageFormatter {

  /**
   * Islandora utility functions.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Islandora media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSourceService;

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    $instance = parent::create($container, $configuration, $plugin_id, $plugin_definition);
    $instance->utils = $container->get('islandora.utils');
    $instance->mediaSourceService = $container->get('islandora.media_source_service');
    return $instance;
  }

  /**
   * {@inheritdoc}
   */
  public static function defaultSettings() {
    return [
      'image_alt_text' => 'local',
    ] + parent::defaultSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array $form, FormStateInterface $form_state) {
    $element = parent::settingsForm($form, $form_state);
    $alt_text_options = [
      'local' => $this->t('Local'),
      'original_file_fallback' => $this->t('Local, with fallback to Original File'),
      'original_file' => $this->t('Original File'),
    ];
    $element['image_alt_text'] = [
      '#title' => $this->t('Alt text source'),
      '#type' => 'select',
      '#default_value' => $this->getSetting('image_alt_text'),
      '#empty_option' => $this->t('None'),
      '#options' => $alt_text_options,
    ];
    return $element;
  }

  /**
   * {@inheritdoc}
   */
  public function viewElements(FieldItemListInterface $items, $langcode) {
    $elements = parent::viewElements($items, $langcode);

    $image_link_setting = $this->getSetting('image_link');
    $alt_text_setting = $this->getsetting('image_alt_text');

    // Check if we can leave the image as-is:
    if ($image_link_setting !== 'content' && $alt_text_setting === 'local') {
      return $elements;
    }
    $entity = $items->getEntity();
    if ($entity->isNew() || $entity->getEntityTypeId() !== 'media') {
      return $elements;
    }

    if ($alt_text_setting === 'none') {
      foreach ($elements as $element) {
        $element['#item']->set('alt', '');
      }
    }

    if ($image_link_setting === 'content' || $alt_text_setting === 'original_file' || $alt_text_setting === 'original_file_fallback') {
      $node = $this->utils->getParentNode($entity);
      if ($node === NULL) {
        return $elements;
      }

      if ($image_link_setting === 'content') {
        // Set image link.
        $url = $node->toUrl();
        foreach ($elements as &$element) {
          $element['#url'] = $url;
        }
        unset($element);
      }

      if ($alt_text_setting === 'original_file' || $alt_text_setting === 'original_file_fallback') {
        $original_file_term = $this->utils->getTermForUri("http://pcdm.org/use#OriginalFile");

        if ($original_file_term !== NULL) {
          $original_file_media = $this->utils->getMediaWithTerm($node, $original_file_term);

          if ($original_file_media !== NULL) {
            $source_field_name = $this->mediaSourceService->getSourceFieldName($original_file_media->bundle());
            if ($original_file_media->hasField($source_field_name)) {
              $original_file_files = $original_file_media->get($source_field_name);
              // XXX: Support the multifile media use case where there could
              // be multiple files in the source field.
              $i = 0;
              foreach ($original_file_files as $file) {
                if (isset($file->alt)) {
                  $alt_text = $file->get('alt')->getValue();
                  if (isset($elements[$i])) {
                    $element = $elements[$i];
                    if ($alt_text_setting === 'original_file' || $element['#item']->get('alt')->getValue() === '') {
                      $elements[$i]['#item']->set('alt', $alt_text);
                    }
                    $i++;
                  }
                }
              }
            }
          }
        }
      }
    }
    return $elements;
  }

}
