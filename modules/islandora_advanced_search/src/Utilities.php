<?php

namespace Drupal\islandora_advanced_search;

use Drupal\islandora_advanced_search\Plugin\Block\AdvancedSearchBlock;
use Drupal\islandora_advanced_search\Plugin\Block\SearchResultsPagerBlock;

/**
 * Helper functions.
 */
class Utilities {

  /**
   * Gets the list of views for which pager blocks have been created.
   *
   * @return array
   *   List of view and display ids which have that have been used to
   *   derive a SearchResultsPagerBlock.
   */
  public static function getPagerViewDisplays() {
    $views = &drupal_static(__FUNCTION__);
    if (!isset($views)) {
      $block_storage = \Drupal::entityTypeManager()->getStorage('block');
      $active_theme = \Drupal::theme()->getActiveTheme();
      $views = [];
      /** @var \Drupal\block\Entity\Block $block */
      foreach ($block_storage->loadByProperties(['theme' => $active_theme->getName()]) as $block) {
        $plugin = $block->getPlugin();
        if ($plugin instanceof SearchResultsPagerBlock) {
          [$view_id, $display_id] = $plugin->getViewAndDisplayIdentifiers();
          $views[$block->id()] = [$view_id, $display_id];
        }
      }
    }
    return $views;
  }

  /**
   * Gets the list of views for which advanced search blocks have been created.
   *
   * @return array
   *   List of view and display ids which have that have been used to
   *   derive a SearchResultsPagerBlock.
   */
  public static function getAdvancedSearchViewDisplays() {
    $views = &drupal_static(__FUNCTION__);
    if (!isset($views)) {
      $block_storage = \Drupal::entityTypeManager()->getStorage('block');
      $active_theme = \Drupal::theme()->getActiveTheme();
      $views = [];
      /** @var \Drupal\block\Entity\Block $block */
      foreach ($block_storage->loadByProperties(['theme' => $active_theme->getName()]) as $block) {
        $plugin = $block->getPlugin();
        if ($plugin instanceof AdvancedSearchBlock) {
          [$view_id, $display_id] = $plugin->getViewAndDisplayIdentifiers();
          $views[$block->id()] = [$view_id, $display_id];
        }
      }
    }
    return $views;
  }

}
