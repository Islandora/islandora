<?php

namespace Drupal\islandora\Event;

/**
 * Contract for mutating generated Islandora event messages.
 */
interface GeneratedEventMessageEventInterface {

  const EVENT_NAME = 'islandora.generated_event_message';

  /**
   * Gets the generated event message array.
   *
   * @return array
   *   The generated event message array.
   */
  public function getMessage();

  /**
   * Sets the generated event message array.
   *
   * @param array $message
   *   The generated event message array.
   *
   * @return $this
   *   The current event.
   */
  public function setMessage(array $message);

  /**
   * Gets the source entity for the generated message.
   *
   * @return \Drupal\Core\Entity\EntityInterface
   *   The source entity.
   */
  public function getEntity();

  /**
   * Gets the acting user for the generated message.
   *
   * @return \Drupal\user\UserInterface
   *   The acting user.
   */
  public function getUser();

  /**
   * Gets the filtered event data payload.
   *
   * @return array
   *   The filtered event data payload.
   */
  public function getData();

}
