<?php

namespace Drupal\Tests\islandora\Kernel;

use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\islandora\Flysystem\Adapter\FedoraAdapter;
use GuzzleHttp\Psr7\Response;
use GuzzleHttp\Psr7\Utils;
use Islandora\Chullo\IFedoraApi;
use League\Flysystem\Config;
use PHPUnit\Framework\Attributes\CoversClass;
use PHPUnit\Framework\Attributes\Group;
use Prophecy\Argument;
use Prophecy\PhpUnit\ProphecyTrait;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\Mime\MimeTypeGuesserInterface;

/**
 * Tests the Fedora adapter for Flysystem.
 */
#[Group('islandora')]
#[CoversClass(FedoraAdapter::class)]
class FedoraAdapterTest extends IslandoraKernelTestBase {

  use ProphecyTrait;
  /**
   * A mimetype guesser prophecy.
   *
   * @var \Prophecy\Prophecy\ObjectProphecy
   */
  private $mimeGuesser;

  /**
   * A logger prophecy.
   *
   * @var \Prophecy\Prophecy\ObjectProphecy
   */
  private $logger;

  /**
   * The current request.
   *
   * @var \Symfony\Component\HttpFoundation\Request
   */
  private $request;

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();
    $this->mimeGuesser = $this->prophesize(MimeTypeGuesserInterface::class)
      ->reveal();
    $this->logger = $this->prophesize(LoggerChannelInterface::class)->reveal();

