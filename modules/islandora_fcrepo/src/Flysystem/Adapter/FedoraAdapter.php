<?php

namespace Drupal\islandora_fcrepo\Flysystem\Adapter;

use GuzzleHttp\Psr7\Header;
use Drupal\Core\Logger\LoggerChannelInterface;
use Islandora\Chullo\IFedoraApi;
use League\Flysystem\FilesystemAdapter;
use League\Flysystem\FileAttributes;
use League\Flysystem\DirectoryAttributes;
use League\Flysystem\UnableToReadFile;
use League\Flysystem\UnableToWriteFile;
use League\Flysystem\UnableToDeleteFile;
use League\Flysystem\UnableToDeleteDirectory;
use League\Flysystem\UnableToCreateDirectory;
use League\Flysystem\UnableToMoveFile;
use League\Flysystem\UnableToCopyFile;
use League\Flysystem\UnableToRetrieveMetadata;
use League\Flysystem\UnableToCheckExistence;
use League\Flysystem\UnableToSetVisibility;
use League\Flysystem\Visibility;
use League\Flysystem\UnableToListContents;
use League\Flysystem\Config;
use GuzzleHttp\Psr7\Response;
use Symfony\Component\Mime\MimeTypeGuesserInterface;

/**
 * Fedora adapter for Flysystem.
 */
class FedoraAdapter implements FilesystemAdapter {

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
   * Constructs a Fedora adapter for Flysystem.
   *
   * @param \Islandora\Chullo\IFedoraApi $fedora
   *   Fedora client.
   * @param \Symfony\Component\Mime\MimeTypeGuesserInterface $mime_type_guesser
   *   Mimetype guesser.
   * @param \Drupal\Core\Logger\LoggerChannelInterface $logger
   *   The fedora adapter logger channel.
   */
  public function __construct(
    IFedoraApi $fedora,
    MimeTypeGuesserInterface $mime_type_guesser,
    LoggerChannelInterface $logger,
  ) {
    $this->fedora = $fedora;
    $this->mimeTypeGuesser = $mime_type_guesser;
    $this->logger = $logger;
  }

  /**
   * {@inheritdoc}
   */
  public function fileExists(string $path): bool {
    return $this->exists($path, 'file');
  }

  /**
   * {@inheritdoc}
   */
  public function directoryExists(string $path): bool {
    return $this->exists($path, 'dir');
  }

