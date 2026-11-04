<?php

namespace Drupal\Tests\islandora\Unit;

use Drupal\Core\DependencyInjection\ContainerBuilder;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\File\FileSystemInterface;
use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Language\LanguageManagerInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\Site\Settings;
use Drupal\Core\StreamWrapper\PrivateStream;
use Drupal\Core\StreamWrapper\StreamWrapperManager;
use Drupal\file\FileInterface;
use Drupal\file\Validation\FileValidatorInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\media\MediaInterface;
use Drupal\media\Entity\Media;
use Drupal\islandora_text_extraction\Controller\MediaSourceController as OcrController;
use Symfony\Component\HttpFoundation\Request;
use PHPUnit\Framework\TestCase;
use Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException;
use Symfony\Component\HttpKernel\Exception\BadRequestHttpException;
use Symfony\Component\Mime\MimeTypeGuesserInterface;

/**
 * Tests destination validation and authorization before filesystem writes.
 *
 * @group islandora
 */
class MediaSourceWriteAccessTest extends TestCase {

  /**
   * The media source service under test.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $service;
  /**
   * The managed file storage mock.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $files;
  /**
   * The account attempting file writes.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $account;
  /**
   * The temporary private file directory.
   *
   * @var string
   */
  protected $directory;

  /**
   * {@inheritdoc}
   */
  protected function setUp(): void {
    parent::setUp();
    $this->directory = sys_get_temp_dir() . '/islandora-write-' . uniqid();
    mkdir($this->directory);
    new Settings(['file_private_path' => $this->directory]);
    stream_wrapper_register('private', PrivateStream::class);
    $filesystem = $this->createMock(FileSystemInterface::class);
    $filesystem->method('dirname')->willReturn('private://');
    $filesystem->method('prepareDirectory')->willReturn(TRUE);
    $filesystem->method('basename')->willReturnCallback(fn($path, $suffix = NULL) => basename($path, $suffix ?? ''));
    $container = new ContainerBuilder();
    $container->set('file_system', $filesystem);
    \Drupal::setContainer($container);
    $this->files = $this->createMock(EntityStorageInterface::class);
    $images = $this->createMock(EntityStorageInterface::class);
    $images->method('loadMultiple')->willReturn([]);
    $manager = $this->createMock(EntityTypeManagerInterface::class);
    $manager->method('getStorage')->willReturnMap([
      ['file', $this->files],
      ['image_style', $images],
    ]);
    $this->account = $this->createMock(AccountInterface::class);
    $wrappers = $this->getMockBuilder(StreamWrapperManager::class)
      ->disableOriginalConstructor()->onlyMethods(['isValidUri'])->getMock();
    $wrappers->method('isValidUri')->willReturnCallback(fn($uri) => str_starts_with($uri, 'private://'));
    $mime = $this->createMock(MimeTypeGuesserInterface::class);
    $mime->method('guessMimeType')->willReturn('text/plain');
    $this->service = new MediaSourceService(
      $manager,
      $this->account,
      $this->createMock(LanguageManagerInterface::class),
      $filesystem,
      $this->createMock(IslandoraUtils::class),
      $this->createMock(FileValidatorInterface::class),
      $wrappers,
      $mime
    );
  }

  /**
   * {@inheritdoc}
   */
  protected function tearDown(): void {
    stream_wrapper_unregister('private');
    foreach (glob($this->directory . '/*') as $path) {
      unlink($path);
    }
    rmdir($this->directory);
    \Drupal::unsetContainer();
    parent::tearDown();
  }

  /**
   * Existing managed files must allow updates, including at the final sink.
   *
   * @dataProvider writeAccessCases
   */
  public function testManagedFileWriteAccess(bool $allowed, bool $at_sink): void {
    $uri = 'private://target.txt';
    file_put_contents($uri, 'ORIGINAL');
    $file = $this->createMock(FileInterface::class);
    $file->method('getFileUri')->willReturn($uri);
    $file->expects($this->atLeastOnce())->method('access')
      ->with('update', $this->account)->willReturn($allowed);
    $this->files->method('loadByProperties')->with(['uri' => $uri])->willReturn([$file]);
    try {
      if ($at_sink) {
        $resource = fopen('php://memory', 'w+');
        fwrite($resource, 'REPLACEMENT');
        rewind($resource);
        try {
          (new \ReflectionMethod($this->service, 'updateFile'))->invoke($this->service, $file, $resource);
        }
        finally {
          fclose($resource);
        }
      }
      else {
        $this->assertSame($uri, (new \ReflectionMethod($this->service, 'validateContentLocation'))->invoke($this->service, $uri));
      }
      $this->assertTrue($allowed, 'Unauthorized overwrite was allowed.');
    }
    catch (AccessDeniedHttpException $e) {
      $this->assertFalse($allowed);
    }
    $this->assertSame($allowed && $at_sink ? 'REPLACEMENT' : 'ORIGINAL', file_get_contents($uri));
  }

  /**
   * Provides allowed and denied writes at validation and the final write.
   */
  public static function writeAccessCases(): array {
    return [[FALSE, FALSE], [TRUE, FALSE], [FALSE, TRUE], [TRUE, TRUE]];
  }

