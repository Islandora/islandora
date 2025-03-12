<?php

namespace Drupal\islandora_iiif;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\file\FileInterface;
use Drupal\jwt\Authentication\Provider\JwtAuth;
use GuzzleHttp\Client;
use GuzzleHttp\Exception\ClientException;
use GuzzleHttp\Exception\ConnectException;
use GuzzleHttp\Exception\RequestException;
use GuzzleHttp\Exception\ServerException;

/**
 * Get IIIF related info for a given File or Image entity.
 */
class IiifInfo {

  /**
   * The config factory.
   *
   * @var \Drupal\Core\Config\ConfigFactoryInterface
   */
  protected $configFactory;


  /**
   * The HTTP client.
   *
   * @var \GuzzleHttp\Client
   */
  protected $httpClient;

  /**
   * This module's config.
   *
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  protected $iiifConfig;

  /**
   * JWT Auth provider service.
   *
   * @var \Drupal\jwt\Authentication\Provider\JwtAuth
   */
  protected $jwtAuth;

  /**
   * The logger.
   *
   * @var \Drupal\Core\Logger\LoggerChannelInterface
   */
  protected $logger;

  /**
   * Constructs an IiifInfo object.
   *
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   The config factory.
   * @param \Guzzle\Http\Client $http_client
   *   The HTTP Client.
   * @param \Drupal\Core\Logger\LoggerChannelInterface $channel
   *   Logger channel.
   * @param \Drupal\jwt\Authentication\Provider\JwtAuth $jwt_auth
   *   The JWT auth provider.
   */
  public function __construct(ConfigFactoryInterface $config_factory, Client $http_client, LoggerChannelInterface $channel, JwtAuth $jwt_auth) {
    $this->configFactory = $config_factory;

    $this->iiifConfig = $this->configFactory->get('islandora_iiif.settings');
    $this->httpClient = $http_client;
    $this->logger = $channel;
    $this->jwtAuth = $jwt_auth;
  }

  /**
   * The IIIF base URL for an image.
   *
   * Visiting this URL will resolve to the info.json for the image.
   *
   * @return string
   *   The absolute URL on the IIIF server.
   */
  public function baseUrl($image) {

    if ($this->iiifConfig->get('use_relative_paths')) {
      $file_url = ltrim($image->createFileUrl(TRUE), '/');
    }
    else {
      $file_url = $image->createFileUrl(FALSE);
    }

    $iiif_address = $this->iiifConfig->get('iiif_server');
    $iiif_url = rtrim($iiif_address, '/') . '/' . urlencode($file_url);

    return $iiif_url;
  }

  /**
   * Retrieve an image's original dimensions via the IIIF server.
   *
   * @param \Drupal\File\FileInterface $file
   *   The image file.
   *
   * @return array|false
   *   The image dimensions in an array as [$width, $height]
   */
  public function getImageDimensions(FileInterface $file) {
    $iiif_url = $this->baseUrl($file);
    try {
      $info_json = $this->httpClient->request('get', $iiif_url, [
        'headers' => [
          'Authorization' => 'Bearer ' . $this->jwtAuth->generateToken(),
        ],
      ])->getBody();
      $resource = json_decode($info_json, TRUE);
      $width = $resource['width'];
      $height = $resource['height'];
      if (is_numeric($width) && is_numeric($height)) {
        return [intval($width), intval($height)];
      }
    }
    catch (ClientException | ConnectException | RequestException | ServerException $e) {
      $this->logger->info("Error getting image file dimensions from IIIF server: " . $e->getMessage());
    }
    return FALSE;
  }

  /**
   * The IIIF base URL for an image.
   *
   * Visiting this URL resolves to the image resized to the maximum dimensions.
   *
   * @param \Drupal\file\FileInterface $image
   *   The image entity.
   * @param int $width
   *   The maximum width of the image to be returned. 0 for no constraint.
   * @param int $height
   *   The maxim um height of the image to be returned. 0 for no contraint.
   *
   * @return string
   *   The IIIF URl to retrieve the full image with the given max dimensions.
   */
  public function getImageWithMaxDimensions(FileInterface $image, $width = 0, $height = 0) {
    $base_url = $this->baseUrl($image);
    return $base_url . "/full/!$width,$height/0/default.jpg";

  }

}
