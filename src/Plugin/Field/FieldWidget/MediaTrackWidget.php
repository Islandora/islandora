<?php

namespace Drupal\islandora\Plugin\Field\FieldWidget;

use Drupal\Core\Field\Attribute\FieldWidget;
use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Language\LanguageManager;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\file\Plugin\Field\FieldWidget\FileWidget;

/**
 * Plugin implementation of the 'media_track' widget.
 */
#[FieldWidget(
  id: "media_track",
  label: new TranslatableMarkup("Media Track"),
  field_types: ["media_track"]
)]
class MediaTrackWidget extends FileWidget {

  /**
   * {@inheritdoc}
   */
  public static function defaultSettings() {
    return [
      'progress_indicator' => 'throbber',
    ] + parent::defaultSettings();
  }

  /**
   * Overrides FileWidget::formMultipleElements().
   *
   * Special handling for draggable multiple widgets and 'add more' button.
   */
  protected function formMultipleElements(FieldItemListInterface $items, array &$form, FormStateInterface $form_state) {
    $elements = parent::formMultipleElements($items, $form, $form_state);

    $cardinality = $this->fieldDefinition->getFieldStorageDefinition()->getCardinality();
    $file_upload_help = [
      '#theme' => 'file_upload_help',
      '#description' => '',
      '#upload_validators' => $elements[0]['#upload_validators'],
      '#cardinality' => $cardinality,
    ];
    if ($cardinality == 1) {
      // If there's only one field, return it as delta 0.
      if (empty($elements[0]['#default_value']['fids'])) {
        $file_upload_help['#description'] = $this->getFilteredDescription();
        $elements[0]['#description'] = \Drupal::service('renderer')->renderPlain($file_upload_help);
      }
    }
    else {
      $elements['#file_upload_description'] = $file_upload_help;
    }

    return $elements;
  }

  /**
   * {@inheritDoc}
   *
   * Add the field settings so they can be used in the process method.
   */
  public function formElement(FieldItemListInterface $items, $delta, array $element, array &$form, FormStateInterface $form_state) {
    $element = parent::formElement($items, $delta, $element, $form, $form_state);
    $element['#field_settings'] = $this->getFieldSettings();

    return $element;
  }

  /**
   * Form API callback: Processes a media_track field element.
   *
   * Expands the media_track type to include the alt and title fields.
   *
   * This method is assigned as a #process callback in formElement() method.
   */
  public static function process($element, FormStateInterface $form_state, $form) {
    $item = $element['#value'];
    $item['fids'] = $element['fids']['#value'];

    $element['label'] = [
      '#title' => t('Label'),
      '#type' => 'textfield',
      '#default_value' => $item['label'] ?? '',
      '#description' => t('Label for the track file.'),
      '#maxlength' => 128,
      '#access' => (bool) $item['fids'],
    ];
    $element['kind'] = [
      '#title' => t('Kind'),
      '#type' => 'select',
      '#description' => t('The kind of media track.'),
      '#options' => [
        'subtitles' => t('Subtitles'),
        'captions' => t('Captions'),
        'descriptions' => t('Descriptions'),
        'chapters' => t('Chapters'),
        'metadata' => t('Metadata'),
      ],
      '#default_value' => $item['kind'] ?? '',
      '#access' => (bool) $item['fids'],
    ];

    $srclang_options = [];
    if ($element['#field_settings']['languages'] == 'all') {
      // Need to list all languages.
      $languages = LanguageManager::getStandardLanguageList();
      foreach ($languages as $key => $language) {
        if ($language[0] == $language[1]) {
          // Both the native language name and the English language name are
          // the same, so only show one of them.
          $srclang_options[$key] = $language[0];
        }
        else {
          // The native language name and English language name are different
          // so show both of them.
          $srclang_options[$key] = t('@lang0 / @lang1', [
            '@lang0' => $language[0],
            '@lang1' => $language[1],
          ]);
        }
      }
    }
    else {
      // Only list the installed languages.
      $languages = \Drupal::languageManager()->getLanguages();
      foreach ($languages as $key => $language) {
        $srclang_options[$key] = $language->getName();
      }
    }

    $element['srclang'] = [
      '#title' => t('SRC Language'),
      '#description' => t('Choose from one of the installed languages.'),
      '#type' => 'select',
      '#options' => $srclang_options,
      '#default_value' => $item['srclang'] ?? '',
      '#maxlength' => 20,
      '#access' => (bool) $item['fids'],
      '#element_validate' => [[get_called_class(), 'validateRequiredFields']],
    ];
    // @see https://www.w3.org/TR/html/semantics-embedded-content.html#elementdef-track
    $element['default'] = [
      '#type' => 'checkbox',
      '#title' => t('Default track'),
      '#default_value' => $item['default'] ?? '',
      '#description' => t('Use this as the default track of this kind.'),
      '#access' => (bool) $item['fids'],
    ];

    return parent::process($element, $form_state, $form);
  }

  /**
   * Validate callback for kind/srclang/label/default.
   *
   * This is separated in a validate function instead of a #required flag to
   * avoid being validated on the process callback.
   * The 'default' track has complex validation, see HTML5.2 for details.
   */
  public static function validateRequiredFields($element, FormStateInterface $form_state) {
    // Only do validation if the function is triggered from other places than
    // the image process form.
    $triggering_element = $form_state->getTriggeringElement();
    if (!empty($triggering_element['#submit']) && in_array('file_managed_file_submit', $triggering_element['#submit'], TRUE)) {
      $form_state->setLimitValidationErrors([]);
    }
  }

}
