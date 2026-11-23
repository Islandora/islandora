<?php

namespace Drupal\Tests\islandora_fcrepo\Unit;

use League\Flysystem\FilesystemAdapter;
use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\islandora_fcrepo\Flysystem\Adapter\FedoraAdapter;
use Drupal\Tests\UnitTestCase;
use GuzzleHttp\Psr7\Response;
use Islandora\Chullo\IFedoraApi;
use League\Flysystem\Config;
use League\Flysystem\Filesystem;
use League\Flysystem\CorruptedPathDetected;
use League\Flysystem\UnableToReadFile;
use League\Flysystem\UnableToWriteFile;
use League\Flysystem\UnableToDeleteFile;
use League\Flysystem\UnableToCheckExistence;
use League\Flysystem\Visibility;
use League\Flysystem\UnableToCreateDirectory;
use League\Flysystem\UnableToDeleteDirectory;
use League\Flysystem\UnableToRetrieveMetadata;
use League\Flysystem\UnableToSetVisibility;
use League\Flysystem\UnableToMoveFile;
use PHPUnit\Framework\Attributes\DataProvider;
use PHPUnit\Framework\Attributes\Group;
use Symfony\Component\Mime\MimeTypeGuesserInterface;

/**
 * Exercises the Fedora adapter through Flysystem 3's public contract.
 */
#[Group('islandora_fcrepo')]
class FedoraAdapterTest extends UnitTestCase {

  /**
   * {@inheritdoc}
   */
  protected function setUp(): void {
    parent::setUp();
    if (!interface_exists(IFedoraApi::class) || !interface_exists(FilesystemAdapter::class)) {
      $this->markTestSkipped('Install the optional islandora_fcrepo Composer dependencies.');
    }
  }

  /**
   * Builds a Fedora binary response.
   */
  private function binary(string $body = 'content'): Response {
    return new Response(200, [
      'Last-Modified' => 'Wed, 25 Jul 2018 17:42:04 GMT',
      'Link' => '<http://www.w3.org/ns/ldp#NonRDFSource>;rel="type"',
      'Content-Type' => 'text/plain',
      'Content-Length' => strlen($body),
    ], $body);
  }

  /**
   * Creates an adapter with a mocked Fedora client.
   */
  private function adapter(IFedoraApi $api): FedoraAdapter {
    $mime = $this->createMock(MimeTypeGuesserInterface::class);
    $mime->method('guessMimeType')->willReturn('text/plain');
    return new FedoraAdapter($api, $mime, $this->createMock(LoggerChannelInterface::class));
  }

  /**
   * Reads and metadata follow the Flysystem 3 contract.
   */
  public function testReadAndMetadata(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResourceHeaders')->willReturn($this->binary());
    $api->method('getResource')->willReturnCallback(fn() => $this->binary());
    $fs = new Filesystem($this->adapter($api));
    $this->assertTrue($fs->fileExists('file.txt'));
    $this->assertFalse($fs->directoryExists('file.txt'));
    $this->assertSame('content', $fs->read('file.txt'));
    $stream = $fs->readStream('file.txt');
    $this->assertSame('content', stream_get_contents($stream));
    fclose($stream);
    $this->assertSame(7, $fs->fileSize('file.txt'));
    $this->assertSame('text/plain', $fs->mimeType('file.txt'));
    $this->assertSame(1532540524, $fs->lastModified('file.txt'));
    $this->assertSame(Visibility::PRIVATE, $fs->visibility('file.txt'));
  }

  /**
   * Missing files and failed existence checks have distinct results.
   */
  public function testExistenceFailure(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResourceHeaders')->willReturnOnConsecutiveCalls(new Response(404), new Response(503));
    $adapter = $this->adapter($api);
    $this->assertFalse($adapter->fileExists('missing'));
    $this->expectException(UnableToCheckExistence::class);
    $adapter->fileExists('unavailable');
  }

  /**
   * Failed reads throw the Flysystem exception contract.
   */
  public function testReadFailure(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResource')->willReturn(new Response(404));
    $this->expectException(UnableToReadFile::class);
    $this->adapter($api)->read('missing');
  }

  /**
   * Overwrites create a version and send the requested content.
   */
  public function testWriteCreatesVersion(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResourceHeaders')->willReturn($this->binary());
    $api->expects($this->once())->method('createVersion')->willReturn(new Response(201));
    $api->expects($this->once())->method('saveResource')
      ->with('file.txt', 'replacement', ['Content-Type' => 'text/plain'])->willReturn(new Response(204));
    $this->adapter($api)->write('file.txt', 'replacement', new Config());
  }

  /**
   * Failed writes throw instead of returning false.
   */
  public function testWriteFailure(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResourceHeaders')->willReturn(new Response(404));
    $api->method('saveResource')->willReturn(new Response(500));
    $this->expectException(UnableToWriteFile::class);
    $this->adapter($api)->write('file.txt', 'content', new Config());
  }

  /**
   * Moving copies the complete stream before deleting the source.
   */
  public function testMove(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResource')->willReturn($this->binary());
    $api->method('getResourceHeaders')->willReturn(new Response(404));
    $api->expects($this->once())->method('saveResource')->with('new.txt', $this->callback(function ($stream) {
      return is_resource($stream) && stream_get_contents($stream) === 'content';
    }), ['Content-Type' => 'text/plain'])->willReturn(new Response(201));
    $api->expects($this->once())->method('deleteResource')->with('old.txt')->willReturn(new Response(204));
    $this->adapter($api)->move('old.txt', 'new.txt', new Config());
  }

