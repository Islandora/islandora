<?php

namespace Drupal\islandora\EventSubscriber;

use Drupal\islandora\Event\StompHeaderEventException;
use Drupal\islandora\Event\StompHeaderEventInterface;
use Drupal\jwt\Authentication\Provider\JwtAuth;

use Drupal\Core\StringTranslation\StringTranslationTrait;

use Symfony\Component\EventDispatcher\EventSubscriberInterface;

/**
 * Base STOMP header listener.
 */
class StompHeaderEventSubscriber implements EventSubscriberInterface {

  use StringTranslationTrait;

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
      StompHeaderEventInterface::EVENT_NAME => ['baseHeaders', -100],
    ];
  }

  /**
   * Event callback; generate and add base/default headers if not set.
   */
  public function baseHeaders(StompHeaderEventInterface $stomp_event) {
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

    // In ActiveMQ, STOMP messages are not persistent by default; however, we
    // would like them to persist, by default... make it so, unless something
    // else has already set the header.
    if (!$headers->has('persistent')) {
      $headers->set('persistent', 'true');
    }

  }

}
