<?php

namespace Drupal\Tests\islandora\Kernel;

use Drupal\Core\Logger\LoggerChannelInterface;
use Drupal\islandora\Flysystem\Fedora;
use Islandora\Chullo\IFedoraApi;
use League\Flysystem\AdapterInterface;
use Prophecy\PhpUnit\ProphecyTrait;
use Psr\Http\Message\ResponseInterface;
use Symfony\Component\Mime\MimeTypeGuesserInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Session\Session;
use Symfony\Component\HttpFoundation\Session\Storage\MockArraySessionStorage;

/**
 * Tests the Fedora plugin for Flysystem.
 *
 * @group islandora
 * @coversDefaultClass \Drupal\islandora\Flysystem\Fedora
 */
class FedoraPluginTest extends IslandoraKernelTestBase {

  use ProphecyTrait;

  /**
   * Mocks up a plugin.
   */
  protected function createPlugin($return_code) {
    $prophecy = $this->prophesize(ResponseInterface::class);
    $prophecy->getStatusCode()->willReturn($return_code);
    $response = $prophecy->reveal();

    $prophecy = $this->prophesize(IFedoraApi::class);
    $prophecy->getResourceHeaders('')->willReturn($response);
    $prophecy->getBaseUri()->willReturn("");
    $api = $prophecy->reveal();

    $mime_guesser = $this->prophesize(MimeTypeGuesserInterface::class)->reveal();

    $language_manager = $this->container->get('language_manager');
    $logger = $this->prophesize(LoggerChannelInterface::class)->reveal();

    $request = Request::create('/_flysystem/fedora/path/to/file.ext');
    $session = new Session(new MockArraySessionStorage());
    $session->start();
    $request->setSession($session);

    /** @var \Symfony\Component\HttpFoundation\RequestStack $request_stack */
    $request_stack = $this->container->get('request_stack');
    $request_stack->push($request);

    return new Fedora($api, $mime_guesser, $language_manager, $logger, $request_stack);
  }

  /**
   * Tests the getAdapter() method.
   *
   * @covers \Drupal\islandora\Flysystem\Fedora::getAdapter
   */
  public function testGetAdapter() {
    $plugin = $this->createPlugin(200);
    $adapter = $plugin->getAdapter();

    $this->assertTrue($adapter instanceof AdapterInterface, "getAdapter() must return an AdapterInterface");
  }

  /**
   * Tests the ensure() method.
   *
   * @covers \Drupal\islandora\Flysystem\Fedora::ensure
   */
  public function testEnsure() {
    $plugin = $this->createPlugin(200);
    $this->assertTrue(empty($plugin->ensure()), "ensure() must return an empty array on success");

    $plugin = $this->createPlugin(404);
    $this->assertTrue(!empty($plugin->ensure()), "ensure() must return a non-empty array on fail");
  }

}
