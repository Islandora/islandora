<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Component\Plugin\PluginManagerInterface;
use Drupal\Core\Field\FieldDefinitionInterface;
use Drupal\Core\Field\WidgetInterface;

/**
 * Wizard/widget lookup helper trait.
 */
trait WizardTrait {

  use FieldTrait;

  /**
   * The widget plugin manager service.
   *
   * @var \Drupal\Core\Field\WidgetPluginManager
   */
  protected PluginManagerInterface $widgetPluginManager;

  /**
   * Helper; get the base widget for the given field.
   *
   * @param \Drupal\Core\Field\FieldDefinitionInterface $field
   *   The field for which get obtain the widget.
   *
   * @return \Drupal\Core\Field\WidgetInterface
   *   The widget.
   */
  protected function getWidget(FieldDefinitionInterface $field): WidgetInterface {
    return $this->widgetPluginManager->getInstance([
      'field_definition' => $field,
      'form_mode' => 'default',
      'prepare' => TRUE,
    ]);
  }

}
