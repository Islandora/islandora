<?php

namespace Drupal\Tests\islandora_fcrepo\Unit;

use Drupal\flysystem\Adapter\AdapterDriverPluginBase;
use Drupal\islandora_fcrepo\Plugin\Flysystem\Adapter\Fedora;
use Drupal\jwt\Authentication\Provider\JwtAuth;
use Drupal\Tests\UnitTestCase;
use GuzzleHttp\Promise\FulfilledPromise;
use GuzzleHttp\Psr7\Request;
use GuzzleHttp\Psr7\Response;
use PHPUnit\Framework\Attributes\Group;

/**
 * Tests authenticated, streaming requests to Fedora.
 */
#[Group('islandora_fcrepo')]
class FedoraMiddlewareTest extends UnitTestCase {

  /**
   * Tokens are regenerated per request and GET bodies remain streamed.
   */
  public function testAuthenticatedStreamingRequests(): void {
    if (!class_exists(AdapterDriverPluginBase::class)) {
      $this->markTestSkipped('Install the optional islandora_fcrepo Composer dependencies.');
    }
    $jwt = $this->createMock(JwtAuth::class);
    $jwt->expects($this->exactly(2))->method('generateToken')->willReturnOnConsecutiveCalls('first', 'second');
    $requests = [];
    $handler = Fedora::addJwt($jwt)(static function ($request, $options) use (&$requests) {
      $requests[] = [$request->getHeaderLine('Authorization'), $options];
      return new FulfilledPromise(new Response(200));
    });
    $handler(new Request('GET', 'http://fedora/file'), [])->wait();
    $handler(new Request('PUT', 'http://fedora/file'), [])->wait();
    $this->assertSame([['Bearer first', ['stream' => TRUE]], ['Bearer second', []]], $requests);
  }

}
