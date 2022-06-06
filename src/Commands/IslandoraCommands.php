<?php

namespace Drupal\islandora\Commands;

use Consolidation\AnnotatedCommand\CommandData;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Session\AccountProxyInterface;
use Drupal\Core\Session\AccountSwitcherInterface;
use Drupal\Core\Session\UserSession;
use Drush\Commands\DrushCommands;

/**
 * Adds a userid option to migrate:import.
 *
 * ... because the --user option was removed from drush 9.
 */
class IslandoraCommands extends DrushCommands {
  /**
   * Entity type manager object.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * Drupal\Core\Session\AccountProxy definition.
   *
   * @var \Drupal\Core\Session\AccountProxy
   */
  protected $currentUser;

  /**
   * The account switcher service.
   *
   * @var \Drupal\Core\Session\AccountSwitcherInterface
   */
  protected $accountSwitcher;

  /**
   * {@inheritdoc}
   */
  public function __construct(EntityTypeManagerInterface $entity_type_manager, AccountProxyInterface $current_user, AccountSwitcherInterface $account_switcher) {
    $this->entityTypeManager = $entity_type_manager;
    $this->currentUser = $current_user;
    $this->accountSwitcher = $account_switcher;
  }

  /**
   * Add the userid option.
   *
   * @hook option migrate:import
   * @option userid User ID to run the migration.
   */
  public function optionsetImportUser($options = ['userid' => self::REQ]) {
  }

  /**
   * Add the userid option.
   *
   * @hook option migrate:rollback
   * @option userid User ID to rollback the migration.
   */
  public function optionsetRollbackUser($options = ['userid' => self::REQ]) {
  }

  /**
   * Implement migrate import validate hook.
   *
   * @hook validate migrate:import
   */
  public function validateUserImport(CommandData $commandData) {
    $this->validateUser($commandData);
  }

  /**
   * Implement migrate rollback validate hook.
   *
   * @hook validate migrate:rollback
   */
  public function validateUserRollback(CommandData $commandData) {
    $this->validateUser($commandData);
  }

  /**
   * Validate the provided userid.
   */
  protected function validateUser(CommandData $commandData) {
    $userid = $commandData->input()->getOption('userid');
    if ($userid) {
      $account =
      $this->entityTypeManager->getStorage('user')->load($userid);
      if (!$account) {
        throw new \Exception("User ID does not match an existing user.");
      }
    }
  }

  /**
   * Implement migrate import pre-command hook.
   *
   * @hook pre-command migrate:import
   */
  public function preImport(CommandData $commandData) {
    $this->switchUser($commandData);
  }

  /**
   * Implement migrate rollback pre-command hook.
   *
   * @hook pre-command migrate:rollback
   */
  public function preRollback(CommandData $commandData) {
    $this->switchUser($commandData);
  }

  /**
   * Switch the active user account using the provided userid.
   */
  protected function switchUser(CommandData $commandData) {
    $userid = $commandData->input()->getOption('userid');
    if ($userid) {
      $account = $this->entityTypeManager->getStorage('user')->load($userid);
      $userSession = new UserSession([
        'uid'   => $account->id(),
        'name'  => $account->getDisplayName(),
        'roles' => $account->getRoles(),
      ]);
      $this->accountSwitcher->switchTo($userSession);
      $this->logger()->notice(
          dt(
              'Now acting as user ID @id',
              ['@id' => $this->currentUser->id()]
            )
      );
    }
  }

  /**
   * Implement migrate import post-command hook.
   *
   * @hook post-command migrate:import
   */
  public function postImport($result, CommandData $commandData) {
    $this->switchUserBack($commandData);
  }

  /**
   * Implement migrate rollback post-command hook.
   *
   * @hook post-command migrate:rollback
   */
  public function postRollback($result, CommandData $commandData) {
    $this->switchUserBack($commandData);
  }

  /**
   * Switch the user back.
   */
  protected function switchUserBack(CommandData $commandData) {
    if ($commandData->input()->getOption('userid')) {
      $this->logger()->notice(dt(
                                  'Switching back from user @uid.',
                                  ['@uid' => $this->currentUser->id()]
                                ));
      $this->accountSwitcher->switchBack();
    }
  }

}