  /**
   * Checks existence without treating a Fedora failure as a missing resource.
   */
  private function exists(string $path, string $type): bool {
    try {
      $response = $this->fedora->getResourceHeaders($path, ['Connection' => 'close']);
      if (in_array($response->getStatusCode(), [404, 410], TRUE)) {
        return FALSE;
      }
      if ($response->getStatusCode() !== 200) {
        throw new \RuntimeException('Fedora returned ' . $response->getStatusCode());
      }
      return $this->getMetadataFromHeaders($response)['type'] === $type;
    }
    catch (\Throwable $e) {
      throw UnableToCheckExistence::forLocation($path, $e);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function read(string $path): string {
    $stream = $this->readStream($path);
    try {
      $contents = stream_get_contents($stream);
      if ($contents === FALSE) {
        throw UnableToReadFile::fromLocation($path);
      }
      return $contents;
    }
    finally {
      fclose($stream);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function readStream(string $path) {
    // Always return the complete object. Drupal's BinaryFileResponse handles
    // Range requests by seeking through the Flysystem stream wrapper.
    try {
      $response = $this->fedora->getResource($path, ['Connection' => 'close']);
      if ($response->getStatusCode() !== 200 || $this->getMetadataFromHeaders($response)['type'] !== 'file') {
        throw new \RuntimeException('Fedora did not return a complete file.');
      }
      // Preserve the native stream's seekability. A Guzzle stream wrapper
      // advertises seek support even when the underlying HTTP body cannot seek.
      $stream = $response->getBody()->detach();
      if (!is_resource($stream)) {
        throw new \RuntimeException('Fedora did not return a stream resource.');
      }
      return $stream;
    }
    catch (\Throwable $e) {
      throw UnableToReadFile::fromLocation($path, $e->getMessage(), $e);
    }
  }

  /**
   * {@inheritdoc}
   */
  protected function getMetadata($path) {
    $response = $this->fedora->getResourceHeaders($path, ['Connection' => 'close']);

    if ($response->getStatusCode() != 200) {
      throw UnableToRetrieveMetadata::create($path, 'metadata', 'Fedora returned ' . $response->getStatusCode());
    }

    $meta = $this->getMetadataFromHeaders($response);
    $meta['path'] = $path;
    return $meta;
  }

  /**
   * {@inheritdoc}
   */
  public function fileSize(string $path): FileAttributes {
    return $this->fileAttributes($path);
  }

  /**
   * {@inheritdoc}
   */
  public function mimeType(string $path): FileAttributes {
    return $this->fileAttributes($path);
  }

  /**
   * {@inheritdoc}
   */
  public function lastModified(string $path): FileAttributes {
    return $this->fileAttributes($path);
  }

  /**
   * {@inheritdoc}
   */
  public function visibility(string $path): FileAttributes {
    return new FileAttributes($path, visibility: Visibility::PRIVATE);
  }

  /**
   * {@inheritdoc}
   */
  public function setVisibility(string $path, string $visibility): void {
    // Fedora authorization is managed by Drupal/JWT, not filesystem modes.
    if ($visibility !== Visibility::PRIVATE) {
      throw UnableToSetVisibility::atLocation($path, 'Fedora access is managed by Drupal.');
    }
  }

  /**
   * Returns Flysystem file metadata from Fedora headers.
   */
  private function fileAttributes(string $path): FileAttributes {
    try {
      $meta = $this->getMetadata($path);
      if ($meta['type'] !== 'file') {
        throw new \RuntimeException('The resource is a directory.');
      }
      return new FileAttributes($path, (int) $meta['size'], Visibility::PRIVATE, $meta['timestamp'], $meta['mimetype']);
    }
    catch (\Throwable $e) {
      throw UnableToRetrieveMetadata::create($path, 'metadata', $e->getMessage(), $e);
    }
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
      if (isset($link['rel']) && $link['rel'] == 'type' && $link[0] == '<http://www.w3.org/ns/ldp#NonRDFSource>') {
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
  private function listMetadata($directory = '', $recursive = FALSE) {
    // Strip leading and trailing whitespace and /'s.
    $normalized = trim($directory, '/');

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
    if ($response->getStatusCode() !== 200) {
      throw new \RuntimeException('Fedora returned ' . $response->getStatusCode());
    }
    $jsonld = (string) $response->getBody();
    $graph = json_decode($jsonld, TRUE, 512, JSON_THROW_ON_ERROR);

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
      $ancestors = array_merge($this->listMetadata($child_directory, $recursive), $ancestors);
    }

    // // Transform results to their flysystem metadata.
    return array_map(
        [$this, 'transformToMetadata'],
        array_merge($ancestors, $contained)
    );
  }

  /**
   * {@inheritdoc}
   */
  public function listContents(string $path, bool $deep): iterable {
    try {
      foreach ($this->listMetadata($path, $deep) as $meta) {
        yield $meta['type'] === 'file'
          ? new FileAttributes($meta['path'], (int) $meta['size'], Visibility::PRIVATE, $meta['timestamp'], $meta['mimetype'])
          : new DirectoryAttributes($meta['path'], Visibility::PRIVATE, $meta['timestamp']);
      }
    }
    catch (\Throwable $e) {
      throw UnableToListContents::atLocation($path, $deep, $e);
    }
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
  private function save(string $path, $contents, Config $config): void {
    $headers = [
      'Content-Type' => $this->mimeTypeGuesser->guessMimeType($path),
    ];
    if ($this->fileExists($path)) {
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
      throw UnableToWriteFile::atLocation($path, 'Fedora returned ' . $code);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function writeStream(string $path, $contents, Config $config): void {
    try {
      $this->save($path, $contents, $config);
    }
    catch (\Throwable $e) {
      throw UnableToWriteFile::atLocation($path, $e->getMessage(), $e);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function write(string $path, string $contents, Config $config): void {
    $this->writeStream($path, $contents, $config);
  }

  /**
   * {@inheritdoc}
   */
  public function delete(string $path): void {
    try {
      $code = $this->fedora->deleteResource($path)->getStatusCode();
      if (!in_array($code, [204, 404, 410], TRUE)) {
        throw new \RuntimeException('Fedora returned ' . $code);
      }
      if ($code !== 404 && $this->deleteTombstone($path) === FALSE) {
        throw new \RuntimeException('Could not delete the Fedora tombstone.');
      }
    }
    catch (\Throwable $e) {
      throw UnableToDeleteFile::atLocation($path, $e->getMessage(), $e);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function deleteDirectory(string $path): void {
    try {
      $this->delete($path);
    }
    catch (\Throwable $e) {
      throw UnableToDeleteDirectory::atLocation($path, $e->getMessage(), $e);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function copy(string $source, string $destination, Config $config): void {
    try {
      $stream = $this->readStream($source);
      try {
        $this->writeStream($destination, $stream, $config);
      }
      finally {
        fclose($stream);
      }
    }
    catch (\Throwable $e) {
      throw UnableToCopyFile::fromLocationTo($source, $destination, $e);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function move(string $source, string $destination, Config $config): void {
    try {
      $this->copy($source, $destination, $config);
      $this->delete($source);
    }
    catch (\Throwable $e) {
      throw UnableToMoveFile::fromLocationTo($source, $destination, $e);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function createDirectory(string $path, Config $config): void {
    try {
      $code = $this->fedora->saveResource($path)->getStatusCode();
      if (!in_array($code, [201, 204], TRUE)) {
        throw new \RuntimeException('Fedora returned ' . $code);
      }
    }
    catch (\Throwable $e) {
      throw UnableToCreateDirectory::atLocation($path, $e->getMessage(), $e);
    }
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
