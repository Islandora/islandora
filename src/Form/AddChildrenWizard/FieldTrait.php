<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Field\FieldDefinitionInterface;

/**
 * Field lookup helper trait.
 */
trait FieldTrait {

  use MediaTypeTrait;

  /**
   * The entity field manager service.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface|null
   */
  protected ?EntityFieldManagerInterface $entityFieldManager = NULL;

  /**
   * Helper; get field instance, given our required values.
   *
   * @param array $values
   *   See ::getMediaType() for which values are required.
   *
   * @return \Drupal\Core\Field\FieldDefinitionInterface
   *   The target field.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  protected function getField(array $values): FieldDefinitionInterface {
    $media_type = $this->getMediaType($values);
    $media_source = $media_type->getSource();
    $source_field = $media_source->getSourceFieldDefinition($media_type);

    $fields = $this->entityFieldManager()->getFieldDefinitions('media', $media_type->id());

    return $fields[$source_field->getFieldStorageDefinition()->getName()] ??
      $media_source->createSourceField($media_type);
  }

  /**
   * Lazy-initialization of the entity field manager service.
   *
   * @return \Drupal\Core\Entity\EntityFieldManagerInterface
   *   The entity field manager service.
   */
  protected function entityFieldManager() : EntityFieldManagerInterface {
    if ($this->entityFieldManager === NULL) {
      $this->setEntityFieldManager(\Drupal::service('entity_field.manager'));
    }
    return $this->entityFieldManager;
  }

  /**
   * Setter for entity field manager.
   */
  public function setEntityFieldManager(EntityFieldManagerInterface $entity_field_manager) : self {
    $this->entityFieldManager = $entity_field_manager;
    return $this;
  }

}
