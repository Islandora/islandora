<?php

namespace Drupal\islandora\EventSubscriber;

use Drupal\islandora\Event\StompHeaderEventInterface;
use Drupal\jwt\Authentication\Provider\JwtAuth;

use Drupal\Core\Messenger\MessengerInterface;

use Psr\Log\LoggerInterface;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;

/**
 * Base STOMP header listener.
 */
class StompHeaderEventSubscriber implements EventSubscriberInterface {

  /**
   * The JWT auth service.
   *
   * @var \Drupal\jwt\Authentication\Provider\JwtAuth
   */
  protected $auth;

  /**
   * Constructor.
   */
  public function __construct(
    JwtAuth $auth
  ) {
    $this->auth = $auth;
  }

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents() {
    return [
      StompHeaderEventInterface::EVENT_NAME => 'baseAuth',
    ];
  }

  /**
   * Event callback; generate and add base authorization header if none is set.
   */
  public function baseAuth(StompHeaderEventInterface $stomp_event) {
    $headers = $stomp_event->getHeaders();
    if (!$headers->has('Authorization')) {
      $token = $this->auth->generateToken();
      if (empty($token)) {
        // JWT does not seem to be properly configured.
        // phpcs:ignore DrupalPractice.General.ExceptionT.ExceptionT
        throw new StompHeaderEventException($this->t('Error getting JWT token for message. Check JWT Configuration.'));
      }
      else {
        $headers->set('Authorization', "Bearer $token");
      }
    }

  }
}
