<?php

namespace Drupal\islandora\Event;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Session\AccountInterface;

use Symfony\Component\HttpFoundation\ParameterBag;
use Symfony\Component\EventDispatcher\Event;

/**
 * Event used to build headers for STOMP.
 */
class StompHeaderEvent extends Event implements StompHeaderEventInterface {

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
   * An array of data to be sent with the STOMP request, for context.
   *
   * @var array
   */
  protected $data;

  /**
   * An array of configuration used to generate $data, for context.
   *
   * @var array
   */
  protected $configuration;

  /**
   * The set of headers.
   *
   * @var \Symfony\Component\HttpFoundation\ParameterBag
   */
  protected $headers;

  /**
   * Constructor.
   */
  public function __construct(EntityInterface $entity, AccountInterface $user, array $data, array $configuration) {
    $this->entity = $entity;
    $this->user = $user;
    $this->data = $data;
    $this->configuration = $configuration;
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
  public function getData() {
    return $this->data;
  }

  /**
   * {@inheritdoc}
   */
  public function getHeaders() {
    return $this->headers;
  }

  /**
   * {@inheritdoc}
   */
  public function getConfiguration() {
    return $this->configuration;
  }

}
