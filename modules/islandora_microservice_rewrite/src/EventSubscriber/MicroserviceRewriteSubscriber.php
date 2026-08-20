<?php

namespace Drupal\islandora_microservice_rewrite\EventSubscriber;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\islandora\Event\GeneratedEventMessageEventInterface;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;

/**
 * Rewrites microservice message URIs based on submodule configuration.
 */
class MicroserviceRewriteSubscriber implements EventSubscriberInterface {

  /**
   * The submodule configuration factory.
   *
   * Injected now so Chunk 4 only adds behavior, not new service wiring.
   *
   * @var \Drupal\Core\Config\ConfigFactoryInterface
   */
  protected $configFactory;

  /**
   * Constructs the rewrite subscriber scaffold.
   */
  public function __construct(ConfigFactoryInterface $config_factory) {
    $this->configFactory = $config_factory;
  }

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents() {
    return [
      GeneratedEventMessageEventInterface::EVENT_NAME => 'rewriteMessageUris',
    ];
  }

  /**
   * Rewrites configured URI fields in the generated message payload.
   *
   * @param \Drupal\islandora\Event\GeneratedEventMessageEventInterface $event
   *   The generated message event.
   */
  public function rewriteMessageUris(GeneratedEventMessageEventInterface $event) {
    $rules = $this->configFactory
      ->get('islandora_microservice_rewrite.settings')
      ->get('rewrite_rules');

    if (empty($rules)) {
      return;
    }

    [$find, $replace] = $this->parseRewriteRules($rules);
    if (empty($find)) {
      return;
    }

    $message = $event->getMessage();
    if (!empty($message['attachment']['content']) && is_array($message['attachment']['content'])) {
      foreach (['file_upload_uri', 'source_uri', 'destination_uri'] as $field) {
        if (isset($message['attachment']['content'][$field])) {
          $message['attachment']['content'][$field] = str_replace(
            $find,
            $replace,
            $message['attachment']['content'][$field]
          );
        }
      }
    }

    $this->rewriteLinkUrls($message, $find, $replace);

    $event->setMessage($message);
  }

  /**
   * Parses newline-delimited rewrite rules into find/replace arrays.
   *
   * @param string $rules
   *   The configured rewrite rules.
   *
   * @return array
   *   A two-item array of find and replace values.
   */
  protected function parseRewriteRules($rules) {
    $find = [];
    $replace = [];

    foreach (preg_split('/\r\n|\r|\n/', $rules) as $line) {
      $line = trim($line);
      if ($line === '') {
        continue;
      }

      $parts = explode('|', $line, 2);
      if (count($parts) !== 2) {
        continue;
      }

      $find[] = trim($parts[0]);
      $replace[] = trim($parts[1]);
    }

    return [$find, $replace];
  }

  /**
   * Rewrites ActivityStreams link URLs in a generated event message.
   *
   * @param array $message
   *   The generated event message.
   * @param array $find
   *   The configured URL fragments to replace.
   * @param array $replace
   *   The replacement URL fragments.
   */
  protected function rewriteLinkUrls(array &$message, array $find, array $replace) {
    foreach (['actor', 'object'] as $section) {
      if (empty($message[$section]['url']) || !is_array($message[$section]['url'])) {
        continue;
      }

      foreach ($message[$section]['url'] as &$link) {
        if (is_array($link) && isset($link['href'])) {
          $link['href'] = str_replace($find, $replace, $link['href']);
        }
      }
    }
  }

}
