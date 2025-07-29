<?php

namespace Drupal\islandora\Flysystem\Adapter;

use GuzzleHttp\Psr7\Header;
use Drupal\Core\Logger\LoggerChannelInterface;
use Islandora\Chullo\IFedoraApi;
use League\Flysystem\AdapterInterface;
use League\Flysystem\Adapter\Polyfill\NotSupportingVisibilityTrait;
use League\Flysystem\Adapter\Polyfill\StreamedCopyTrait;
use League\Flysystem\Config;
use GuzzleHttp\Psr7\Response;
use GuzzleHttp\Psr7\StreamWrapper;
use Symfony\Component\Mime\MimeTypeGuesserInterface;
use Symfony\Component\HttpFoundation\Request;

/**
 * Fedora adapter for Flysystem.
 */
class FedoraAdapter implements AdapterInterface {

  use StreamedCopyTrait;
  use NotSupportingVisibilityTrait;

  /**
   * Fedora client.
   *
   * @var \Islandora\Chullo\IFedoraApi
   */
  protected $fedora;

  /**
   * Mimetype guesser.
   *
   * @var \Symfony\Component\Mime\MimeTypeGuesserInterface
   */
  protected $mimeTypeGuesser;

  /**
   * Logger.
   *
   * @var \Drupal\Core\Logger\LoggerChannelInterface
   */
  protected $logger;

  /**
   * The current request.
   *
   * @var \Symfony\Component\HttpFoundation\Request
   */
  protected $request;

  /**
   * The path to fedora OCFL root.
   *
   * @var string
   */
  protected $fedoraRoot;

  /**
   * Constructs a Fedora adapter for Flysystem.
   *
   * @param \Islandora\Chullo\IFedoraApi $fedora
   *   Fedora client.
   * @param \Symfony\Component\Mime\MimeTypeGuesserInterface $mime_type_guesser
   *   Mimetype guesser.
   * @param \Drupal\Core\Logger\LoggerChannelInterface $logger
   *   The fedora adapter logger channel.
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The current request.
   * @param string $fedora_root
   *   The path to fedora's OCFL root directory.
   */
  public function __construct(
    IFedoraApi $fedora,
    MimeTypeGuesserInterface $mime_type_guesser,
    LoggerChannelInterface $logger,
    Request $request,
    string $fedora_root = '',
  ) {
    $this->fedora = $fedora;
    $this->mimeTypeGuesser = $mime_type_guesser;
    $this->logger = $logger;
    $this->request = $request;
    $this->fedoraRoot = $fedora_root;
  }

  /**
   * Check if we should read from disk.
   *
   * @return bool
   *   TRUE if fedoraRoot is configured and not empty.
   */
  protected function useDiskReading($path) {
    // Prevent directory traversal.
    if (strpos($path, '..') !== FALSE || strpos($path, './') !== FALSE) {
      return FALSE;
    }

    // If we're setting up a directory in fedora
    // do not attempt to read from disk.
    $info = pathinfo($path);
    return !empty($info['extension']) && !empty($this->fedoraRoot);
  }

  /**
   * Convert Fedora path to disk path.
   *
   * @param string $path
   *   Fedora resource path.
   *
   * @return string
   *   Full disk path to the file.
   */
  protected function getDiskPath(string $path) : string {
    // Remove leading slash if present.
    $path = ltrim($path, '/');
    $fedora_id = 'info:fedora/' . $path;
    $ocfl_dir = $this->getOcflDir($fedora_id);
    $inventory = $ocfl_dir . '/extensions/0005-mutable-head/head/inventory.json';
    if (!file_exists($inventory)) {
      $this->logger->warning('OCFL inventory not found: @path', ['@path' => $inventory]);
      return "";
    }

    $inventory_json = file_get_contents($inventory);
    if ($inventory_json === FALSE) {
      $this->logger->error('Failed to read OCFL inventory: @path', ['@path' => $inventory]);
      return "";
    }

    $inventory = json_decode($inventory_json, TRUE);
    if (json_last_error() !== JSON_ERROR_NONE) {
      $this->logger->error('Invalid JSON in OCFL inventory: @error', ['@error' => json_last_error_msg()]);
      return "";
    }

    if (!isset($inventory['head'], $inventory['versions'], $inventory['manifest'])) {
      $this->logger->error('Malformed OCFL inventory structure missing required fields: @path', ['@path' => $inventory]);
      return "";
    }

    $head = $inventory['head'];
    if (!isset($inventory['versions'][$head]['state'])) {
      $this->logger->error('Missing version state in OCFL inventory for version @version: @path', [
        '@version' => $head,
        '@path' => $inventory,
      ]);
      return "";
    }

    $state = $inventory['versions'][$head]['state'];
    $manifest = $inventory['manifest'];

    $components = explode('/', $path);
    $filename = array_pop($components);
    foreach ($state as $digest => $files) {
      if (!in_array($filename, $files)) {
        continue;
      }
      if (empty($manifest[$digest][0])) {
        continue;
      }

      return $ocfl_dir . '/' . $manifest[$digest][0];
    }

    return "";
  }

