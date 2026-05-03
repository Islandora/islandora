<?php

namespace Drupal\Tests\islandora_iiif\Unit;

use Drupal\islandora_iiif\Plugin\views\style\IIIFManifest;
use PHPUnit\Framework\TestCase;

/**
 * Tests IIIF manifest image service metadata helpers.
 *
 * @group islandora_iiif
 */
class IIIFManifestTest extends TestCase {

  /**
   * Creates a manifest plugin instance without invoking the constructor.
   */
  protected function createManifestPlugin(): TestableIIIFManifest {
    return (new \ReflectionClass(TestableIIIFManifest::class))
      ->newInstanceWithoutConstructor();
  }

  /**
   * Tests the v2 image service descriptor.
   */
  public function testV2ImageServiceDescriptor(): void {
    $plugin = $this->createManifestPlugin();
    $url = 'https://example.test/iiif/2/example-id';

    $service = $plugin->publicGetImageServiceDescriptor($url);

    $this->assertSame($url, $service['@id']);
    $this->assertSame('http://iiif.io/api/image/2/context.json', $service['@context']);
    $this->assertSame('http://iiif.io/api/image/2/profiles/level2.json', $service['profile']);
  }

  /**
   * Tests the v3 image service descriptor.
   */
  public function testV3ImageServiceDescriptor(): void {
    $plugin = $this->createManifestPlugin();
    $url = 'https://example.test/iiif/3/example-id';

    $service = $plugin->publicGetImageServiceDescriptor($url);

    $this->assertSame($url, $service['@id']);
    $this->assertSame('http://iiif.io/api/image/3/context.json', $service['@context']);
    $this->assertSame('level2', $service['profile']);
  }

  /**
   * Tests the full image URL for Image API 2.
   */
  public function testV2FullImageUrl(): void {
    $plugin = $this->createManifestPlugin();
    $url = 'https://example.test/iiif/2/example-id';

    $this->assertSame(
      'https://example.test/iiif/2/example-id/full/full/0/default.jpg',
      $plugin->publicBuildFullImageUrl($url)
    );
  }

  /**
   * Tests the full image URL for Image API 3.
   */
  public function testV3FullImageUrl(): void {
    $plugin = $this->createManifestPlugin();
    $url = 'https://example.test/iiif/3/example-id';

    $this->assertSame(
      'https://example.test/iiif/3/example-id/full/max/0/default.jpg',
      $plugin->publicBuildFullImageUrl($url)
    );
  }

  /**
   * Tests the thumbnail URL.
   */
  public function testThumbnailUrl(): void {
    $plugin = $this->createManifestPlugin();
    $url = 'https://example.test/iiif/3/example-id';

    $this->assertSame(
      'https://example.test/iiif/3/example-id/full/200,/0/default.jpg',
      $plugin->publicBuildThumbnailUrl($url)
    );
  }

}

/**
 * Testable wrapper exposing protected helper methods.
 */
class TestableIIIFManifest extends IIIFManifest {

  /**
   * Exposes the protected service helper for unit testing.
   */
  public function publicGetImageServiceDescriptor(string $iiif_url): array {
    return $this->getImageServiceDescriptor($iiif_url);
  }

  /**
   * Exposes the protected full image URL helper for unit testing.
   */
  public function publicBuildFullImageUrl(string $iiif_url): string {
    return $this->buildFullImageUrl($iiif_url);
  }

  /**
   * Exposes the protected thumbnail URL helper for unit testing.
   */
  public function publicBuildThumbnailUrl(string $iiif_url): string {
    return $this->buildThumbnailUrl($iiif_url);
  }

}