  /**
   * Tests that unmanaged files cannot be overwritten.
   */
  public function testUnmanagedFileCannotBeOverwritten(): void {
    $uri = 'private://unmanaged.txt';
    file_put_contents($uri, 'ORIGINAL');
    $this->files->method('loadByProperties')->willReturn([]);
    try {
      (new \ReflectionMethod($this->service, 'validateContentLocation'))->invoke($this->service, $uri);
      $this->fail('Unmanaged overwrite was allowed.');
    }
    catch (AccessDeniedHttpException $e) {
      $this->assertSame('ORIGINAL', file_get_contents($uri));
    }
  }

  /**
   * Tests that a new destination passes validation without creating a file.
   */
  public function testNewFileIsAllowed(): void {
    $this->files->method('loadByProperties')->willReturn([]);
    $uri = 'private://new.txt';
    $this->assertSame($uri, (new \ReflectionMethod($this->service, 'validateContentLocation'))->invoke($this->service, $uri));
    $this->assertFileDoesNotExist($this->directory . '/new.txt');
  }

  /**
   * Tests that invalid destinations are rejected before file access.
   *
   * @dataProvider invalidLocations
   */
  public function testInvalidLocation(string $uri): void {
    $this->files->expects($this->never())->method('loadByProperties');
    $this->expectException(BadRequestHttpException::class);
    (new \ReflectionMethod($this->service, 'validateContentLocation'))->invoke($this->service, $uri);
  }

  /**
   * Provides malformed and non-canonical destination URIs.
   */
  public static function invalidLocations(): array {
    return [
      [''],
      ['/tmp/target.txt'],
      ['private://../target.txt'],
      ['private://subdir/../target.txt'],
      ["private://a\0.txt"],
    ];
  }

  /**
   * Tests that source updates require media update access.
   */
  public function testSourceUpdateRequiresMediaAccess(): void {
    $media = $this->createMock(MediaInterface::class);
    $media->method('access')->with('update', $this->account)->willReturn(FALSE);
    $this->files->expects($this->never())->method('loadByProperties');
    $this->expectException(AccessDeniedHttpException::class);
    $this->service->updateSourceField($media, NULL, 'text/plain');
  }

  /**
   * Tests that overwrites retain the existing managed file entity.
   */
  public function testExistingFileEntityIsReused(): void {
    $file = $this->createMock(FileInterface::class);
    $this->files->method('loadByProperties')->willReturn([$file]);
    $this->files->expects($this->never())->method('create');
    $this->assertSame($file, (new \ReflectionMethod($this->service, 'initializeFile'))->invoke($this->service, 'private://target.txt'));
  }

  /**
   * Tests that OCR attachments use the shared validated upload service.
   */
  public function testOcrUsesValidatedMediaUpload(): void {
    $service = $this->createMock(MediaSourceService::class);
    $media = $this->createMock(Media::class);
    $field = $this->createMock(FieldItemListInterface::class);
    $media->method('hasField')->with('field_ocr')->willReturn(TRUE);
    $media->method('get')->with('field_ocr')->willReturn($field);
    $field->method('access')->with('edit')->willReturn(TRUE);
    $field->expects($this->once())->method('setValue')->with('OCR');
    $media->expects($this->once())->method('save');
    $service->expects($this->once())->method('putToMedia')->with(
      $media, 'field_file', $this->callback(fn($body) => is_resource($body) && stream_get_contents($body) === 'OCR'),
      'text/plain', 'private://target.txt'
    );
    $request = Request::create('/', 'PUT', [], [], [], [
      'CONTENT_TYPE' => 'text/plain',
      'HTTP_CONTENT_LOCATION' => 'private://target.txt',
    ], 'OCR');
    $this->assertSame(200, (new OcrController($service))->attachToMedia($media, 'field_file', 'field_ocr', $request)->getStatusCode());
  }

  /**
   * Tests that missing OCR text fields are rejected before writing.
   */
  public function testOcrRejectsMissingTextFieldBeforeWriting(): void {
    $service = $this->createMock(MediaSourceService::class);
    $service->expects($this->never())->method('putToMedia');
    $media = $this->createMock(Media::class);
    $media->method('hasField')->willReturn(FALSE);
    $media->expects($this->never())->method('save');
    $this->expectException(BadRequestHttpException::class);
    (new OcrController($service))->attachToMedia($media, 'field_file', 'missing', Request::create('/', 'PUT', [], [], [], [], 'OCR'));
  }

  /**
   * Tests that inaccessible OCR text fields are rejected before writing.
   */
  public function testOcrRejectsDeniedTextFieldBeforeWriting(): void {
    $service = $this->createMock(MediaSourceService::class);
    $service->expects($this->never())->method('putToMedia');
    $media = $this->createMock(Media::class);
    $field = $this->createMock(FieldItemListInterface::class);
    $media->method('hasField')->willReturn(TRUE);
    $media->method('get')->willReturn($field);
    $field->method('access')->willReturn(FALSE);
    $media->expects($this->never())->method('save');
    $this->expectException(AccessDeniedHttpException::class);
    (new OcrController($service))->attachToMedia($media, 'field_file', 'field_ocr', Request::create('/', 'PUT', [], [], [], [], 'OCR'));
  }

}
