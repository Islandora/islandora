<?php

namespace Drupal\islandora\Flysystem;

use Drupal\Core\Language\LanguageManagerInterface;
use Drupal\Core\Logger\RfcLogLevel;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Url;
use Drupal\flysystem\Plugin\FlysystemPluginInterface;
use Drupal\flysystem\Plugin\FlysystemUrlTrait;
use Drupal\islandora\Flysystem\Adapter\FedoraAdapter;
use Drupal\jwt\Authentication\Provider\JwtAuth;
use GuzzleHttp\Exception\ConnectException;
use GuzzleHttp\HandlerStack;
use GuzzleHttp\Client;
use Islandora\Chullo\IFedoraApi;
use Islandora\Chullo\FedoraApi;
use Psr\Http\Message\RequestInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\File\MimeType\MimeTypeGuesserInterface;

/**
 * Drupal plugin for the Fedora Flysystem adapter.
 *
 * @Adapter(id = "fedora")
 */
class Fedora implements FlysystemPluginInterface, ContainerFactoryPluginInterface {

  use FlysystemUrlTrait;

  /**
   * Fedora client.
   *
   * @var \Islandora\Chullo\IFedoraApi
   */
  protected $fedora;

  /**
   * Mimetype guesser.
   *
   * @var \Symfony\Component\HttpFoundation\File\Mimetype\MimeTypeGuesserInterface
   */
  protected $mimeTypeGuesser;

  /**
   * Language manager.
   *
   * @var \Drupal\Core\Language\LanguageManagerInterface
   */
  protected $languageManager;

  /**
   * Constructs a Fedora plugin for Flysystem.
   *
   * @param \Islandora\Chullo\IFedoraApi $fedora
   *   Fedora client.
   * @param \Symfony\Component\HttpFoundation\File\Mimetype\MimeTypeGuesserInterface $mime_type_guesser
   *   Mimetype guesser.
   * @param \Drupal\Core\Language\LanguageManagerInterface $language_manager
   *   Language manager.
   */
  public function __construct(
    IFedoraApi $fedora,
    MimeTypeGuesserInterface $mime_type_guesser,
    LanguageManagerInterface $language_manager
  ) {
    $this->fedora = $fedora;
    $this->mimeTypeGuesser = $mime_type_guesser;
    $this->languageManager = $language_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {

    // Construct guzzle client to middleware that adds JWT.
    $stack = HandlerStack::create();
    $stack->push(static::addJwt($container->get('jwt.authentication.jwt')));
    $client = new Client([
      'handler' => $stack,
      'base_uri' => $configuration['root'],
    ]);
    $fedora = new FedoraApi($client);

    // Return it.
    return new static(
      $fedora,
      $container->get('file.mime_type.guesser'),
      $container->get('language_manager')
    );
  }

  /**
   * Guzzle middleware to add a header to outgoing requests.
   *
   * @param \Drupal\jwt\Authentication\Provider\JwtAuth $jwt
   *   JWT.
   */
  public static function addJwt(JwtAuth $jwt) {
    return function (callable $handler) use ($jwt) {
      return function (
        RequestInterface $request,
        array $options
      ) use (
        $handler,
        $jwt
      ) {
        $request = $request->withHeader('Authorization', 'Bearer ' . $jwt->generateToken());
        return $handler($request, $options);
      };
    };
  }

  /**
   * {@inheritdoc}
   */
  public function getAdapter() {
    return new FedoraAdapter($this->fedora, $this->mimeTypeGuesser);
  }

  /**
   * {@inheritdoc}
   */
  public function ensure($force = FALSE) {
    // Check fedora root for sanity.
    try {
      $response = $this->fedora->getResourceHeaders('');
      $statusCode = $response->getStatusCode();
      $message = '%url returned %status';
    }
    catch (ConnectException $e) {
      // Fedora is unavailable.
      $message = '%url is unavailable, cannot connect.';
      $statusCode = 500;
    }
    if ($statusCode != 200) {
      return [
        [
          'severity' => RfcLogLevel::ERROR,
          'message' => $message,
          'context' => [
            '%url' => $this->fedora->getBaseUri(),
            '%status' => $statusCode,
          ],
        ],
      ];
    }
    return [];
  }

  /**
   * {@inheritdoc}
   */
  public function getExternalUrl($uri) {
    $path = str_replace('\\', '/', $this->getTarget($uri));

    $arguments = [
      'scheme' => $this->getScheme($uri),
      'filepath' => $path,
    ];

    // Force file urls to be language neutral.
    $undefined = $this->languageManager->getLanguage('und');
    return Url::fromRoute(
      'flysystem.serve',
      $arguments,
      ['absolute' => TRUE, 'language' => $undefined]
    )->toString();
  }

}
