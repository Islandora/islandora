<?php

namespace Drupal\Tests\islandora_fcrepo\Unit;

use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\Core\StreamWrapper\StreamWrapperManagerInterface;
use Drupal\flysystem\Adapter\FilesystemFactoryInterface;
use Drupal\flysystem\StreamWrapper\FlysystemStreamWrapper;
use Drupal\islandora_fcrepo\Flysystem\Adapter\FedoraAdapter;
use Drupal\system\FileDownloadController;
use Drupal\Tests\UnitTestCase;
use GuzzleHttp\Psr7\Response;
use Islandora\Chullo\IFedoraApi;
use League\Flysystem\Filesystem;
use PHPUnit\Framework\Attributes\DataProvider;
use PHPUnit\Framework\Attributes\Group;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException;
use Symfony\Component\Mime\MimeTypeGuesserInterface;

/**
 * Checks Drupal download authorization and byte ranges over the Fedora adapter.
 */
#[Group('islandora_fcrepo')]
class FedoraDownloadTest extends UnitTestCase {

  /**
   * {@inheritdoc}
   */
  protected function setUp(): void {
    parent::setUp();
    if (!interface_exists(IFedoraApi::class) || !class_exists(FlysystemStreamWrapper::class)) {
      $this->markTestSkipped('Install the optional islandora_fcrepo Composer dependencies.');
    }
    $api = $this->createMock(IFedoraApi::class);
    $response = static function () {
      // A socket models the non-seekable body returned by streaming HTTP GETs.
      $sockets = stream_socket_pair(STREAM_PF_UNIX, STREAM_SOCK_STREAM, STREAM_IPPROTO_IP);
      fwrite($sockets[1], '0123456789');
      fclose($sockets[1]);
      return new Response(200, [
        'Last-Modified' => 'Wed, 25 Jul 2018 17:42:04 GMT',
        'Link' => '<http://www.w3.org/ns/ldp#NonRDFSource>;rel="type"',
        'Content-Type' => 'text/plain',
        'Content-Length' => 10,
      ], $sockets[0]);
    };
    $api->method('getResourceHeaders')->willReturnCallback($response);
    $api->method('getResource')->willReturnCallback($response);
    $adapter = new FedoraAdapter($api, $this->createMock(MimeTypeGuesserInterface::class), $this->createMock(LoggerChannelInterface::class));
    $factory = $this->createMock(FilesystemFactoryInterface::class);
    $factory->method('getFilesystem')->willReturn(new Filesystem($adapter));
    FlysystemStreamWrapper::primeStaticFactory($factory);
    stream_wrapper_register('fcrepotest', FlysystemStreamWrapper::class);
  }

  /**
   * {@inheritdoc}
   */
  protected function tearDown(): void {
    if (in_array('fcrepotest', stream_get_wrappers(), TRUE)) {
      stream_wrapper_unregister('fcrepotest');
    }
    parent::tearDown();
  }

  /**
   * Creates the real Drupal controller with controlled hook results.
   */
  private function controller(array $headers): FileDownloadController {
    $manager = $this->createMock(StreamWrapperManagerInterface::class);
    $manager->method('normalizeUri')->willReturnArgument(0);
    $manager->method('isValidScheme')->willReturn(TRUE);
    $handler = $this->createMock(ModuleHandlerInterface::class);
    $handler->method('invokeAll')->with('file_download', ['fcrepotest://file.txt'])->willReturn($headers);
    $controller = $this->getMockBuilder(FileDownloadController::class)
      ->setConstructorArgs([$manager])->onlyMethods(['moduleHandler'])->getMock();
    $controller->method('moduleHandler')->willReturn($handler);
    return $controller;
  }

  /**
   * A denial from another module takes precedence over response headers.
   */
  public function testAccessDenied(): void {
    $request = Request::create('/_flysystem/fcrepotest', 'GET', ['file' => 'file.txt']);
    $this->expectException(AccessDeniedHttpException::class);
    $this->controller(['Content-Type' => 'text/plain', -1])->download($request, 'fcrepotest');
  }

  /**
   * Byte ranges are interpreted by Drupal, not truncated by the adapter.
   */
  #[DataProvider('ranges')]
  public function testRange(string $range, int $status, string $body): void {
    $request = Request::create('/_flysystem/fcrepotest', 'GET', ['file' => 'file.txt']);
    $request->headers->set('Range', $range);
    $response = $this->controller(['Content-Type' => 'text/plain'])->download($request, 'fcrepotest');
    $response->prepare($request);
    $this->assertSame($status, $response->getStatusCode());
    ob_start();
    $response->sendContent();
    $this->assertSame($body, ob_get_clean());
  }

  /**
   * Bounded, suffix, open-ended, and unsatisfiable ranges.
   */
  public static function ranges(): array {
    return [
      ['bytes=2-4', 206, '234'],
      ['bytes=-3', 206, '789'],
      ['bytes=7-', 206, '789'],
      ['bytes=20-30', 416, ''],
    ];
  }

}
