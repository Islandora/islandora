<?php

namespace Drupal\Tests\islandora_fcrepo\Unit;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Site\Settings;
use Drupal\file\FileInterface;
use Drupal\islandora\Event\GeneratedEventMessageEvent;
use Drupal\islandora_fcrepo\EventSubscriber\FedoraEventSubscriber;
use Drupal\Tests\UnitTestCase;
use Drupal\user\UserInterface;
use PHPUnit\Framework\Attributes\Group;

/**
 * Checks that Fedora messages are supplied entirely by the optional module.
 */
#[Group('islandora_fcrepo')]
class FedoraEventSubscriberTest extends UnitTestCase {

  /**
   * Fedora URIs retain the configured scheme and normalized path.
   */
  public function testFedoraFile(): void {
    new Settings(['flysystem' => ['fcrepo' => ['driver' => 'fedora', 'config' => ['root' => 'http://fedora/rest/']]]]);
    $file = $this->createMock(FileInterface::class);
    $file->method('getFileUri')->willReturn('fcrepo:///path/file.txt');
    $event = new GeneratedEventMessageEvent(['type' => 'Update'], $file, $this->createMock(UserInterface::class), []);
    (new FedoraEventSubscriber())->alterMessage($event);
    $this->assertSame('http://fedora/rest/', $event->getMessage()['target']);
    $this->assertSame('http://fedora/rest/path/file.txt', $event->getMessage()['attachment']['content']['fedora_uri']);
  }

  /**
   * Indexing can target Fedora even when the source is stored elsewhere.
   */
  public function testPublicFileTarget(): void {
    new Settings(['flysystem' => ['fedora' => ['driver' => 'fedora', 'config' => ['root' => 'http://fedora/rest/']]]]);
    $file = $this->createMock(FileInterface::class);
    $file->method('getFileUri')->willReturn('public://file.txt');
    $event = new GeneratedEventMessageEvent([], $file, $this->createMock(UserInterface::class), []);
    (new FedoraEventSubscriber())->alterMessage($event);
    $this->assertSame(['target' => 'http://fedora/rest/'], $event->getMessage());
  }

  /**
   * Sites without a configured Fedora root receive no Fedora event fields.
   */
  public function testNoFedoraSettings(): void {
    new Settings([]);
    $event = new GeneratedEventMessageEvent(['type' => 'Update'], $this->createMock(EntityInterface::class), $this->createMock(UserInterface::class), []);
    (new FedoraEventSubscriber())->alterMessage($event);
    $this->assertSame(['type' => 'Update'], $event->getMessage());
  }

}
