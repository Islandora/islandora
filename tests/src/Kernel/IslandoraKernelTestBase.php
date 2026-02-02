<?php

namespace Drupal\Tests\islandora\Kernel;

use Drupal\KernelTests\KernelTestBase;
use PHPUnit\Framework\Attributes\RunTestsInSeparateProcesses;

/**
 * Abstract base class for Islandora kernel tests.
 */
#[RunTestsInSeparateProcesses]
abstract class IslandoraKernelTestBase extends KernelTestBase {

  /**
   * {@inheritdoc}
   */
  protected static $modules = [
    'system',
    'user',
    'field',
    'filter',
    'block',
    'node',
    'path',
    'text',
    'options',
    'serialization',
    'rest',
    'basic_auth',
    'hal',
    'rdf',
    'action',
    'context',
    'jsonld',
    'views',
    'key',
    'jwt',
    'file',
    'image',
    'media',
    'islandora',
    'flysystem',
  ];

  /**
   * {@inheritdoc}
   */
  public function setUp(): void {
    parent::setUp();

    // Bootstrap minimal Drupal environment to run the tests.
    $this->installSchema('node', 'node_access');
    $this->installEntitySchema('user');
    $this->installEntitySchema('node');
    $this->installEntitySchema('context');
    $this->installEntitySchema('file');
    $this->installConfig('filter');
    $this->installConfig('rest');
  }

}
