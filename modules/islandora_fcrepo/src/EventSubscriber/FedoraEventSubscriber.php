<?php

namespace Drupal\islandora_fcrepo\EventSubscriber;

use Drupal\Core\Site\Settings;
use Drupal\Core\StreamWrapper\StreamWrapperManager;
use Drupal\file\FileInterface;
use Drupal\islandora\Event\GeneratedEventMessageEventInterface;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;

/**
 * Adds Fedora information to Islandora messages when Fedora is enabled.
 */
class FedoraEventSubscriber implements EventSubscriberInterface {

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents(): array {
    // Run before subscribers which rewrite outgoing URLs for microservices.
    return [GeneratedEventMessageEventInterface::EVENT_NAME => ['alterMessage', 100]];
  }

  /**
   * Adds the repository target and a file's Fedora URI.
   */
  public function alterMessage(GeneratedEventMessageEventInterface $event): void {
    $settings = Settings::get('flysystem', []);
    $message = $event->getMessage();
    $entity = $event->getEntity();
    $scheme = $entity instanceof FileInterface ? StreamWrapperManager::getScheme($entity->getFileUri()) : NULL;
    $entry = $settings['fedora'] ?? $settings['fcrepo'] ?? [];
    if (($settings[$scheme]['driver'] ?? NULL) === 'fedora') {
      $entry = $settings[$scheme];
    }
    if (($entry['driver'] ?? NULL) !== 'fedora') {
      foreach ($settings as $candidate) {
        if (($candidate['driver'] ?? NULL) === 'fedora') {
          $entry = $candidate;
          break;
        }
      }
    }
    $root = $entry['config']['root'] ?? NULL;
    if (!$root) {
      return;
    }
    $message['target'] = $root;
    if ($entity instanceof FileInterface && ($settings[$scheme]['driver'] ?? NULL) === 'fedora') {
      $message['attachment'] ??= ['type' => 'Object', 'content' => [], 'mediaType' => 'application/json'];
      $message['attachment']['content']['fedora_uri'] = rtrim($root, '/') . '/' . ltrim(StreamWrapperManager::getTarget($entity->getFileUri()), '/');
    }
    $event->setMessage($message);
  }

}
