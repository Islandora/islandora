<?php

namespace Drupal\islandora\Form;

use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Entity\Form\DeleteMultipleForm;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Messenger\MessengerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\TempStore\PrivateTempStoreFactory;
use Drupal\Core\Url;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Psr\Log\LoggerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Confirmation form for the 'Delete node(s) and media' action.
 */
class ConfirmDeleteNodeAndMedia extends DeleteMultipleForm {

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSourceService;

  /**
   * Logger.
   *
   * @var Psr\Log\LoggerInterface
   */
  protected $logger;

  /**
   * Deleted media count.
   *
   * @var string
   */
  protected $deletedMediaCount = [];

  /**
   * Deleted file count.
   *
   * @var string
   */
  protected $deletedFileCount = [];

  /**
   * List of nodes targeted.
   *
   * @var array
   */
  protected $selection = [];

  /**
   * Entity field manager.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface
   */
  protected $entityFieldManager;

  /**
   * The Islandora Utils service.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected IslandoraUtils $utils;

  /**
   * {@inheritdoc}
   */
  public function __construct(AccountInterface $current_user, EntityTypeManagerInterface $entity_type_manager, EntityFieldManagerInterface $entity_field_manager, PrivateTempStoreFactory $temp_store_factory, MessengerInterface $messenger, IslandoraUtils $utils, MediaSourceService $media_source_service, LoggerInterface $logger) {
    $this->currentUser = $current_user;
    $this->entityTypeManager = $entity_type_manager;
    $this->entityFieldManager = $entity_field_manager;
    $this->tempStore = $temp_store_factory->get('node_and_media_delete_confirm');
    $this->messenger = $messenger;
    $this->utils = $utils;
    $this->mediaSourceService = $media_source_service;
    $this->logger = $logger;
    $this->deletedMediaCount = 0;
    $this->deletedFileCount = 0;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('current_user'),
      $container->get('entity_type.manager'),
      $container->get('entity_field.manager'),
      $container->get('tempstore.private'),
      $container->get('messenger'),
      $container->get('islandora.utils'),
      $container->get('islandora.media_source_service'),
      $container->get('logger.channel.islandora'));
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'node_and_media_delete_confirm_form';
  }

  /**
   * {@inheritdoc}
   */
  public function getQuestion() {
    return $this->formatPlural(count($this->selection),
      'Are you sure you want to delete this node and its associated media and files?',
      'Are you sure you want to delete these nodes and their associated media and files?');
  }

  /**
   * {@inheritdoc}
   */
  public function getCancelUrl() {
    return new Url('entity.media.collection');
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, $entity_type_id = NULL) {
    return parent::buildForm($form, $form_state, 'node');
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $deleted_media = 0;
    $node_storage = $this->entityTypeManager->getStorage('node');
    $nodes = $node_storage->loadMultiple(array_keys($this->selection));
    $deleteable_nodes = [];
    foreach ($nodes as $node) {
      if ($node->access('delete', $this->currentUser)) {
        $deleteable_nodes[] = $node;
      }
      else {
        $nondeleteable_nodes = $node;
      }
    }
    foreach ($deleteable_nodes as $candidate) {
      $media = $this->utils->getMedia($candidate);
      $this->utils->deleteMediaAndFiles($media);
      $candidate->delete();
    }
    $this->messenger->addStatus($this->getDeletedMessage(count($deleteable_nodes)));
    if ($nondeleteable_nodes) {
      $failures = count($nondeleteable_nodes);
      $this->messenger->addStatus($this->formatPlural($failures, 'Unable to delete 1 node', 'Unable to delete @count nodes'));
    }
    $this->tempStore->delete($this->currentUser->id());
    $form_state->setRedirectUrl($this->getCancelUrl());
  }

}
