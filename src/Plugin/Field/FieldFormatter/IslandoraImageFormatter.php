<?php

namespace Drupal\islandora\Plugin\Field\FieldFormatter;

use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Field\FieldDefinitionInterface;
use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\File\FileUrlGeneratorInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\image\Plugin\Field\FieldFormatter\ImageFormatter;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Plugin implementation of the 'image' formatter for a media.
 *
 * @FieldFormatter(
 *   id = "islandora_image",
 *   label = @Translation("Islandora Image"),
 *   field_types = {
 *     "image"
 *   },
 *   quickedit = {
 *     "editor" = "image"
 *   }
 * )
 */
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
   * Constructs an IslandoraImageFormatter object.
   *
   * @param string $plugin_id
   *   The plugin_id for the formatter.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Field\FieldDefinitionInterface $field_definition
   *   The definition of the field to which the formatter is associated.
   * @param array $settings
   *   The formatter settings.
   * @param string $label
   *   The formatter label display setting.
   * @param string $view_mode
   *   The view mode.
   * @param array $third_party_settings
   *   Any third party settings settings.
   * @param \Drupal\Core\Session\AccountInterface $current_user
   *   The current user.
   * @param \Drupal\Core\Entity\EntityStorageInterface $image_style_storage
   *   The image style storage.
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   * @param \Drupal\Core\File\FileUrlGeneratorInterface $file_url_generator
   *   The File URL Generator.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source_service
   *   Utils to get the source file from media.
   */
  public function __construct(
    $plugin_id,
    $plugin_definition,
    FieldDefinitionInterface $field_definition,
    array $settings,
    $label,
    $view_mode,
    array $third_party_settings,
    AccountInterface $current_user,
    EntityStorageInterface $image_style_storage,
    IslandoraUtils $utils,
    FileUrlGeneratorInterface $file_url_generator,
    MediaSourceService $media_source_service
  ) {
    parent::__construct(
      $plugin_id,
      $plugin_definition,
      $field_definition,
      $settings,
      $label,
      $view_mode,
      $third_party_settings,
      $current_user,
      $image_style_storage,
      $file_url_generator
    );
    $this->utils = $utils;
    $this->mediaSourceService = $media_source_service;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $plugin_id,
      $plugin_definition,
      $configuration['field_definition'],
      $configuration['settings'],
      $configuration['label'],
      $configuration['view_mode'],
      $configuration['third_party_settings'],
      $container->get('current_user'),
      $container->get('entity_type.manager')->getStorage('image_style'),
      $container->get('islandora.utils'),
      $container->get('file_url_generator'),
      $container->get('islandora.media_source_service')
    );
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
