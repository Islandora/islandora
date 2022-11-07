<?php

namespace Drupal\islandora\Form\AddChildrenWizard;

use Drupal\Core\Database\Connection;
use Drupal\Core\Datetime\DateFormatterInterface;
use Drupal\Core\DependencyInjection\DependencySerializationTrait;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Messenger\MessengerInterface;
use Drupal\Core\Session\AccountProxyInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\Core\Url;
use Drupal\file\FileInterface;
use Drupal\islandora\IslandoraUtils;
use Drupal\media\MediaInterface;
use Drupal\node\NodeInterface;
use Symfony\Component\HttpKernel\Exception\HttpException;
use Symfony\Component\HttpKernel\Exception\HttpExceptionInterface;

/**
 * Abstract addition batch processor.
 */
abstract class AbstractBatchProcessor {

  use FieldTrait;
  use DependencySerializationTrait;
  use StringTranslationTrait;

  /**
   * The entity type manager service.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface|null
   */
  protected ?EntityTypeManagerInterface $entityTypeManager = NULL;

  /**
   * The database connection serivce.
   *
   * @var \Drupal\Core\Database\Connection|null
   */
  protected ?Connection $database;

  /**
   * The current user.
   *
   * @var \Drupal\Core\Session\AccountProxyInterface|null
   */
  protected ?AccountProxyInterface $currentUser;

  /**
   * The messenger service.
   *
   * @var \Drupal\Core\Messenger\MessengerInterface
   */
  protected MessengerInterface $messenger;

  /**
   * The date formatter service.
   *
   * @var \Drupal\Core\Datetime\DateFormatterInterface
   */
  protected DateFormatterInterface $dateFormatter;

  /**
   * Constructor.
   */
  public function __construct(
    EntityTypeManagerInterface $entity_type_manager,
    Connection $database,
    AccountProxyInterface $current_user,
    MessengerInterface $messenger,
    DateFormatterInterface $date_formatter
  ) {
    $this->entityTypeManager = $entity_type_manager;
    $this->database = $database;
    $this->currentUser = $current_user;
    $this->messenger = $messenger;
    $this->dateFormatter = $date_formatter;
  }

  /**
   * Implements callback_batch_operation() for our child addition batch.
   */
  public function batchOperation($delta, $info, array $values, &$context) {
    $transaction = $this->database->startTransaction();

    try {
      $entities[] = $node = $this->getNode($info, $values);
      $entities[] = $this->createMedia($node, $info, $values);

      $context['results'] = array_merge_recursive($context['results'], [
        'validation_violations' => $this->validationClassification($entities),
      ]);
      $context['results']['count'] = ($context['results']['count'] ?? 0) + 1;
    }
    catch (HttpExceptionInterface $e) {
      $transaction->rollBack();
      throw $e;
    }
    catch (\Exception $e) {
      $transaction->rollBack();
      throw new HttpException(500, $e->getMessage(), $e);
    }
  }

  /**
   * Loads the file indicated.
   *
   * @param mixed $info
   *   Widget values.
   *
   * @return \Drupal\file\FileInterface|null
   *   The loaded file.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  protected function getFile($info) : ?FileInterface {
    return (is_array($info) && isset($info['target_id'])) ?
      $this->entityTypeManager->getStorage('file')->load($info['target_id']) :
      NULL;
  }

  /**
   * Get the node to which to attach our media.
   *
   * @param mixed $info
   *   Info from the widget used to create the request.
   * @param array $values
   *   Additional form inputs.
   *
   * @return \Drupal\node\NodeInterface
   *   The node to which to attach the created media.
   */
  abstract protected function getNode($info, array $values) : NodeInterface;

  /**
   * Get a name to use for bulk-created assets.
   *
   * @param mixed $info
   *   Widget values.
   * @param array $values
   *   Form values.
   *
   * @return string
   *   An applicable name.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  protected function getName($info, array $values) : string {
    $file = $this->getFile($info);
    return $file ? $file->getFilename() : strtr('Bulk ingest, {date}', [
      '{date}' => $this->dateFormatter->format(time(), 'long'),
    ]);
  }

  /**
   * Create a media referencing the given file, associated with the given node.
   *
   * @param \Drupal\node\NodeInterface $node
   *   The node to which the media should be associated.
   * @param mixed $info
   *   The widget info for the media source field.
   * @param array $values
   *   Values from the wizard, which should contain at least:
   *   - media_type: The machine name/ID of the media type as which to create
   *     the media
   *   - use: An array of the selected "media use" terms.
   *
   * @return \Drupal\media\MediaInterface
   *   The created media entity.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  protected function createMedia(NodeInterface $node, $info, array $values) : MediaInterface {
    $taxonomy_term_storage = $this->entityTypeManager->getStorage('taxonomy_term');

    // Create a media with the file attached and also pointing at the node.
    $field = $this->getField($values);

    $media_values = array_merge(
      [
        'bundle' => $values['media_type'],
        'name' => $this->getName($info, $values),
        IslandoraUtils::MEDIA_OF_FIELD => $node,
        IslandoraUtils::MEDIA_USAGE_FIELD => ($values['use'] ?
          $taxonomy_term_storage->loadMultiple($values['use']) :
          NULL),
        'uid' => $this->currentUser->id(),
        // XXX: Published... no constant?
        'status' => 1,
      ],
      [
        $field->getName() => [
          $info,
        ],
      ]
    );
    $media = $this->entityTypeManager->getStorage('media')->create($media_values);
    if ($media->save() !== SAVED_NEW) {
      throw new \Exception("Failed to create media.");
    }

    return $media;
  }

  /**
   * Helper to bulk process validatable entities.
   *
   * @param array $entities
   *   An array of entities to scan for validation violations.
   *
   * @return array
   *   An associative array mapping entity type IDs to entity IDs to a count
   *   of validation violations found on then given entity.
   */
  protected function validationClassification(array $entities) {
    $violations = [];

    foreach ($entities as $entity) {
      $entity_violations = $entity->validate();
      if ($entity_violations->count() > 0) {
        $violations[$entity->getEntityTypeId()][$entity->id()] = $entity_violations->count();
      }
    }

    return $violations;
  }

  /**
   * Implements callback_batch_finished() for our child addition batch.
   */
  public function batchProcessFinished($success, $results, $operations): void {
    if ($success) {
      foreach ($results['validation_violations'] ?? [] as $entity_type => $info) {
        foreach ($info as $id => $count) {
          $this->messenger->addWarning($this->formatPlural(
            $count,
            '1 validation error present in <a target="_blank" href=":uri">bulk created entity of type %type, with ID %id</a>.',
            '@count validation errors present in <a target="_blank" href=":uri">bulk created entity of type %type, with ID %id</a>.',
            [
              '%type' => $entity_type,
              ':uri' => Url::fromRoute("entity.{$entity_type}.canonical", [$entity_type => $id])->toString(),
              '%id' => $id,
            ]
          ));
        }
      }
    }
    else {
      $this->messenger->addError($this->t('Encountered an error when processing.'));
    }
  }

}
