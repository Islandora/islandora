<?php

namespace Drupal\islandora\Event;

use Drupal\Component\EventDispatcher\Event;
use Drupal\Core\Entity\EntityInterface;
use Drupal\user\UserInterface;

/**
 * Event used to alter generated Islandora event messages.
 */
class GeneratedEventMessageEvent extends Event implements GeneratedEventMessageEventInterface {

  /**
   * The generated event message array.
   *
   * @var array
   */
  protected $message;

  /**
   * The source entity for the generated message.
   *
   * @var \Drupal\Core\Entity\EntityInterface
   */
  protected $entity;

  /**
   * The acting user for the generated message.
   *
   * @var \Drupal\user\UserInterface
   */
  protected $user;

  /**
   * The filtered event data payload.
   *
   * @var array
   */
  protected $data;

  /**
   * Constructor.
   *
   * @param array $message
   *   The generated event message array.
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The source entity.
   * @param \Drupal\user\UserInterface $user
   *   The acting user.
   * @param array $data
   *   The filtered event data payload.
   */
  public function __construct(array $message, EntityInterface $entity, UserInterface $user, array $data) {
    $this->message = $message;
    $this->entity = $entity;
    $this->user = $user;
    $this->data = $data;
  }

  /**
   * {@inheritdoc}
   */
  public function getMessage() {
    return $this->message;
  }

  /**
   * {@inheritdoc}
   */
  public function setMessage(array $message) {
    $this->message = $message;
    return $this;
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

}
