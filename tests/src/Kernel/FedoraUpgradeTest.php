<?php

namespace Drupal\Tests\islandora\Kernel;

use Drupal\flysystem\Adapter\AdapterDriverInterface;
use Drupal\Core\Site\Settings;
use Drupal\Core\Utility\UpdateException;
use Drupal\file\Entity\File;
use PHPUnit\Framework\Attributes\Group;
use PHPUnit\Framework\Attributes\DataProvider;

/**
 * Checks the optional Fedora module upgrade without contacting Fedora.
 */
#[Group('islandora')]
class FedoraUpgradeTest extends IslandoraKernelTestBase {

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();
    require_once $this->container->get('extension.list.module')->getPath('islandora') . '/islandora.post_update.php';
  }

  /**
   * Ordinary sites never enable Fedora, even if settings mention it.
   */
  public function testNoFedoraFiles(): void {
    new Settings(['flysystem' => ['fedora' => ['driver' => 'fedora', 'config' => ['root' => 'http://fedora/']]]]);
    File::create(['uri' => 'public://example.txt', 'filename' => 'example.txt'])->save();
    islandora_post_update_enable_fcrepo();
    $this->assertFalse($this->container->get('module_handler')->moduleExists('islandora_fcrepo'));
  }

  /**
   * Existing fcrepo files require the optional module and its dependencies.
   */
  #[DataProvider('fedoraUris')]
  public function testExistingFedoraFiles(string $file_uri, array $settings): void {
    new Settings(['flysystem' => $settings]);
    // Insert directly: the previous stream wrapper need not be available yet.
    $this->container->get('database')->insert('file_managed')->fields([
      'uuid' => 'df3414ba-e34f-4dda-bb36-7cd963639132',
      'uid' => 0,
      'langcode' => 'en',
      'filename' => 'example.txt',
      'uri' => $file_uri,
      'filemime' => 'text/plain',
      'filesize' => 1,
      'status' => 1,
      'created' => 1,
      'changed' => 1,
    ])->execute();
    if (!interface_exists(AdapterDriverInterface::class)) {
      $this->expectException(UpdateException::class);
      $this->expectExceptionMessage('composer require');
    }
    islandora_post_update_enable_fcrepo();
    $this->assertTrue(\Drupal::moduleHandler()->moduleExists('islandora_fcrepo'));
    // The upgrade is safe to repeat and leaves the original URI intact.
    islandora_post_update_enable_fcrepo();
    $uri = \Drupal::database()->select('file_managed', 'f')->fields('f', ['uri'])->execute()->fetchField();
    $this->assertSame($file_uri, $uri);
  }

  /**
   * Current, historical, and custom Fedora storage scheme names.
   */
  public static function fedoraUris(): array {
    return [
      ['fcrepo://example.txt', []],
      ['fedora://example.txt', []],
      ['repository://example.txt', ['repository' => ['driver' => 'fedora', 'config' => ['root' => 'http://fedora/']]]],
    ];
  }

}