  /**
   * Get file metadata from disk with optional disk path return.
   *
   * @param string $path
   *   Fedora resource path.
   * @param bool $return_disk_path
   *   Whether to return the disk path in the metadata.
   *
   * @return array|false
   *   Metadata array with optional disk_path, or FALSE on failure.
   */
  protected function getDiskMetadata($path, $return_disk_path = FALSE) {
    $diskPath = $this->getDiskPath($path);

    if (!$diskPath || !file_exists($diskPath)) {
      return FALSE;
    }

    $stat = stat($diskPath);
    if ($stat === FALSE) {
      return FALSE;
    }

    $meta = [
      'type' => 'file',
      'path' => $path,
      'timestamp' => $stat['mtime'],
      'size' => $stat['size'],
      'mimetype' => $this->mimeTypeGuesser->guessMimeType($diskPath),
    ];

    if ($return_disk_path) {
      $meta['disk_path'] = $diskPath;
    }

    return $meta;
  }

  /**
   * Helper function to get the OCFL directory of a fcrepo object ID.
   */
  protected function getOcflDir(string $objectId) : string {
    $digest = hash('sha256', $objectId);
    $tupleSize = 3;
    $numberOfTuples = 3;
    $path = rtrim($this->fedoraRoot, '/') . '/';
    for ($i = 0; $i < $numberOfTuples * $tupleSize; $i += $tupleSize) {
      $tuple = substr($digest, $i, $tupleSize);
      $path .= $tuple . "/";
    }

    $path .= $digest;

    return $path;
  }

  /**
   * {@inheritdoc}
   */
  public function has($path) {
    if ($this->useDiskReading($path)) {
      $diskPath = $this->getDiskPath($path);

      return $diskPath != "" && file_exists($diskPath);
    }

    $response = $this->fedora->getResourceHeaders($path, ['Connection' => 'close']);
    return $response->getStatusCode() == 200;
  }

  /**
   * {@inheritdoc}
   */
  public function read($path) {
    $meta = $this->readStream($path);

    if (!$meta) {
      return FALSE;
    }

    if (!$this->useDiskReading($path) && isset($meta['stream'])) {
      $meta['contents'] = stream_get_contents($meta['stream']);
      fclose($meta['stream']);
      unset($meta['stream']);
    }

    return $meta;
  }

  /**
   * {@inheritdoc}
   */
  public function readStream($path) {
    if ($this->useDiskReading($path)) {
      $meta = $this->getDiskMetadata($path, TRUE);

      if ($meta === FALSE) {
        return FALSE;
      }

      $stream = fopen($meta['disk_path'], 'r');
      if ($stream === FALSE) {
        return FALSE;
      }

      // Remove the disk_path from metadata before returning.
      unset($meta['disk_path']);
      $meta['stream'] = $stream;

      return $meta;
    }

    $headers = ['Connection' => 'close'];

    // If the request is for a range
    // pass that header to fedora.
    if ($this->request && $this->request->headers->has('Range')) {
      $range = $this->request->headers->get('Range');
      if (str_starts_with($range, 'bytes=')) {
        // Since \Symfony\Component\HttpFoundation\BinaryFileResponse seeks
        // to the start of the range based on the request's Range header
        // we need to always set start to 0 so fedora returns
        // all the bytes between zero and the start of the range.
        [$start, $end] = explode('-', substr($range, 6), 2) + [1 => ""];
        $headers['Range'] = "bytes=0-$end";
      }
    }

    $response = $this->fedora->getResource($path, $headers);
    if (!in_array($response->getStatusCode(), [200, 206], TRUE)) {
      return FALSE;
    }

    $meta = $this->getMetadataFromHeaders($response);
    $meta['path'] = $path;
    if ($meta['type'] == 'file') {
      $meta['stream'] = StreamWrapper::getResource($response->getBody());
    }

    return $meta;
  }

