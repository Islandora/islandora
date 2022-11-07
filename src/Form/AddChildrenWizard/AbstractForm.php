<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Core\DependencyInjection\ClassResolverInterface;
use Drupal\Core\Form\FormBuilderInterface;
use Drupal\Core\Render\RendererInterface;
use Drupal\Core\Routing\RouteMatchInterface;
use Drupal\Core\Session\AccountProxyInterface;
use Drupal\Core\TempStore\SharedTempStoreFactory;
use Drupal\ctools\Wizard\FormWizardBase;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\EventDispatcher\EventDispatcherInterface;

/**
 * Bulk children addition wizard base form.
 */
abstract class AbstractForm extends FormWizardBase {

  const TEMPSTORE_ID = 'abstract.abstract';
  const TYPE_SELECTION_FORM = MediaTypeSelectionForm::class;
  const FILE_SELECTION_FORM = AbstractFileSelectionForm::class;

  /**
   * The Islandora Utils service.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected IslandoraUtils $utils;

  /**
   * The current node ID.
   *
   * @var mixed|null
   */
  protected $nodeId;

  /**
   * The current route match.
   *
   * @var \Drupal\Core\Routing\RouteMatchInterface
   */
  protected RouteMatchInterface $currentRoute;

  /**
   * The current user.
   *
   * @var \Drupal\Core\Session\AccountProxyInterface
   */
  protected AccountProxyInterface $currentUser;

  /**
   * Constructor.
   */
  public function __construct(
    SharedTempStoreFactory $tempstore,
    FormBuilderInterface $builder,
    ClassResolverInterface $class_resolver,
    EventDispatcherInterface $event_dispatcher,
    RouteMatchInterface $route_match,
    RendererInterface $renderer,
    $tempstore_id,
    AccountProxyInterface $current_user,
    $machine_name = NULL,
    $step = NULL
  ) {
    parent::__construct($tempstore, $builder, $class_resolver, $event_dispatcher, $route_match, $renderer, $tempstore_id,
      $machine_name, $step);

    $this->nodeId = $this->routeMatch->getParameter('node');
    $this->currentUser = $current_user;
  }

  /**
   * {@inheritdoc}
   */
  public static function getParameters() : array {
    return array_merge(
      parent::getParameters(),
      [
        'tempstore_id' => static::TEMPSTORE_ID,
        'current_user' => \Drupal::service('current_user'),
      ]
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getOperations($cached_values) {
    $ops = [];

    $ops['type_selection'] = [
      'title' => $this->t('Type Selection'),
      'form' => static::TYPE_SELECTION_FORM,
      'values' => [
        'node' => $this->nodeId,
      ],
    ];
    $ops['file_selection'] = [
      'title' => $this->t('Widget Input for Selected Type'),
      'form' => static::FILE_SELECTION_FORM,
      'values' => [
        'node' => $this->nodeId,
      ],
    ];

    return $ops;
  }

  /**
   * {@inheritdoc}
   */
  public function getNextParameters($cached_values) {
    return parent::getNextParameters($cached_values) + ['node' => $this->nodeId];
  }

  /**
   * {@inheritdoc}
   */
  public function getPreviousParameters($cached_values) {
    return parent::getPreviousParameters($cached_values) + ['node' => $this->nodeId];
  }

}
