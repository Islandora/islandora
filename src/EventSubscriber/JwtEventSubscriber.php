<?php

namespace Drupal\islandora\EventSubscriber;

use Drupal\islandora\Form\IslandoraSettingsForm;
use Drupal\jwt\Authentication\Event\JwtAuthValidateEvent;
use Drupal\jwt\Authentication\Event\JwtAuthValidEvent;
use Drupal\jwt\Authentication\Event\JwtAuthGenerateEvent;
use Drupal\jwt\Authentication\Event\JwtAuthEvents;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Session\AccountInterface;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;

/**
 * Subscribes to Jwt Events.
 *
 * @package Drupal\islandora\EventSubscriber
 */
class JwtEventSubscriber implements EventSubscriberInterface {

  const AUDIENCE = 'islandora';

  /**
   * User storage to load users.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $userStorage;

  /**
   * The current user.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $currentUser;

  /**
   * Constructor.
   *
   * @param \Drupal\Core\Entity\EntityStorageInterface $userStorage
   *   User storage to load users.
   * @param \Drupal\Core\Session\AccountInterface $user
   *   The current user.
   */
  public function __construct(
    EntityStorageInterface $userStorage,
    AccountInterface $user
  ) {
    $this->userStorage = $userStorage;
    $this->currentUser = $user;
  }

  /**
   * Factory.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entityManager
   *   Entity manager to get user storage.
   * @param \Drupal\Core\Session\AccountInterface $user
   *   The current user.
   */
  public static function create(
    EntityTypeManagerInterface $entityManager,
    AccountInterface $user
  ) {
    return new static(
      $entityManager->getStorage('user'),
      $user
    );
  }

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents() {
    $events[JwtAuthEvents::VALIDATE][] = ['validate'];
    $events[JwtAuthEvents::VALID][] = ['loadUser'];
    $events[JwtAuthEvents::GENERATE][] = ['setIslandoraClaims'];

    return $events;
  }

  /**
   * Sets claims for a Islandora consumer on the JWT.
   *
   * @param \Drupal\jwt\Authentication\Event\JwtAuthGenerateEvent $event
   *   The event.
   */
  public function setIslandoraClaims(JwtAuthGenerateEvent $event) {
    global $base_secure_url;

    // Standard claims, validated at JWT validation time.
    $event->addClaim('iat', time());
    $expiry_setting = \Drupal::config(IslandoraSettingsForm::CONFIG_NAME)
      ->get(IslandoraSettingsForm::JWT_EXPIRY);
    $expiry = $expiry_setting ? $expiry_setting : '+2 hour';
    $event->addClaim('exp', strtotime($expiry));
    $event->addClaim('webid', $this->currentUser->id());
    $event->addClaim('iss', $base_secure_url);

    // Islandora claims we need to validate.
    $event->addClaim('sub', $this->currentUser->getAccountName());
    $event->addClaim('roles', $this->currentUser->getRoles(FALSE));

    $event->addClaim('aud', [static::AUDIENCE]);
  }

  /**
   * Validates that the Islandora data is present in the JWT.
   *
   * @param \Drupal\jwt\Authentication\Event\JwtAuthValidateEvent $event
   *   A JwtAuth event.
   */
  public function validate(JwtAuthValidateEvent $event) {
    $token = $event->getToken();

    $aud = $token->getClaim('aud');

    if (!$aud) {
      // Deprecation cycle: Avoid invalidating if there's no "aud" claim, to
      // allow tokens in flight before the introduction of this claim to remain
      // valid.
    }
    elseif (!in_array(static::AUDIENCE, $aud, TRUE)) {
      $event->invalidate('Missing audience entry.');
      return;
    }

    $uid = $token->getClaim('webid');
    $name = $token->getClaim('sub');
    $roles = $token->getClaim('roles');
    $url = $token->getClaim('iss');
    if ($uid === NULL || $name === NULL || $roles === NULL || $url === NULL) {
      $event->invalidate("Expected data missing from payload.");
      return;
    }

    $user = $this->userStorage->load($uid);
    if ($user === NULL) {
      $event->invalidate("Specified UID does not exist.");
    }
    elseif ($user->getAccountName() !== $name) {
      $event->invalidate("Account name does not match.");
    }
  }

  /**
   * Load and set a Drupal user to be authentication based on the JWT's uid.
   *
   * @param \Drupal\jwt\Authentication\Event\JwtAuthValidEvent $event
   *   A JwtAuth event.
   */
  public function loadUser(JwtAuthValidEvent $event) {
    $token = $event->getToken();
    $uid = $token->getClaim('webid');
    $user = $this->userStorage->load($uid);
    $event->setUser($user);
  }

}
