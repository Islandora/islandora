<?php

namespace Drupal\islandora\Form;

use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Entity\Form\DeleteMultipleForm;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Messenger\MessengerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\TempStore\PrivateTempStoreFactory;
use Drupal\Core\Url;
use Drupal\islandora\IslandoraUtils;
use Psr\Log\LoggerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Confirmation form for the 'Delete media and file(s)' action.
 */
class ConfirmDeleteMediaAndFile extends DeleteMultipleForm {

  /**
   * The current user.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $currentUser;

  /**
   * Logger.
   *
   * @var Psr\Log\LoggerInterface
   */
  protected $logger;

  /**
   * List of media targeted.
   *
   * @var array
   */
  protected $selection = [];

  /**
   * The Islandora Utils service.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected IslandoraUtils $utils;

  /**
   * {@inheritdoc}
   */
  public function __construct(AccountInterface $current_user, EntityTypeManagerInterface $entity_type_manager, PrivateTempStoreFactory $temp_store_factory, MessengerInterface $messenger, LoggerInterface $logger, IslandoraUtils $utils) {
    $this->currentUser = $current_user;
    $this->entityTypeManager = $entity_type_manager;
    $this->tempStore = $temp_store_factory->get('media_and_file_delete_confirm');
    $this->messenger = $messenger;
    $this->logger = $logger;
    $this->utils = $utils;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('current_user'),
      $container->get('entity_type.manager'),
      $container->get('tempstore.private'),
      $container->get('messenger'),
      $container->get('logger.channel.islandora'),
      $container->get('islandora.utils')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'media_and_file_delete_confirm_form';
  }

  /**
   * {@inheritdoc}
   */
  public function getQuestion() {
    return $this->formatPlural(count($this->selection),
      'Are you sure you want to delete this media and associated files?',
      'Are you sure you want to delete these media and associated files?');
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
    return parent::buildForm($form, $form_state, 'media');
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    // Similar to parent::submitForm(), but let's blend in the related files and
    // optimize based on the fact that we know we're working with media.
    $media_storage = $this->entityTypeManager->getStorage('media');
    $media = $media_storage->loadMultiple(array_keys($this->selection));
    $results = $this->utils->deleteMediaAndFiles($media);
    $this->logger->notice($results['deleted']);
    $this->messenger->addStatus($results['deleted']);
    if (isset($results['inaccessible'])) {
      $this->messenger->addWarning($results['inaccessible']);
    }
    $this->tempStore->delete($this->currentUser->id());
    $form_state->setRedirectUrl($this->getCancelUrl());
  }

}