    $this->request = Request::create('/_flysystem/fedora/path/to/file.ext');
  }

  /**
   * Shared functionality for an adapter.
   */
  protected function createAdapterBase() {
    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(200);
    $prophecy->getHeader('Last-Modified')
      ->willReturn(["Wed, 25 Jul 2018 17:42:04 GMT"]);
    $prophecy->getHeader('Link')
      ->willReturn([
        '<http://www.w3.org/ns/ldp#Resource>;rel="type"',
        '<http://www.w3.org/ns/ldp#NonRDFSource>;rel="type"',
      ]);
    $prophecy->getHeader('Content-Type')->willReturn(['text/plain']);
    $prophecy->getHeader('Content-Length')->willReturn([strlen("DERP")]);
    $prophecy->getBody()->willReturn(Utils::streamFor("DERP"));

    return $prophecy;
  }

  /**
   * Mocks up an adapter for Fedora calls that return 404.
   */
  protected function createAdapterForFail() {
    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(404);
    $response = $prophecy->reveal();

    $prophecy = $this->prophesize(IFedoraApi::class);
    $prophecy->getResourceHeaders('', ['Connection' => 'close'])->willReturn($response);
    $prophecy->getResource('', ['Connection' => 'close'])->willReturn($response);
    $api = $prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Fedora LDP-NR response.
   */
  protected function createAdapterForFile() {
    $prophecy = $this->createAdapterBase();
    $response = $prophecy->reveal();

    $prophecy = $this->prophesize(IFedoraApi::class);
    $prophecy->getResourceHeaders('', ['Connection' => 'close'])->willReturn($response);
    $prophecy->getResource('', ['Connection' => 'close'])->willReturn($response);
    $api = $prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Fedora LDP-RS response.
   */
  protected function createAdapterForDirectory() {
    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(200);
    $prophecy->getHeader('Last-Modified')
      ->willReturn(["Wed, 25 Jul 2018 17:42:04 GMT"]);
    $prophecy->getHeader('Link')
      ->willReturn([
        '<http://www.w3.org/ns/ldp#Resource>;rel="type"',
        '<http://www.w3.org/ns/ldp#RDFSource>;rel="type"',
      ]);
    $response = $prophecy->reveal();

    $prophecy = $this->prophesize(IFedoraApi::class);
    $prophecy->getResourceHeaders('', ['Connection' => 'close'])->willReturn($response);
    $api = $prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Fedora write requests.
   */
  protected function createAdapterForWrite() {
    $fedora_prophecy = $this->prophesize(IFedoraApi::class);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(201);
    $fedora_prophecy->createVersion('', Argument::any(), NULL,
      Argument::any())->willReturn($prophecy->reveal());
    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(201);

    $fedora_prophecy->saveResource('', '', Argument::any())
      ->willReturn($prophecy->reveal());

    $prophecy = $this->createAdapterBase();

    $fedora_prophecy->getResourceHeaders('', ['Connection' => 'close'])->willReturn($prophecy->reveal());

    $api = $fedora_prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Fedora write requests that fail.
   */
  protected function createAdapterForWriteFail() {
    $fedora_prophecy = $this->prophesize(IFedoraApi::class);
    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(500);
    $fedora_prophecy->getResourceHeaders('', ['Connection' => 'close'])->willReturn($prophecy->reveal());

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(500);

    $fedora_prophecy->saveResource('', '', Argument::any())
      ->willReturn($prophecy->reveal());

    $api = $fedora_prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for creating directories requests.
   */
  protected function createAdapterForCreateDir() {
    $fedora_prophecy = $this->prophesize(IFedoraApi::class);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(201);

    $fedora_prophecy->saveResource('')->willReturn($prophecy->reveal());

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(200);
    $prophecy->getHeader('Last-Modified')
      ->willReturn(["Wed, 25 Jul 2018 17:42:04 GMT"]);
    $prophecy->getHeader('Link')
      ->willReturn([
        '<http://www.w3.org/ns/ldp#Resource>;rel="type"',
        '<http://www.w3.org/ns/ldp#RDFSource>;rel="type"',
      ]);

    $fedora_prophecy->getResourceHeaders('', ['Connection' => 'close'])->willReturn($prophecy->reveal());

    $api = $fedora_prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Delete requests.
   */
  protected function createAdapterForDelete() {
    $fedora_prophecy = $this->prophesize(IFedoraApi::class);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(204);

    $fedora_prophecy->deleteResource('')->willReturn($prophecy->reveal());
    $fedora_prophecy->getResourceHeaders('', ['Connection' => 'close'])->willReturn($prophecy->reveal());
    $api = $fedora_prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Delete requests that fail.
   */
  protected function createAdapterForDeleteFail() {
    $fedora_prophecy = $this->prophesize(IFedoraApi::class);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(500);

    $fedora_prophecy->deleteResource('')->willReturn($prophecy->reveal());

    $api = $fedora_prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Delete requests with a tombstone.
   */
  protected function createAdapterForDeleteWithTombstone() {
    $fedora_prophecy = $this->prophesize(IFedoraApi::class);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(204);

    $head_prophecy = $this->prophesize(Response::class);
    $head_prophecy->getStatusCode()->willReturn(410);
    $head_prophecy->getHeader('Link')
      ->willReturn(['<some-path-to-a-tombstone>; rel="hasTombstone"']);

    $tombstone_prophecy = $this->prophesize(Response::class);
    $tombstone_prophecy->getStatusCode()->willReturn(204);

    $fedora_prophecy->deleteResource('')->willReturn($prophecy->reveal());
    $fedora_prophecy->getResourceHeaders('', ['Connection' => 'close'])
      ->willReturn($head_prophecy->reveal());
    $fedora_prophecy->deleteResource('some-path-to-a-tombstone')
      ->willReturn($tombstone_prophecy->reveal());

    $api = $fedora_prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Mocks up an adapter for Delete requests with a tombstone which fail.
   */
  protected function createAdapterForDeleteWithTombstoneFail() {
    $fedora_prophecy = $this->prophesize(IFedoraApi::class);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(204);

    $head_prophecy = $this->prophesize(Response::class);
    $head_prophecy->getStatusCode()->willReturn(410);
    $head_prophecy->getHeader('Link')
      ->willReturn(['<some-path-to-a-tombstone>; rel="hasTombstone"']);

    $tombstone_prophecy = $this->prophesize(Response::class);
    $tombstone_prophecy->getStatusCode()->willReturn(500);

    $fedora_prophecy->deleteResource('')->willReturn($prophecy->reveal());
    $fedora_prophecy->getResourceHeaders('', ['Connection' => 'close'])
      ->willReturn($head_prophecy->reveal());
    $fedora_prophecy->deleteResource('some-path-to-a-tombstone')
      ->willReturn($tombstone_prophecy->reveal());

    $api = $fedora_prophecy->reveal();

    return new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);
  }

  /**
   * Asserts the stucture/contents of a metadata response for a file.
   */
  protected function assertFileMetadata(array $metadata) {
    $this->assertTrue($metadata['type'] == 'file', "Expecting 'type' of 'file', received '" . $metadata['type'] . "'");
    $this->assertTrue($metadata['timestamp'] == '1532540524', "Expecting 'timestamp' of '1532540524', received '" . $metadata['timestamp'] . "'");
    $this->assertTrue($metadata['size'] == strlen("DERP"), "Expecting 'size' of '" . strlen("DERP") . "', received '" . $metadata['size'] . "'");
    $this->assertTrue($metadata['mimetype'] == 'text/plain', "Expecting 'mimetype' of 'image/png', received '" . $metadata['mimetype'] . "'");
  }

  /**
   * Asserts the stucture/contents of a metadata response for a directory.
   */
  protected function assertDirMetadata(array $metadata) {
    $this->assertTrue($metadata['type'] == 'dir', "Expecting 'type' of 'dir', received '" . $metadata['type'] . "'");
    $this->assertTrue($metadata['timestamp'] == '1532540524', "Expecting 'timestamp' of '1532540524', received '" . $metadata['timestamp'] . "'");
  }

  /**
   * Tests getMetadata() failure.
   */
  public function testGetMetadataFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->getMetadata('') == FALSE, "getMetadata() must return FALSE on non-200");
  }

  /**
   * Tests getMetadata() for file.
   */
  public function testGetMetadataForFile() {
    $adapter = $this->createAdapterForFile();

    $metadata = $adapter->getMetadata('');
    $this->assertFileMetadata($metadata);
  }

  /**
   * Tests getMetadata() for directory.
   */
  public function testGetMetadataForDirectory() {
    $adapter = $this->createAdapterForDirectory();

    $metadata = $adapter->getMetadata('');
    $this->assertDirMetadata($metadata);
  }

  /**
   * Tests readStream().
   */
  public function testReadStream() {
    $adapter = $this->createAdapterForFile();

    $metadata = $adapter->readStream('');
    $this->assertFileMetadata($metadata);
    $this->assertTrue(is_resource($metadata['stream']), "Expecting a 'stream' that is a resource");
  }

  /**
   * Tests readStream() failure.
   */
  public function testReadStreamFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->readStream('') == FALSE, "readStream() should return FALSE on non-200 from Fedora");
  }

  /**
   * Tests read().
   */
  public function testRead() {
    $adapter = $this->createAdapterForFile();

    $metadata = $adapter->read('');
    $this->assertFileMetadata($metadata);
    $this->assertTrue($metadata['contents'] == "DERP", "Expecting 'content' of 'DERP', received '{$metadata['contents']}'");
  }

  /**
   * Tests read() failure.
   */
  public function testReadFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->read('') == FALSE, "readStream() should return FALSE on non-200 from Fedora");
  }

  /**
   * Tests has() failure.
   */
  public function testHasFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->has('') == FALSE, "has() must return FALSE on non-200");
  }

  /**
   * Tests has() success.
   */
  public function testHasSuccess() {
    $adapter = $this->createAdapterForFile();

    $this->assertTrue($adapter->has('') == TRUE, "has() must return TRUE on 200");
  }

  /**
   * Tests getSize() failure.
   */
  public function testGetSizeFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->getSize('') == FALSE, "getSize() must return FALSE on non-200");
  }

  /**
   * Tests getSize() success.
   */
  public function testGetSizeSuccess() {
    $adapter = $this->createAdapterForFile();

    $metadata = $adapter->getSize('');
    $this->assertTrue($metadata['size'] == strlen("DERP"), "Expecting 'size' of '" . strlen("DERP") . "', received '" . $metadata['size'] . "'");
  }

  /**
   * Tests getMimetype() failure.
   */
  public function testGetMimetypeFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->getMimetype('') == FALSE, "getMimetype() must return FALSE on non-200");
  }

  /**
   * Tests getMimetype() success.
   */
  public function testGetMimetypeSuccess() {
    $adapter = $this->createAdapterForFile();

    $metadata = $adapter->getMimetype('');
    $this->assertTrue($metadata['mimetype'] == 'text/plain', "Expecting 'mimetype' of 'text/plain', received '" . $metadata['mimetype'] . "'");
  }

  /**
   * Tests getTimestamp() failure.
   */
  public function testGetTimestampFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->getTimestamp('') == FALSE, "getTimestamp() must return FALSE on non-200");
  }

  /**
   * Tests getTimestamp() success.
   */
  public function testGetTimestampSuccess() {
    $adapter = $this->createAdapterForFile();

    $metadata = $adapter->getTimestamp('');
    $this->assertTrue($metadata['timestamp'] == '1532540524', "Expecting 'timestamp' of '1532540524', received '" . $metadata['timestamp'] . "'");
  }

  /**
   * Tests write() failure.
   */
  public function testWriteFail() {
    $adapter = $this->createAdapterForWriteFail();

    $this->assertTrue($adapter->write('', '', $this->prophesize(Config::class)
      ->reveal()) == FALSE, "write() must return FALSE on non-201 or non-204");
  }

  /**
   * Tests write().
   */
  public function testWrite() {
    $adapter = $this->createAdapterForWrite();

    $metadata = $adapter->write('', '', $this->prophesize(Config::class)
      ->reveal());
    $this->assertFileMetadata($metadata);
  }

  /**
   * Tests writeStream() failure.
   */
  public function testWriteStreamFail() {
    $adapter = $this->createAdapterForWriteFail();

    $this->assertTrue($adapter->writeStream('', '', $this->prophesize(Config::class)
      ->reveal()) == FALSE, "writeStream() must return FALSE on non-201 or non-204");
  }

  /**
   * Tests writeStream().
   */
  public function testWriteStream() {
    $adapter = $this->createAdapterForWrite();

    $metadata = $adapter->writeStream('', '', $this->prophesize(Config::class)
      ->reveal());
    $this->assertFileMetadata($metadata);
  }

  /**
   * Tests update() failure.
   */
  public function testUpdateFail() {
    $adapter = $this->createAdapterForWriteFail();

    $this->assertTrue($adapter->update('', '', $this->prophesize(Config::class)
      ->reveal()) == FALSE, "write() must return FALSE on non-201 or non-204");
  }

  /**
   * Tests update().
   */
  public function testUpdate() {
    $adapter = $this->createAdapterForWrite();

    $metadata = $adapter->update('', '', $this->prophesize(Config::class)
      ->reveal());
    $this->assertFileMetadata($metadata);
  }

  /**
   * Tests updateStream() failure.
   */
  public function testUpdateStreamFail() {
    $adapter = $this->createAdapterForWriteFail();

    $this->assertTrue($adapter->updateStream('', '', $this->prophesize(Config::class)
      ->reveal()) == FALSE, "writeStream() must return FALSE on non-201 or non-204");
  }

  /**
   * Tests updateStream().
   */
  public function testUpdateStream() {
    $adapter = $this->createAdapterForWrite();

    $metadata = $adapter->updateStream('', '', $this->prophesize(Config::class)
      ->reveal());
    $this->assertFileMetadata($metadata);
  }

  /**
   * Tests delete() failure.
   */
  public function testDeleteFail() {
    $adapter = $this->createAdapterForDeleteFail();

    $this->assertTrue($adapter->delete('') == FALSE, "delete() must return FALSE on non-204 or non-404");
  }

  /**
   * Tests delete().
   */
  public function testDelete() {
    $adapter = $this->createAdapterForDelete();

    $this->assertTrue($adapter->delete('') == TRUE, "delete() must return TRUE on 204 or 404");
  }

  /**
   * Tests deleteDir() failure.
   */
  public function testDeleteDirFail() {
    $adapter = $this->createAdapterForDeleteFail();

    $this->assertTrue($adapter->deleteDir('') == FALSE, "deleteDir() must return FALSE on non-204 or non-404");
  }

  /**
   * Tests deleteDir().
   */
  public function testDeleteDir() {
    $adapter = $this->createAdapterForDelete();

    $this->assertTrue($adapter->deleteDir('') == TRUE, "deleteDir() must return TRUE on 204 or 404");
  }

  /**
   * Tests delete() with tombstone.
   */
  public function testDeleteWithTombstone() {
    $adapter = $this->createAdapterForDeleteWithTombstone();

    $this->assertTrue($adapter->delete(''), 'delete() must return TRUE on 204 or 404 reponse after deleting the tombstone.');
  }

  /**
   * Tests delete() with tombstone failure.
   */
  public function testDeleteWithTombstoneFail() {
    $adapter = $this->createAdapterForDeleteWithTombstoneFail();

    $this->assertFalse($adapter->delete(''), 'delete() must return FALSE on non-(204 or 404) response after deleting the tombstone.');
  }

  /**
   * Tests rename() failure.
   */
  public function testRenameFail() {
    $adapter = $this->createAdapterForFail();

    $this->assertTrue($adapter->rename('', '') == FALSE, "rename() must return FALSE on Fedora error");
  }

  /**
   * Tests rename().
   */
  public function testRename() {
    $prophecy = $this->createAdapterBase();

    $response = $prophecy->reveal();

    $fedora_prophecy = $this->prophesize(IFedoraApi::class);
    $fedora_prophecy->getResource(Argument::any(), ['Connection' => 'close'])->willReturn($response);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(201);
    $response = $prophecy->reveal();

    $fedora_prophecy->createVersion('',
      Argument::any(), NULL,
      Argument::any())->willReturn($prophecy->reveal());
    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(201);

    $fedora_prophecy->saveResource(Argument::any(), Argument::any(), Argument::any())
      ->willReturn($response);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(200);
    $prophecy->getHeader('Last-Modified')
      ->willReturn(["Wed, 25 Jul 2018 17:42:04 GMT"]);
    $prophecy->getHeader('Link')->willReturn([
      '<http://www.w3.org/ns/ldp#Resource>;rel="type"',
      '<http://www.w3.org/ns/ldp#NonRDFSource>;rel="type"',
    ]);
    $prophecy->getHeader('Content-Type')->willReturn(['text/plain']);
    $prophecy->getHeader('Content-Length')->willReturn([strlen("DERP")]);
    $response = $prophecy->reveal();

    $fedora_prophecy->getResourceHeaders(Argument::any(), ['Connection' => 'close'])
      ->willReturn($response);

    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(204);
    $response = $prophecy->reveal();

    $fedora_prophecy->deleteResource(Argument::any())->willReturn($response);

    $api = $fedora_prophecy->reveal();

    $adapter = new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);

    $this->assertTrue($adapter->rename('', '') == TRUE, "rename() must return TRUE on success");
  }

  /**
   * Tests createDir() failure.
   */
  public function testCreateDirFail() {
    $prophecy = $this->prophesize(Response::class);
    $prophecy->getStatusCode()->willReturn(500);

    $fedora_prophecy = $this->prophesize(IFedoraApi::class);
    $fedora_prophecy->saveResource('')->willReturn($prophecy->reveal());

    $api = $fedora_prophecy->reveal();

    $adapter = new FedoraAdapter($api, $this->mimeGuesser, $this->logger, $this->request);

    $this->assertTrue($adapter->createDir('', $this->prophesize(Config::class)
      ->reveal()) == FALSE, "createDir() must return FALSE on fail");
  }

  /**
   * Tests createDir().
   */
  public function testCreateDir() {
    $adapter = $this->createAdapterForCreateDir();

    $metadata = $adapter->createDir('', $this->prophesize(Config::class)
      ->reveal());
    $this->assertDirMetadata($metadata);
  }

}
