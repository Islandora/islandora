<?php

namespace Drupal\islandora_fits\Plugin\Field\FieldFormatter;

use Drupal\Core\Field\FieldItemInterface;
use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Field\FormatterBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\file\Entity\File;
use Drupal\Core\Link;
use Drupal\Core\Url;

/**
 * Plugin implementation of the 'fits_formatter' formatter.
 *
 * @FieldFormatter(
 *   id = "fits_formatter",
 *   label = @Translation("Fits formatter"),
 *   field_types = {
 *     "file"
 *   }
 * )
 */
class FitsFormatter extends FormatterBase {

  /**
   * {@inheritdoc}
   */
  public static function defaultSettings() {
    return [
        // Implement default settings.
      ] + parent::defaultSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array $form, FormStateInterface $form_state) {
    return [
        // Implement settings form.
      ] + parent::settingsForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function settingsSummary() {
    $summary = [];
    // Implement settings summary.
    return $summary;
  }

  /**
   * {@inheritdoc}
   */
  public function viewElements(FieldItemListInterface $items, $langcode) {
    $elements = [];

    foreach ($items as $delta => $item) {
      $elements[$delta] = ['#markup' => $this->viewValue($item)];
    }

    return $elements;
  }

  /**
   * Generate the output appropriate for one field item.
   *
   * @param \Drupal\Core\Field\FieldItemInterface $item
   *   One field item.
   *
   * @return string
   *   The textual output generated.
   */
  protected function viewValue(FieldItemInterface $item) {
    $transformer = \Drupal::getContainer()->get('islandora_fits.transformxml');
    $fileItem = $item->getValue();
    $file = File::load($fileItem['target_id']);
    $url = Url::fromUri(file_create_url($file->getFileUri()));
    $link = Link::fromTextAndUrl("Link to XML", $url);
    $link = $link->toRenderable();
    $contents = file_get_contents($file->getFileUri());
    if (mb_detect_encoding($contents) != 'UTF-8') {
      $contents = utf8_encode($contents);
    }
    $output = $transformer->transformFits($contents);
    $output['#link'] = $link;
    $output['#title'] = $this->t("FITS Metadata");
    return \Drupal::service('renderer')->render($output);
  }

}