  /**
   * {@inheritdoc}
   */
  public function getMetadata($path) {
    if ($this->useDiskReading($path)) {
      return $this->getDiskMetadata($path);
    }

    $response = $this->fedora->getResourceHeaders($path, ['Connection' => 'close']);

    if ($response->getStatusCode() != 200) {
      return FALSE;
    }

    $meta = $this->getMetadataFromHeaders($response);
    $meta['path'] = $path;
    return $meta;
  }

  /**
   * {@inheritdoc}
   */
  public function getSize($path) {
    return $this->getMetadata($path);
  }

  /**
   * {@inheritdoc}
   */
  public function getMimetype($path) {
    return $this->getMetadata($path);
  }

  /**
   * {@inheritdoc}
   */
  public function getTimestamp($path) {
    return $this->getMetadata($path);
  }

  /**
   * Gets metadata from response headers.
   *
   * @param \GuzzleHttp\Psr7\Response $response
   *   Response.
   */
  protected function getMetadataFromHeaders(Response $response) {
    $last_modified = \DateTime::createFromFormat(
        \DateTime::RFC1123,
        $response->getHeader('Last-Modified')[0]
    );

    // NonRDFSource's are considered files.  Everything else is a
    // directory.
    $type = 'dir';
    $links = Header::parse($response->getHeader('Link'));

    foreach ($links as $link) {
      if ($link['rel'] == 'type' && $link[0] == '<http://www.w3.org/ns/ldp#NonRDFSource>') {
        $type = 'file';
        break;
      }
    }

    $meta = [
      'type' => $type,
      'timestamp' => $last_modified->getTimestamp(),
    ];

    if ($type == 'file') {
      $meta['size'] = $response->getHeader('Content-Length')[0];
      $meta['mimetype'] = $response->getHeader('Content-Type')[0];
    }

    return $meta;
  }

  /**
   * {@inheritdoc}
   */
  public function listContents($directory = '', $recursive = FALSE) {
    // Strip leading and trailing whitespace and /'s.
    $normalized = trim($directory, ' \t\n\r\0\x0B/');

    // Exit early if it's a file.
    $meta = $this->getMetadata($normalized);
    if ($meta['type'] == 'file') {
      return [];
    }
    // Get the resource from Fedora.
    $response = $this->fedora->getResource($normalized, [
      'Accept' => 'application/ld+json',
      'Connection' => 'close',
    ]);
    $jsonld = (string) $response->getBody();
    $graph = json_decode($jsonld, TRUE);

    $uri = $this->fedora->getBaseUri() . $normalized;

    // Hack it out of the graph.
    // There may be more than one resource returned.
    $resource = [];
    foreach ($graph as $elem) {
      if (isset($elem['@id']) && $elem['@id'] == $uri) {
        $resource = $elem;
        break;
      }
    }

    // Exit early if resource doesn't contain other resources.
    if (!isset($resource['http://www.w3.org/ns/ldp#contains'])) {
      return [];
    }

    // Collapse uris to a single array.
    $contained = array_map(
        function ($elem) {
            return $elem['@id'];
        },
        $resource['http://www.w3.org/ns/ldp#contains']
    );

    // Exit early if not recursive.
    if (!$recursive) {
      // Transform results to their flysystem metadata.
      return array_map(
        [$this, 'transformToMetadata'],
        $contained
      );
    }

    // Recursively get containment for ancestors.
    $ancestors = [];

    foreach ($contained as $child_uri) {
      $child_directory = explode($this->fedora->getBaseUri(), $child_uri)[1];
      $ancestors = array_merge($this->listContents($child_directory, $recursive), $ancestors);
    }

    // // Transform results to their flysystem metadata.
    return array_map(
        [$this, 'transformToMetadata'],
        array_merge($ancestors, $contained)
    );
  }

