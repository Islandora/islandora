<?php

namespace Drupal\islandora_text_extraction;

use Drupal\islandora\IslandoraUtils;
use Drupal\media\MediaInterface;
use Psr\Log\LoggerInterface;

/**
 * Creates a GeminiClient as a Drupal service.
 *
 * @package Drupal\islandora
 */
class SearchReindexer {

  /**
   * Islandora Utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Logger.
   *
   * @var \Psr\Log\LoggerInterface
   */
  protected $logger;

  /**
   * Constructor.
   *
   * @param \Drupal\islandora\IslandoraUtils $utils
   *   Islandora utils.
   * @param \Psr\Log\LoggerInterface $logger
   *   The logger channel.
   */
  public function __construct(IslandoraUtils $utils, LoggerInterface $logger) {
    $this->utils = $utils;
    $this->logger = $logger;
  }

  /**
   * Reindexes parent node for a media. No-op if parent does not exist.
   *
   * @param Drupal\media\MediaInterface $media
   *   Media whose parent you want to reindex.
   */
  public function reindexParent(MediaInterface $media) {
    $parent = $this->utils->getParentNode($media);

    if ($parent === NULL) {
      return;
    }

    $this->logger->debug(
      "Re-indexing parent node @nid for extracted text @mid using the search_api",
      ['@nid' => $parent->id(), '@mid' => $media->id()]
    );

    $parent->original = $parent;
    search_api_entity_update($parent);
  }

}
