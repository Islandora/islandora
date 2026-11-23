<?php

namespace Drupal\Tests\islandora_fcrepo\Kernel;

use Drupal\flysystem\Exception\AdapterConfigurationException;
use Drupal\islandora_fcrepo\Flysystem\Adapter\FedoraAdapter;
use Drupal\Tests\islandora\Kernel\IslandoraKernelTestBase;
use Islandora\Chullo\IFedoraApi;
use PHPUnit\Framework\Attributes\Group;
use Drupal\Core\Site\Settings;
use Drupal\islandora_fcrepo\PathProcessor\FedoraPathProcessor;
use Symfony\Component\HttpFoundation\Request;

/**
 * Verifies discovery and construction through Flysystem 3's plugin manager.
 */
#[Group('islandora_fcrepo')]
class FedoraPluginTest extends IslandoraKernelTestBase {

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    if (!interface_exists(IFedoraApi::class)) {
      $this->markTestSkipped('Install the optional islandora_fcrepo Composer dependencies.');
    }
    parent::setUp();
    $this->enableModules(['flysystem', 'islandora_fcrepo']);
  }

  /**
   * Builds a Fedora adapter without making a network request.
   */
  public function testPluginDiscovery(): void {
    $driver = $this->container->get('plugin.manager.flysystem.adapter_driver')->createInstance('fedora');
    $this->assertInstanceOf(FedoraAdapter::class, $driver->buildAdapter(['root' => 'http://fedora:8080/fcrepo/rest/']));
  }

  /**
   * An invalid root produces a configuration error.
   */
  public function testInvalidRoot(): void {
    $driver = $this->container->get('plugin.manager.flysystem.adapter_driver')->createInstance('fedora');
    $this->expectException(AdapterConfigurationException::class);
    $driver->buildAdapter([]);
  }

  /**
   * Installation preserves legacy display choices and customized actions.
   */
  public function testConfigurationMigration(): void {
    $storage = $this->container->get('config.storage');
    $storage->write('islandora.settings', ['gemini_pseudo_bundles' => ['object:node']]);
    $storage->write('system.action.index_node_in_fedora', [
      'id' => 'index_node_in_fedora',
      'langcode' => 'en',
      'status' => TRUE,
      'label' => 'Custom Fedora indexing',
      'type' => 'node',
      'plugin' => 'emit_node_event',
      'dependencies' => ['enforced' => ['module' => ['islandora_core_feature']]],
      'configuration' => ['queue' => 'custom-fedora-queue', 'event' => 'Update'],
    ]);
    $this->container->get('config.factory')->reset();
    require_once $this->container->get('extension.list.module')->getPath('islandora_fcrepo') . '/islandora_fcrepo.install';
    islandora_fcrepo_install();
    $this->assertSame(['object:node'], $storage->read('islandora_fcrepo.settings')['gemini_pseudo_bundles']);
    $this->assertArrayNotHasKey('gemini_pseudo_bundles', $storage->read('islandora.settings'));
    $action = $storage->read('system.action.index_node_in_fedora');
    $this->assertSame('custom-fedora-queue', $action['configuration']['queue']);
    $this->assertSame(['islandora_fcrepo'], $action['dependencies']['enforced']['module']);
  }

  /**
   * Fedora URLs retain their path form and route through Drupal downloads.
   */
  public function testLegacyDownloadUrls(): void {
    new Settings(['flysystem' => ['fcrepo' => ['driver' => 'fedora', 'config' => ['root' => 'http://fedora/rest/']]]]);
    $this->container->get('router.builder')->rebuild();
    $uri = 'fcrepo://nested/file.txt';
    islandora_fcrepo_file_url_alter($uri);
    $this->assertStringEndsWith('/_flysystem/fcrepo/nested/file.txt', $uri);
    $request = Request::create($uri);
    $processor = new FedoraPathProcessor();
    $this->assertSame('/_flysystem/fcrepo', $processor->processInbound('/_flysystem/fcrepo/nested/file.txt', $request));
    $this->assertSame('nested/file.txt', $request->query->get('file'));
    $this->assertSame('/_flysystem/unrelated/file.txt', $processor->processInbound('/_flysystem/unrelated/file.txt', $request));
  }

}
