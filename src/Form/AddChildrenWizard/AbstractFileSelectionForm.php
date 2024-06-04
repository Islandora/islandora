<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Core\Batch\BatchBuilder;
use Drupal\Core\Field\BaseFieldDefinition;
use Drupal\Core\Field\FieldDefinitionInterface;
use Drupal\Core\Field\FieldItemList;
use Drupal\Core\Field\FieldStorageDefinitionInterface;
use Drupal\Core\Field\WidgetInterface;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Session\AccountProxyInterface;
use Drupal\field\FieldStorageConfigInterface;
use Drupal\media\MediaTypeInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Children addition wizard's second step.
 */
abstract class AbstractFileSelectionForm extends FormBase {

  use WizardTrait;

  const BATCH_PROCESSOR = 'abstract.abstract';

  /**
   * The current user.
   *
   * @var \Drupal\Core\Session\AccountProxyInterface|null
   */
  protected ?AccountProxyInterface $currentUser;

  /**
   * The batch processor service.
   *
   * @var \Drupal\islandora\Form\AddChildrenWizard\AbstractBatchProcessor|null
   */
  protected ?AbstractBatchProcessor $batchProcessor;

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container): self {
    $instance = parent::create($container);

    $instance->entityTypeManager = $container->get('entity_type.manager');
    $instance->widgetPluginManager = $container->get('plugin.manager.field.widget');
    $instance->entityFieldManager = $container->get('entity_field.manager');
    $instance->currentUser = $container->get('current_user');

    $instance->batchProcessor = $container->get(static::BATCH_PROCESSOR);

    return $instance;
  }

  /**
   * Helper; get the media type, based off discovering from form state.
   *
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The form state.
   *
   * @return \Drupal\media\MediaTypeInterface
   *   The target media type.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  protected function getMediaTypeFromFormState(FormStateInterface $form_state): MediaTypeInterface {
    return $this->getMediaType($form_state->getTemporaryValue('wizard'));
  }

  /**
   * Helper; get field instance, based off discovering from form state.
   *
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The form state.
   *
   * @return \Drupal\Core\Field\FieldDefinitionInterface
   *   The field definition.
   */
  protected function getFieldFromFormState(FormStateInterface $form_state): FieldDefinitionInterface {
    $cached_values = $form_state->getTemporaryValue('wizard');

    $field = $this->getField($cached_values);
    $def = $field->getFieldStorageDefinition();
    if ($def instanceof FieldStorageConfigInterface) {
      $def->set('cardinality', FieldStorageDefinitionInterface::CARDINALITY_UNLIMITED);
    }
    elseif ($def instanceof BaseFieldDefinition) {
      $def->setCardinality(FieldStorageDefinitionInterface::CARDINALITY_UNLIMITED);
    }
    else {
      throw new \Exception('Unable to remove cardinality limit.');
    }

    return $field;
  }

  /**
   * Helper; get widget for the field, based on discovering from form state.
   *
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The form state.
   *
   * @return \Drupal\Core\Field\WidgetInterface
   *   The widget.
   */
  protected function getWidgetFromFormState(FormStateInterface $form_state): WidgetInterface {
    return $this->getWidget($this->getFieldFromFormState($form_state));
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state): array {
    // Using the media type selected in the previous step, grab the
    // media bundle's "source" field, and create a multi-file upload widget
    // for it, with the same kind of constraints.
    $field = $this->getFieldFromFormState($form_state);
    $items = FieldItemList::createInstance($field, $field->getName(), $this->getMediaTypeFromFormState($form_state)->getTypedData());

    $form['#tree'] = TRUE;
    $form['#parents'] = [];
    $widget = $this->getWidgetFromFormState($form_state);
    $form['files'] = $widget->form(
      $items,
      $form,
      $form_state
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $cached_values = $form_state->getTemporaryValue('wizard');

    $widget = $this->getWidgetFromFormState($form_state);
    $builder = (new BatchBuilder())
      ->setTitle($this->t('Bulk creating...'))
      ->setInitMessage($this->t('Initializing...'))
      ->setFinishCallback([$this, 'batchProcessFinished']);
    $values = $form_state->getValue($this->getField($cached_values)->getName());
    $massaged_values = $widget->massageFormValues($values, $form, $form_state);
    foreach ($massaged_values as $delta => $info) {
      $builder->addOperation(
        [$this, 'batchOperation'],
        [$delta, $info, $cached_values]
      );
    }
    batch_set($builder->toArray());
  }

  /**
   * Wrap batch processor operation call to side-step serialization issues.
   *
   * Previously, we referred to the method on the processors directly; however,
   * this can lead to issues regarding the (un)serialization of the services as
   * which the processors are implemented. For example, if decorating one of the
   * processors to extend it, it loses the reference back to be able to load the
   * "inner"/decorated processor.
   *
   * @see \Drupal\islandora\Form\AddChildrenWizard\AbstractBatchProcessor::batchOperation()
   */
  public function batchOperation($delta, $info, $cached_values, &$context) : void {
    $this->batchProcessor->batchOperation($delta, $info, $cached_values, $context);
  }

  /**
   * Wrap batch processor finished call to side-step serialization issues.
   *
   * Previously, we referred to the method on the processors directly; however,
   * this can lead to issues regarding the (un)serialization of the services as
   * which the processors are implemented. For example, if decorating one of the
   * processors to extend it, it loses the reference back to be able to load the
   * "inner"/decorated processor.
   *
   * @see \Drupal\islandora\Form\AddChildrenWizard\AbstractBatchProcessor::batchProcessFinished()
   */
  public function batchProcessFinished($success, $results, $operations) : void {
    $this->batchProcessor->batchProcessFinished($success, $results, $operations);
  }

}
