<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\Core\Action\ActionBase;
use Drupal\Core\Database\Connection;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\islandora\MediaSource\MediaSourceService;
use Psr\Log\LoggerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\file\Entity\File;

/**
 * Deletes a media and its source file.
 *
 * @Action(
 *   id = "delete_media_and_file",
 *   label = @Translation("Delete media and file"),
 *   type = "media"
 * )
 */
class DeleteMediaAndFile extends ActionBase implements ContainerFactoryPluginInterface {

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSourceService;

  /**
   * Database connection.
   *
   * @var \Drupal\Core\Database\Connection
   */
  protected $connection;

  /**
   * Logger.
   *
   * @var Psr\Log\LoggerInterface
   */
  protected $logger;

  /**
   * Constructor.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin ID for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\islandora\MediaSource\MediaSourceService $media_source_service
   *   Media source service.
   * @param \Drupal\Core\Database\Connection $connection
   *   Database connection.
   * @param Psr\Log\LoggerInterface $logger
   *   Logger.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    MediaSourceService $media_source_service,
    Connection $connection,
    LoggerInterface $logger
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->mediaSourceService = $media_source_service;
    $this->connection = $connection;
    $this->logger = $logger;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('islandora.media_source_service'),
      $container->get('database'),
      $container->get('logger.channel.islandora')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function execute($entity = NULL) {
    if (!$entity) {
      return;
    }

    $transaction = $this->connection->startTransaction();

    try {
      // Delete all the source files and then the media.
      $source_field = $this->mediaSourceService->getSourceFieldName($entity->bundle());
      foreach ($entity->get($source_field)->referencedEntities() as $file) {
        $file->delete();
        // Check for non-source fields;
        foreach ($entity->getFieldDefinitions() as $field) {
          if ($field->getType() == 'file') {
            $fid = $entity->get($field->getName())->getValue()[0]['target_id'];
            if ($fid) {
              File::load($fid)->delete();
            }
          }
        }
      }
      $entity->delete();
    }
    catch (\Exception $e) {
      $transaction->rollBack();
      $this->logger->error("Cannot delete media and its files. Rolling back transaction: @msg", ["@msg" => $e->getMessage()]);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function access($object, AccountInterface $account = NULL, $return_as_object = FALSE) {
    return $object->access('delete', $account, $return_as_object);
  }

}