  /**
   * Deletion removes a Fedora tombstone too.
   */
  public function testDeleteTombstone(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResourceHeaders')->willReturn(new Response(410, ['Link' => '<tombstone>; rel="hasTombstone"']));
    $api->expects($this->exactly(2))->method('deleteResource')->willReturnCallback(function ($path) {
      $this->assertContains($path, ['file.txt', 'tombstone']);
      return new Response(204);
    });
    $this->adapter($api)->delete('file.txt');
  }

  /**
   * Failed tombstone deletion propagates as a delete failure.
   */
  public function testDeleteTombstoneFailure(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResourceHeaders')->willReturn(new Response(410, ['Link' => '<tombstone>; rel="hasTombstone"']));
    $api->method('deleteResource')->willReturnOnConsecutiveCalls(new Response(204), new Response(500));
    $this->expectException(UnableToDeleteFile::class);
    $this->adapter($api)->delete('file.txt');
  }

  /**
   * The patched upstream normalizer rejects malformed UTF-8 before I/O.
   */
  public function testMalformedUtf8IsRejected(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->expects($this->never())->method('getResource');
    $this->expectException(CorruptedPathDetected::class);
    (new Filesystem($this->adapter($api)))->read("bad\xFF/path");
  }

  /**
   * Directory writes and deletes use Flysystem's void-returning contract.
   */
  public function testDirectories(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->expects($this->once())->method('saveResource')->with('directory')->willReturn(new Response(201));
    $api->expects($this->once())->method('deleteResource')->with('directory')->willReturn(new Response(204));
    $api->method('getResourceHeaders')->willReturn(new Response(404));
    $adapter = $this->adapter($api);
    $adapter->createDirectory('directory', new Config());
    $adapter->deleteDirectory('directory');
  }

  /**
   * Files and directories are returned as Flysystem storage attributes.
   */
  public function testRecursiveListing(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getBaseUri')->willReturn('http://fedora/rest/');
    $api->method('getResourceHeaders')->willReturnCallback(function ($path) {
      return str_ends_with($path, '.txt') ? $this->binary() : new Response(200, [
        'Last-Modified' => 'Wed, 25 Jul 2018 17:42:04 GMT',
        'Link' => '<http://www.w3.org/ns/ldp#RDFSource>;rel="type"',
      ]);
    });
    $api->method('getResource')->willReturnCallback(static function ($path) {
      $children = $path === 'root' ? ['root/file.txt', 'root/nested'] : ['root/nested/child.txt'];
      return new Response(200, [], json_encode([[
        '@id' => 'http://fedora/rest/' . $path,
        'http://www.w3.org/ns/ldp#contains' => array_map(static fn($child) => ['@id' => 'http://fedora/rest/' . $child], $children),
      ],
      ]));
    });
    $fs = new Filesystem($this->adapter($api));
    $shallow = $fs->listContents('root')->toArray();
    $this->assertCount(2, $shallow);
    $this->assertTrue($shallow[0]->isFile());
    $this->assertTrue($shallow[1]->isDir());
    $paths = array_map(static fn($entry) => $entry->path(), $fs->listContents('root', TRUE)->toArray());
    $this->assertEqualsCanonicalizing(['root/file.txt', 'root/nested', 'root/nested/child.txt'], $paths);
  }

  /**
   * Failed operations expose specific Flysystem exceptions to callers.
   */
  #[DataProvider('failures')]
  public function testOperationFailures(string $operation, string $exception): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResourceHeaders')->willReturn(new Response(500));
    $api->method('deleteResource')->willReturn(new Response(500));
    $api->method('saveResource')->willReturn(new Response(500));
    $fs = new Filesystem($this->adapter($api));
    $this->expectException($exception);
    $fs->$operation('path');
  }

  /**
   * Operation-specific exceptions for directory and metadata failures.
   */
  public static function failures(): array {
    return [
      ['createDirectory', UnableToCreateDirectory::class],
      ['deleteDirectory', UnableToDeleteDirectory::class],
      ['fileSize', UnableToRetrieveMetadata::class],
      ['mimeType', UnableToRetrieveMetadata::class],
      ['lastModified', UnableToRetrieveMetadata::class],
    ];
  }

  /**
   * Filesystem permissions cannot bypass Fedora authorization.
   */
  public function testPublicVisibilityIsUnsupported(): void {
    $this->expectException(UnableToSetVisibility::class);
    $this->adapter($this->createMock(IFedoraApi::class))->setVisibility('file.txt', Visibility::PUBLIC);
  }

  /**
   * A failed move must never delete the original file.
   */
  public function testFailedMoveKeepsSource(): void {
    $api = $this->createMock(IFedoraApi::class);
    $api->method('getResource')->willReturn($this->binary());
    $api->method('getResourceHeaders')->willReturn(new Response(404));
    $api->method('saveResource')->willReturn(new Response(500));
    $api->expects($this->never())->method('deleteResource');
    $this->expectException(UnableToMoveFile::class);
    $this->adapter($api)->move('old.txt', 'new.txt', new Config());
  }

}
