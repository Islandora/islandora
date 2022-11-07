<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\media\MediaTypeInterface;

/**
 * Media type lookup helper trait.
 */
trait MediaTypeTrait {

  /**
   * The entity type manager service.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface|null
   */
  protected ?EntityTypeManagerInterface $entityTypeManager = NULL;

  /**
   * Helper; get media type, given our required values.
   *
   * @param array $values
   *   An associative array which must contain at least:
   *   - media_type: The machine name of the media type to load.
   *
   * @return \Drupal\media\MediaTypeInterface
   *   The loaded media type.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  protected function getMediaType(array $values): MediaTypeInterface {
    return $this->entityTypeManager()->getStorage('media_type')->load($values['media_type']);
  }

  /**
   * Lazy-initialization of the entity type manager service.
   *
   * @return \Drupal\Core\Entity\EntityTypeManagerInterface
   *   The entity type manager service.
   */
  protected function entityTypeManager() : EntityTypeManagerInterface {
    if ($this->entityTypeManager === NULL) {
      $this->setEntityTypeManager(\Drupal::service('entity_type.manager'));
    }
    return $this->entityTypeManager;
  }

  /**
   * Setter for the entity type manager service.
   */
  public function setEntityTypeManager(EntityTypeManagerInterface $entity_type_manager) : self {
    $this->entityTypeManager = $entity_type_manager;
    return $this;
  }

}
