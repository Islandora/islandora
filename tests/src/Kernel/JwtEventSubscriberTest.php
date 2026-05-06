<?php

namespace Drupal\Tests\islandora\Kernel;

use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\islandora\EventSubscriber\JwtEventSubscriber;
use Drupal\jwt\Authentication\Event\JwtAuthGenerateEvent;
use Drupal\jwt\Authentication\Event\JwtAuthValidEvent;
use Drupal\jwt\Authentication\Event\JwtAuthValidateEvent;
use Drupal\jwt\JsonWebToken\JsonWebToken;
use Drupal\jwt\JsonWebToken\JsonWebTokenInterface;
use Drupal\Tests\user\Traits\UserCreationTrait;
use PHPUnit\Framework\Attributes\CoversClass;
use PHPUnit\Framework\Attributes\Group;
use Prophecy\PhpUnit\ProphecyTrait;

/**
 * JwtEventSubscriber tests.
 */
#[Group('islandora')]
#[CoversClass(JwtEventSubscriber::class)]
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
   * Tests that generated tokens are valid.
   */
  public function testGeneratesValidToken(): void {
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
   * Tests that malformed tokens are invalidated.
   */
  public function testInvalidatesMalformedToken(): void {
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
   * Tests that tokens with bad UIDs are invalidated.
   */
  public function testInvalidatesBadUid(): void {
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
   * Tests that tokens with bad accounts are invalidated.
   */
  public function testInvalidatesBadAccount(): void {
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
   * Tests that the correct user is loaded.
   */
  public function testLoadsUser(): void {
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
