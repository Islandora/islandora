<?php

namespace Drupal\Tests\islandora\Kernel;

use Prophecy\PhpUnit\ProphecyTrait;
use Drupal\jwt\Authentication\Event\JwtAuthGenerateEvent;
use Drupal\jwt\Authentication\Event\JwtAuthValidEvent;
use Drupal\jwt\Authentication\Event\JwtAuthValidateEvent;
use Drupal\jwt\JsonWebToken\JsonWebToken;
use Drupal\jwt\JsonWebToken\JsonWebTokenInterface;
use Drupal\Tests\user\Traits\UserCreationTrait;
use Drupal\core\Entity\EntityStorageInterface;
use Drupal\islandora\EventSubscriber\JwtEventSubscriber;

/**
 * JwtEventSubscriber tests.
 *
 * @group islandora
 * @coversDefaultClass \Drupal\islandora\EventSubscriber\JwtEventSubscriber
 */
class JwtEventSubscriberTest extends IslandoraKernelTestBase {

  use ProphecyTrait;
  use UserCreationTrait;

  /**
   * The current user.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $user;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    $this->user = $this->createUser();
  }

  /**
   * @covers \Drupal\islandora\EventSubscriber\JwtEventSubscriber::setIslandoraClaims
   */
  public function testGeneratesValidToken() {
    $entity_storage = $this->container->get('entity_type.manager')->getStorage('user');
    $subscriber = new JwtEventSubscriber($entity_storage, $this->user);

    // Generate a new token.
    $jwt = new JsonWebToken();
    $event = new JwtAuthGenerateEvent($jwt);
    $subscriber->setIslandoraClaims($event);

    // Validate it.
    $validateEvent = new JwtAuthValidateEvent($jwt);
    $subscriber->validate($validateEvent);

    $this->assertTrue($validateEvent->isValid(), "Generated tokens must be valid.");
  }

  /**
   * @covers \Drupal\islandora\EventSubscriber\JwtEventSubscriber::validate
   */
  public function testInvalidatesMalformedToken() {
    $entity_storage = $this->container->get('entity_type.manager')->getStorage('user');
    $subscriber = new JwtEventSubscriber($entity_storage, $this->user);

    // Create a new event with mock jwt that returns null for all functions.
    $prophecy = $this->prophesize(JsonWebTokenInterface::class);
    $jwt = $prophecy->reveal();
    $event = new JwtAuthValidateEvent($jwt);

    $subscriber->validate($event);

    $this->assertFalse($event->isValid(), "Malformed event must be invalidated");
  }

  /**
   * @covers \Drupal\islandora\EventSubscriber\JwtEventSubscriber::validate
   */
  public function testInvalidatesBadUid() {
    // Mock user entity storage, returns null when loading user.
    $prophecy = $this->prophesize(EntityStorageInterface::class);
    $entity_storage = $prophecy->reveal();

    $subscriber = new JwtEventSubscriber($entity_storage, $this->user);

    // Generate a new token.
    $jwt = new JsonWebToken();
    $event = new JwtAuthGenerateEvent($jwt);
    $subscriber->setIslandoraClaims($event);

    // Validate it.
    $validateEvent = new JwtAuthValidateEvent($jwt);
    $subscriber->validate($validateEvent);

    $this->assertFalse($validateEvent->isValid(), "Event must be invalidated when user cannot be loaded.");
  }

  /**
   * @covers \Drupal\islandora\EventSubscriber\JwtEventSubscriber::validate
   */
  public function testInvalidatesBadAccount() {
    $anotherUser = $this->createUser();

    // Mock user entity storage, loads the wrong user.
    $prophecy = $this->prophesize(EntityStorageInterface::class);
    $prophecy->load($this->user->id())->willReturn($anotherUser);
    $entity_storage = $prophecy->reveal();

    $subscriber = new JwtEventSubscriber($entity_storage, $this->user);

    // Generate a new token.
    $jwt = new JsonWebToken();
    $event = new JwtAuthGenerateEvent($jwt);
    $subscriber->setIslandoraClaims($event);

    // Validate it.
    $validateEvent = new JwtAuthValidateEvent($jwt);
    $subscriber->validate($validateEvent);

    $this->assertFalse($validateEvent->isValid(), "Event must be invalidated when users don't align.");
  }

  /**
   * @covers \Drupal\islandora\EventSubscriber\JwtEventSubscriber::loadUser
   */
  public function testLoadsUser() {
    $entity_storage = $this->container->get('entity_type.manager')->getStorage('user');
    $subscriber = new JwtEventSubscriber($entity_storage, $this->user);

    // Generate a new token.
    $jwt = new JsonWebToken();
    $event = new JwtAuthGenerateEvent($jwt);
    $subscriber->setIslandoraClaims($event);

    $validEvent = new JwtAuthValidEvent($jwt);
    $subscriber->loadUser($validEvent);

    $this->assertEquals($this->user->id(), $validEvent->getAccount()->id(), "Correct user must be loaded to valid event.");
  }

}
