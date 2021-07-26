<?php

/**
 * @file
 * Contains islandora.tokens.inc.
 *
 * This file provides islandora tokens.
 */

use Drupal\Core\Render\BubbleableMetadata;
use Drupal\media\Entity\Media;
use Drupal\file\Entity\File;

/**
 * Implements hook_token_info().
 */
function islandora_token_info() {
  $type = [
    'name' => t('Islandora Tokens'),
    'description' => t('Tokens for Islandora objects.'),
  ];
  $node['media-thumbnail-image:url'] = [
    'name' => t('Media: Thumbnail Image URL.'),
    'description' => t('URL of Thumbnail Image associated with Islandora Object via Media.'),
  ];

  $node['media-thumbnail-image:alt'] = [
    'name' => t('Alternative text for Media: Thumbnail Image.'),
    'description' => t('Alternative text for Thumbnail Image associated with Islandora Object via Media.'),
  ];

  // Deprecated in favour if hyphenated version.
  $node['media_thumbnail_image:url'] = [
    'name' => t('Media: Thumbnail Image URL.'),
    'description' => t('Deprecated: URL of Thumbnail Image associated with Islandora Object via Media.'),
  ];

  // Deprecated in favour if hyphenated version.
  $node['media_thumbnail_image:alt'] = [
    'name' => t('Alternative text for Media: Thumbnail Image.'),
    'description' => t('Deprecated: Alternative text for Thumbnail Image associated with Islandora Object via Media.'),
  ];

  $node['pdf_url'] = [
    'name' => t("PDF Url"),
    'description' => t('URL to related media file if "Original file" is a PDF file'),
  ];

  return [
    'types' => ['islandoratokens' => $type],
    'tokens' => ['islandoratokens' => $node],
  ];
}

/**
 * Implements hook_tokens().
 */
function islandora_tokens($type, $tokens, array $data, array $options, BubbleableMetadata $bubbleable_metadata) {
  $replacements = [];
  if ($type == 'islandoratokens' && !empty($data['node'])) {
    if (!is_array($tokens) || empty($tokens)) {
      \Drupal::logger('islandora')
        ->alert(
            'Tokens not correct format: @tokens', [
              '@tokens' => print_r($tokens, 1),
            ]
        );
      return;
    }
    $islandoraUtils = \Drupal::service('islandora.utils');
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'media-thumbnail-image:url':
        case 'media_thumbnail_image:url':
          $term = $islandoraUtils->getTermForUri('http://pcdm.org/use#ThumbnailImage');
          $media = $islandoraUtils->getMediaWithTerm($data['node'], $term);
          // Is there media?
          // @todo is this single or multiple?
          if ($media) {
            $file = \Drupal::service('islandora.media_source_service')->getSourceFile($media);
            if (!empty($file)) {
              $url = $file->url();
              $replacements[$original] = $url;
            }
          }
          break;

        case 'media-thumbnail-image:alt':
        case 'media_thumbnail_image:alt':
          $alt = '';
          $term = $islandoraUtils->getTermForUri('http://pcdm.org/use#ThumbnailImage');
          $media = $islandoraUtils->getMediaWithTerm($data['node'], $term);
          // Is there media?
          // @todo is this single or multiple?
          if ($media) {
            // Is the media an image?
            if (isset($media->field_media_image)) {
              $alt = $media->field_media_image[0]->alt;
            }
          }
          // @todo get alt from original or service file, if thumbnail
          // alt is empty.
          $replacements[$original] = $alt;
          break;

        case 'pdf_url':
          $replacements[$original] = ' ' . islandora_url_to_service_file_media_by_mimetype($data['node'], 'application/pdf');
          break;
      }
    }
  }
  return $replacements;
}

/**
 * Gets Original File PDF file URL.
 *
 * @param object $node
 *   A core drupal node object.
 * @param string $mime_type
 *   The name of the node's field to check for the specific relationship.
 *
 * @return string
 *   The tokenized value for the given data.
 */
function islandora_url_to_service_file_media_by_mimetype($node, $mime_type) {
  $islandora_utils = \Drupal::service('islandora.utils');
  $origfile_term = $islandora_utils->getTermForUri('http://pcdm.org/use#OriginalFile');
  $origfile_media = $islandora_utils->getMediaWithTerm($node, $origfile_term);
  // Get the media file's mime_type value.
  if (is_object($origfile_media)) {
    $origfile_mime_type = ($origfile_media->hasField('field_mime_type')) ?
      $origfile_media->get('field_mime_type')->getValue() : NULL;
    $origfile_mime_type = (is_array($origfile_mime_type) &&
      array_key_exists(0, $origfile_mime_type) &&
      is_array($origfile_mime_type[0]) &&
      array_key_exists('value', $origfile_mime_type[0])) ?
      $origfile_mime_type[0]['value'] : '';
    // Compare the media file's mime_type to the given value.
    if ($origfile_mime_type == $mime_type) {
      $vid = $origfile_media->id();
      if (!is_null($vid)) {
        $media = Media::load($vid);
        $bundle = $media->bundle();
        // Since this is Islandora and we assume the Original File is a
        // Document type... but doing it dynamically.
        $fid = $media->get('field_media_' . $bundle)->getValue();
        $fid_value = (is_array($fid) && array_key_exists(0, $fid) &&
          array_key_exists('target_id', $fid[0])) ?
          $fid[0]['target_id'] : NULL;
        if (!is_null($fid_value)) {
          $file = File::load($fid_value);
          if ($file) {
            $url = $islandora_utils->getDownloadUrl($file);
            return $url;
          }
        }
      }
    }
  }
}
