<?php

namespace Drupal\islandora\Plugin\Field\FieldType;

use Drupal\Core\Field\FieldDefinitionInterface;
use Drupal\Core\Field\FieldStorageDefinitionInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\StreamWrapper\StreamWrapperInterface;
use Drupal\Core\TypedData\DataDefinition;
use Drupal\file\Plugin\Field\FieldType\FileItem;

/**
 * Plugin implementation of the 'media_track' field type.
 *
 * @FieldType(
 *   id = "media_track",
 *   label = @Translation("Media track"),
 *   description = @Translation("This field stores the ID of a media track file as an integer value."),
 *   category = "reference",
 *   default_widget = "media_track",
 *   default_formatter = "file_default",
 *   column_groups = {
 *     "file" = {
 *       "label" = @Translation("File"),
 *       "columns" = {
 *         "target_id"
 *       },
 *       "require_all_groups_for_translation" = TRUE
 *     },
 *     "label" = {
 *       "label" = @Translation("Track label"),
 *       "translatable" = FALSE,
 *     },
 *     "kind" = {
 *       "label" = @Translation("Kind"),
 *       "translatable" = FALSE
 *     },
 *     "srclang" = {
 *       "label" = @Translation("SRC Language"),
 *       "translatable" = FALSE
 *     },
 *     "default" = {
 *       "label" = @Translation("Default"),
 *       "translatable" = FALSE
 *     },
 *   },
 *   list_class = "\Drupal\file\Plugin\Field\FieldType\FileFieldItemList",
 *   constraints = {"ReferenceAccess" = {}, "FileValidation" = {}}
 * )
 */
class MediaTrackItem extends FileItem {

  /**
   * {@inheritdoc}
   */
  public static function defaultFieldSettings() {
    $settings = [
      'file_extensions' => 'vtt',
      'languages' => 'installed',
    ] + parent::defaultFieldSettings();

    unset($settings['description_field']);
    return $settings;
  }

  /**
   * {@inheritdoc}
   */
  public static function schema(FieldStorageDefinitionInterface $field_definition) {
    return [
      'columns' => [
        'target_id' => [
          'description' => 'The ID of the file entity.',
          'type' => 'int',
          'unsigned' => TRUE,
        ],
        'label' => [
          'description' => "Label of track, for the track's 'label' attribute.",
          'type' => 'varchar',
          'length' => 128,
        ],
        'kind' => [
          'description' => "Type of track, for the track's 'kind' attribute.",
          'type' => 'varchar',
          'length' => 20,
        ],
        'srclang' => [
          'description' => "Language of track, for the track's 'srclang' attribute.",
          'type' => 'varchar',
          'length' => 20,
        ],
        'default' => [
          'description' => "Flag to indicate whether to use this as the default track of this kind.",
          'type' => 'int',
          'size' => 'tiny',
          'unsigned' => TRUE,
          'default' => 0,
        ],
      ],
      'indexes' => [
        'target_id' => ['target_id'],
      ],
      'foreign keys' => [
        'target_id' => [
          'table' => 'file_managed',
          'columns' => ['target_id' => 'fid'],
        ],
      ],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public static function propertyDefinitions(FieldStorageDefinitionInterface $field_definition) {
    $properties = parent::propertyDefinitions($field_definition);

    unset($properties['display']);
    unset($properties['description']);

    $properties['label'] = DataDefinition::create('string')
      ->setLabel(t('Label'))
      ->setDescription(t("Label of track, for the track's 'label' attribute."));

    $properties['kind'] = DataDefinition::create('string')
      ->setLabel(t('Track kind'))
      ->setDescription(t("Type of track, for the track's 'kind' attribute."));

    $properties['srclang'] = DataDefinition::create('string')
      ->setLabel(t('SRC Language'))
      ->setDescription(t("Language of track, for the track's 'srclang' attribute."));

    $properties['default'] = DataDefinition::create('boolean')
      ->setLabel(t('Default'))
      ->setDescription(t("Flag to indicate whether to use this as the default track of this kind."));

    return $properties;
  }

  /**
   * {@inheritdoc}
   */
  public function storageSettingsForm(array &$form, FormStateInterface $form_state, $has_data) {
    $element = [];

    $scheme_options = \Drupal::service('stream_wrapper_manager')->getNames(StreamWrapperInterface::WRITE_VISIBLE);
    $element['uri_scheme'] = [
      '#type' => 'radios',
      '#title' => t('Upload destination'),
      '#options' => $scheme_options,
      '#default_value' => $this->getSetting('uri_scheme'),
      '#description' => t('Select where the final files should be stored. Private file storage has significantly more overhead than public files, but allows restricted access to files within this field.'),
    ];

    return $element;
  }

  /**
   * {@inheritdoc}
   */
  public function fieldSettingsForm(array $form, FormStateInterface $form_state) {
    // Get base form from FileItem.
    $element = parent::fieldSettingsForm($form, $form_state);
    $settings = $this->getSettings();

    // Remove the description option.
    unset($element['description_field']);

    $element['languages'] = [
      '#type' => 'radios',
      '#title' => $this->t('List all languages'),
      '#description' => $this->t('Allow the user to select all languages or only the currently installed languages?'),
      '#options' => [
        'all' => $this->t('All Languages'),
        'installed' => $this->t('Currently Installed Languages'),
      ],
      '#default_value' => $settings['languages'] ?: 'installed',
    ];

    return $element;
  }

  /**
   * {@inheritdoc}
   */
  public static function generateSampleValue(FieldDefinitionInterface $field_definition) {
    $sample_file_text = <<<EOT
WEBVTT

00:00:00.500 --> 00:00:02.000
This is a test subtitle.

00:00:04.500 --> 00:00:06.300
This is another test subtitle.
EOT;

    /** @var \Drupal\file\FileRepositoryInterface $file_repo */
    $file_repo = \Drupal::service('file.repository');
    $file = $file_repo->writeData($sample_file_text, "public://sample_file.vtt");

    $values = [
      'target_id' => $file->id(),
      // Randomize the rest of these...
      'label' => '',
      'kind' => '',
      'srclang' => '',
      // Careful with this one, complex validation.
      'default' => 0,
    ];
    return $values;
  }

  /**
   * {@inheritdoc}
   */
  public function isDisplayed() {
    return TRUE;
  }

}
