<?php

namespace Drupal\islandora\Event;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Session\AccountInterface;

use Symfony\Component\HttpFoundation\ParameterBag;
use Symfony\Component\EventDispatcher\Event;

/**
 * Event used to build headers for STOMP.
 */
class StompHeaderEvent implements StompHeaderEventInterface {

  /**
   * Stashed entity, for context.
   *
   * @var \Drupal\Core\Entity\EntityInterface
   */
  protected $entity;

  /**
   * Stashed user info, for context.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $user;

  /**
   * The set of headers.
   *
   * @var \Symfony\Component\HttpFoundation\ParameterBag
   */
  protected $headers;

  /**
   * Constructor.
   */
  public function __construct(EntityInterface $entity, AccountInterface $user) {
    $this->entity = $entity;
    $this->user = $user;
    $this->headers = new ParameterBag();
  }

  /**
   * {@inheritdoc}
   */
  public function getEntity() {
    return $this->entity;
  }

  /**
   * {@inheritdoc}
   */
  public function getUser() {
    return $this->user;
  }

  /**
   * {@inheritdoc}
   */
  public function getHeaders() {
    return $this->headers;
  }

}