  /**
   * Normalizes data for listContents().
   *
   * @param string $uri
   *   Uri.
   */
  protected function transformToMetadata($uri) {
    if (is_array($uri)) {
      return $uri;
    }
    $exploded = explode($this->fedora->getBaseUri(), $uri);
    return $this->getMetadata($exploded[1]);
  }

  /**
   * {@inheritdoc}
   */
  public function write($path, $contents, Config $config) {
    $headers = [
      'Content-Type' => $this->mimeTypeGuesser->guessMimeType($path),
    ];
    if ($this->has($path)) {
      $fedora_url = $path;
      $date = new \DateTime();
      $timestamp = $date->format("D, d M Y H:i:s O");
      // Create version in Fedora.
      try {
        $response = $this->fedora->createVersion(
          $fedora_url,
          $timestamp,
          NULL,
          $headers
        );
        if (isset($response) && $response->getStatusCode() == 201) {
          $this->logger->info('Created a version in Fedora for ' . $fedora_url);
        }
        else {
          $this->logger->error(
            "Client error: `Failed to create a Fedora version of $fedora_url`. Response is " . print_r($response, TRUE)
          );

        }
      }
      catch (\Exception $e) {
        $this->logger->error('Caught exception when creating version: ' . $e->getMessage() . "\n");
      }
    }

    $response = $this->fedora->saveResource(
        $path,
        $contents,
        $headers
    );

    $code = $response->getStatusCode();
    if (!in_array($code, [201, 204])) {
      return FALSE;
    }

    return $this->getMetadata($path);
  }

  /**
   * {@inheritdoc}
   */
  public function writeStream($path, $contents, Config $config) {
    return $this->write($path, $contents, $config);
  }

  /**
   * {@inheritdoc}
   */
  public function update($path, $contents, Config $config) {
    return $this->write($path, $contents, $config);
  }

  /**
   * {@inheritdoc}
   */
  public function updateStream($path, $contents, Config $config) {
    return $this->write($path, $contents, $config);
  }

  /**
   * {@inheritdoc}
   */
  public function delete($path) {
    $response = $this->fedora->deleteResource($path);
    $code = $response->getStatusCode();
    if ($code == 204) {
      // Deleted so check for a tombstone as well.
      $tomb_code = $this->deleteTombstone($path);
      if (!is_null($tomb_code)) {
        return $tomb_code;
      }
    }
    return in_array($code, [204, 404]);
  }

  /**
   * {@inheritdoc}
   */
  public function deleteDir($dirname) {
    return $this->delete($dirname);
  }

  /**
   * {@inheritdoc}
   */
  public function rename($path, $newpath) {
    if ($this->copy($path, $newpath)) {
      return $this->delete($path);
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function createDir($dirname, Config $config) {
    $response = $this->fedora->saveResource(
        $dirname
    );

    $code = $response->getStatusCode();
    if (!in_array($code, [201, 204])) {
      return FALSE;
    }

    return $this->getMetadata($dirname);
  }

  /**
   * Delete a tombstone for a path if it exists.
   *
   * @param string $path
   *   The original deleted resource path.
   *
   * @return bool|null
   *   NULL if no tombstone, TRUE if tombstone deleted, FALSE otherwise.
   */
  private function deleteTombstone($path) {
    $response = $this->fedora->getResourceHeaders($path, ['Connection' => 'close']);
    $return = NULL;
    if ($response->getStatusCode() == 410) {
      $return = FALSE;
      $link_headers = Header::parse($response->getHeader('Link'));
      if ($link_headers) {
        $tombstones = array_filter($link_headers, function ($o) {
          return (isset($o['rel']) && $o['rel'] == 'hasTombstone');
        });
        foreach ($tombstones as $tombstone) {
          // Trim <> from URL.
          $url = rtrim(ltrim($tombstone[0], '<'), '>');
          $response = $this->fedora->deleteResource($url);
          if ($response->getStatusCode() == 204) {
            $return = TRUE;
          }
        }
      }
    }
    return $return;
  }

}
