<?php

namespace Drupal\islandora_fcrepo\Plugin\Flysystem\Adapter;

use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\flysystem\Adapter\AdapterDriverPluginBase;
use Drupal\flysystem\Attribute\FlysystemAdapter;
use Drupal\flysystem\Exception\AdapterConfigurationException;
use Drupal\islandora_fcrepo\Flysystem\Adapter\FedoraAdapter;
use Drupal\jwt\Authentication\Provider\JwtAuth;
use GuzzleHttp\HandlerStack;
use Islandora\Chullo\FedoraApi;
use League\Flysystem\FilesystemAdapter;
use Psr\Http\Message\RequestInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Builds the Fedora adapter with the current user's JWT credentials.
 */
#[FlysystemAdapter(id: 'fedora', label: new TranslatableMarkup('Fedora'))]
class Fedora extends AdapterDriverPluginBase implements ContainerFactoryPluginInterface {

  /**
   * The service container.
   *
   * @var \Symfony\Component\DependencyInjection\ContainerInterface
   */
  protected $container;

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    $plugin = new static($configuration, $plugin_id, $plugin_definition);
    $plugin->container = $container;
    return $plugin;
  }

  /**
   * {@inheritdoc}
   */
  public function buildAdapter(array $config): FilesystemAdapter {
    if (empty($config['root']) || !filter_var($config['root'], FILTER_VALIDATE_URL) ||
      !in_array(parse_url($config['root'], PHP_URL_SCHEME), ['http', 'https'], TRUE)) {
      throw new AdapterConfigurationException('Fedora requires an HTTP(S) root URL.');
    }
    $stack = HandlerStack::create();
    $stack->push(static::addJwt($this->container->get('jwt.authentication.jwt')));
    return new FedoraAdapter(
      FedoraApi::createWithHandler($config['root'], $stack),
      $this->container->get('file.mime_type.guesser'),
      $this->container->get('logger.channel.fedora_flysystem'),
    );
  }

  /**
   * Adds a freshly generated JWT to each outgoing Fedora request.
   */
  public static function addJwt(JwtAuth $jwt) {
    return function (callable $handler) use ($jwt) {
      return function (RequestInterface $request, array $options) use ($handler, $jwt) {
        // Keep binary reads streaming; Drupal may consume only a byte range.
        if ($request->getMethod() === 'GET') {
          $options['stream'] = TRUE;
        }
        return $handler($request->withHeader('Authorization', 'Bearer ' . $jwt->generateToken()), $options);
      };
    };
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state, array $config): array {
    $form['root'] = [
      '#type' => 'url',
      '#title' => $this->t('Fedora root URL'),
      '#default_value' => $config['root'] ?? '',
      '#required' => TRUE,
    ];
    return $form;
  }

}
