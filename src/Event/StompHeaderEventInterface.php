<?php

namespace Drupal\islandora\Event;

/**
 * Contract for representing an event to build headers for STOMP messages.
 */
interface StompHeaderEventInterface {

  const EVENT_NAME = 'islandora.stomp.header_event';

  /**
   * Get the headers being built for STOMP.
   *
   * XXX: Ironically, using ParameterBag instead of HeaderBag due to case-
   * sensitivity: In the context of HTTP, headers are case insensitive (and is
   * what HeaderBag is intended; however, STOMP headers are case sensitive.
   *
   * @return \Symfony\Component\HttpFoundation\ParameterBag
   *   The headers
   */
  public function getHeaders();

  /**
   * Fetch the entity provided as context.
   *
   * @return \Drupal\Core\Entity\EntityInterface
   *   The entity provided as context.
   */
  public function getEntity();

  /**
   * Fetch the user provided as context.
   *
   * @return \Drupal\Core\Session\AccountInterface
   *   The user provided as context.
   */
  public function getUser();

  /**
   * Fetch the data to be sent in the body of the request.
   *
   * @return array
   *   The array of data.
   */
  public function getData();

  /**
   * Fetch the configuration of the action, for context.
   *
   * @return array
   *   The array of configuration for the upstream action.
   */
  public function getConfiguration();

}
