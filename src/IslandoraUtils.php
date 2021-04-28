<?php

namespace Drupal\islandora;

use Drupal\context\ContextManager;
use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Entity\Query\QueryException;
use Drupal\Core\Entity\Query\QueryInterface;
use Drupal\Core\Language\LanguageManagerInterface;
use Drupal\Core\Site\Settings;
use Drupal\Core\Url;
use Drupal\file\FileInterface;
use Drupal\flysystem\FlysystemFactory;
use Drupal\islandora\ContextProvider\FileContextProvider;
use Drupal\islandora\ContextProvider\MediaContextProvider;
use Drupal\islandora\ContextProvider\NodeContextProvider;
use Drupal\islandora\ContextProvider\TermContextProvider;
use Drupal\media\MediaInterface;
use Drupal\node\NodeInterface;
use Drupal\taxonomy\TermInterface;

/**
 * Utility functions for figuring out when to fire derivative reactions.
 */
class IslandoraUtils {

  const EXTERNAL_URI_FIELD = 'field_external_uri';

  const MEDIA_OF_FIELD = 'field_media_of';

  const MEDIA_USAGE_FIELD = 'field_media_use';

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * The entity field manager.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface
   */
  protected $entityFieldManager;

  /**
   * Context manager.
   *
   * @var \Drupal\context\ContextManager
   */
  protected $contextManager;

  /**
   * Flysystem factory.
   *
   * @var \Drupal\flysystem\FlysystemFactory
   */
  protected $flysystemFactory;

  /**
   * Language manager.
   *
   * @var \Drupal\Core\Language\LanguageManagerInterface
   */
  protected $languageManager;

  /**
   * Constructor.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $entity_field_manager
   *   The entity field manager.
   * @param \Drupal\context\ContextManager $context_manager
   *   Context manager.
   * @param \Drupal\flysystem\FlysystemFactory $flysystem_factory
   *   Flysystem factory.
   * @param \Drupal\Core\Language\LanguageManagerInterface $language_manager
   *   Language manager.
   */
  public function __construct(
    EntityTypeManagerInterface $entity_type_manager,
    EntityFieldManagerInterface $entity_field_manager,
    ContextManager $context_manager,
    FlysystemFactory $flysystem_factory,
    LanguageManagerInterface $language_manager
  ) {
    $this->entityTypeManager = $entity_type_manager;
    $this->entityFieldManager = $entity_field_manager;
    $this->contextManager = $context_manager;
    $this->flysystemFactory = $flysystem_factory;
    $this->languageManager = $language_manager;
  }

  /**
   * Gets nodes that a media belongs to.
   *
   * @param \Drupal\media\MediaInterface $media
   *   The Media whose node you are searching for.
   *
   * @return \Drupal\node\NodeInterface
   *   Parent node.
   *
   * @throws \Drupal\Core\TypedData\Exception\MissingDataException
   *   Method $field->first() throws if data structure is unset and no item can
   *   be created.
   */
  public function getParentNode(MediaInterface $media) {
    if (!$media->hasField(self::MEDIA_OF_FIELD)) {
      return NULL;
    }
    $field = $media->get(self::MEDIA_OF_FIELD);
    if ($field->isEmpty()) {
      return NULL;
    }
    $parent = $field->first()
      ->get('entity')
      ->getTarget();
    if (!is_null($parent)) {
      return $parent->getValue();
    }
    return NULL;
  }

  /**
   * Gets media that belong to a node.
   *
   * @param \Drupal\node\NodeInterface $node
   *   The parent node.
   *
   * @return \Drupal\media\MediaInterface[]
   *   The children Media.
   *
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   *   Calling getStorage() throws if the entity type doesn't exist.
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   *   Calling getStorage() throws if the storage handler couldn't be loaded.
   */
  public function getMedia(NodeInterface $node) {
    if (!$this->entityTypeManager->getStorage('field_storage_config')
      ->load('media.' . self::MEDIA_OF_FIELD)) {
      return [];
    }
    $mids = $this->entityTypeManager->getStorage('media')->getQuery()
      ->condition(self::MEDIA_OF_FIELD, $node->id())
      ->execute();
    if (empty($mids)) {
      return [];
    }
    return $this->entityTypeManager->getStorage('media')->loadMultiple($mids);
  }

  /**
   * Gets media that belong to a node with the specified term.
   *
   * @param \Drupal\node\NodeInterface $node
   *   The parent node.
   * @param \Drupal\taxonomy\TermInterface $term
   *   Taxonomy term.
   *
   * @return \Drupal\media\MediaInterface
   *   The child Media.
   *
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   *   Calling getStorage() throws if the entity type doesn't exist.
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   *   Calling getStorage() throws if the storage handler couldn't be loaded.
   */
  public function getMediaWithTerm(NodeInterface $node, TermInterface $term) {
    $mids = $this->getMediaReferencingNodeAndTerm($node, $term);
    if (empty($mids)) {
      return NULL;
    }
    return $this->entityTypeManager->getStorage('media')->load(reset($mids));
  }

  /**
   * Gets Media that reference a File.
   *
   * @param int $fid
   *   File id.
   *
   * @return \Drupal\media\MediaInterface[]
   *   Array of media.
   *
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   *   Calling getStorage() throws if the entity type doesn't exist.
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   *   Calling getStorage() throws if the storage handler couldn't be loaded.
   */
  public function getReferencingMedia($fid) {
    // Get media fields that reference files.
    $fields = $this->getReferencingFields('media', 'file');

    // Process field names, stripping off 'media.' and appending 'target_id'.
    $conditions = array_map(
      function ($field) {
        return ltrim($field, 'media.') . '.target_id';
      },
      $fields
    );

    // Query for media that reference this file.
    $query = $this->entityTypeManager->getStorage('media')->getQuery();
    $group = $query->orConditionGroup();
    foreach ($conditions as $condition) {
      $group->condition($condition, $fid);
    }
    $query->condition($group);

    return $this->entityTypeManager->getStorage('media')
      ->loadMultiple($query->execute());
  }

  /**
   * Gets the taxonomy term associated with an external uri.
   *
   * @param string $uri
   *   External uri.
   *
   * @return \Drupal\taxonomy\TermInterface|null
   *   Term or NULL if not found.
   *
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   *   Calling getStorage() throws if the entity type doesn't exist.
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   *   Calling getStorage() throws if the storage handler couldn't be loaded.
   */
  public function getTermForUri($uri) {
    // Get authority link fields to search.
    $field_map = $this->entityFieldManager->getFieldMap();
    $fields = [];
    foreach ($field_map['taxonomy_term'] as $field_name => $field_data) {
      if ($field_data['type'] == 'authority_link') {
        $fields[] = $field_name;
      }
    }
    // Add field_external_uri.
    $fields[] = self::EXTERNAL_URI_FIELD;

    $query = $this->entityTypeManager->getStorage('taxonomy_term')->getQuery();

    $orGroup = $query->orConditionGroup();
    foreach ($fields as $field) {
      $orGroup->condition("$field.uri", $uri);
    }

    $results = $query
      ->condition($orGroup)
      ->execute();

    if (empty($results)) {
      return NULL;
    }

    return $this->entityTypeManager->getStorage('taxonomy_term')
      ->load(reset($results));
  }

  /**
   * Gets the taxonomy term associated with an external uri.
   *
   * @param \Drupal\taxonomy\TermInterface $term
   *   Taxonomy term.
   *
   * @return string|null
   *   URI or NULL if not found.
   *
   * @throws \Drupal\Core\TypedData\Exception\MissingDataException
   *   Method $field->first() throws if data structure is unset and no item can
   *   be created.
   */
  public function getUriForTerm(TermInterface $term) {
    $fields = $this->getUriFieldNamesForTerms();
    foreach ($fields as $field_name) {
      if ($term && $term->hasField($field_name)) {
        $field = $term->get($field_name);
        if (!$field->isEmpty()) {
          $link = $field->first()->getValue();
          return $link['uri'];
        }
      }
    }
    return NULL;
  }

  /**
   * Gets every field name that might contain an external uri for a term.
   *
   * @return string[]
   *   Field names for fields that a term may have as an external uri.
   */
  public function getUriFieldNamesForTerms() {
    // Get authority link fields to search.
    $field_map = $this->entityFieldManager->getFieldMap();
    $fields = [];
    foreach ($field_map['taxonomy_term'] as $field_name => $field_data) {
      $data_types = ['authority_link', 'field_external_authority_link'];
      if (in_array($field_data['type'], $data_types)) {
        $fields[] = $field_name;
      }
    }
    // Add field_external_uri.
    $fields[] = self::EXTERNAL_URI_FIELD;
    return $fields;
  }

  /**
   * Executes context reactions for a Node.
   *
   * @param string $reaction_type
   *   Reaction type.
   * @param \Drupal\node\NodeInterface $node
   *   Node to evaluate contexts and pass to reaction.
   */
  public function executeNodeReactions($reaction_type, NodeInterface $node) {
    $provider = new NodeContextProvider($node);
    $provided = $provider->getRuntimeContexts([]);
    $this->contextManager->evaluateContexts($provided);

    // Fire off index reactions.
    foreach ($this->contextManager->getActiveReactions($reaction_type) as $reaction) {
      $reaction->execute($node);
    }
  }

  /**
   * Executes context reactions for a Media.
   *
   * @param string $reaction_type
   *   Reaction type.
   * @param \Drupal\media\MediaInterface $media
   *   Media to evaluate contexts and pass to reaction.
   */
  public function executeMediaReactions($reaction_type, MediaInterface $media) {
    $provider = new MediaContextProvider($media);
    $provided = $provider->getRuntimeContexts([]);
    $this->contextManager->evaluateContexts($provided);

    // Fire off index reactions.
    foreach ($this->contextManager->getActiveReactions($reaction_type) as $reaction) {
      $reaction->execute($media);
    }
  }

  /**
   * Executes context reactions for a File.
   *
   * @param string $reaction_type
   *   Reaction type.
   * @param \Drupal\file\FileInterface $file
   *   File to evaluate contexts and pass to reaction.
   */
  public function executeFileReactions($reaction_type, FileInterface $file) {
    $provider = new FileContextProvider($file);
    $provided = $provider->getRuntimeContexts([]);
    $this->contextManager->evaluateContexts($provided);

    // Fire off index reactions.
    foreach ($this->contextManager->getActiveReactions($reaction_type) as $reaction) {
      $reaction->execute($file);
    }
  }

  /**
   * Executes context reactions for a File.
   *
   * @param string $reaction_type
   *   Reaction type.
   * @param \Drupal\taxonomy\TermInterface $term
   *   Term to evaluate contexts and pass to reaction.
   */
  public function executeTermReactions($reaction_type, TermInterface $term) {
    $provider = new TermContextProvider($term);
    $provided = $provider->getRuntimeContexts([]);
    $this->contextManager->evaluateContexts($provided);

    // Fire off index reactions.
    foreach ($this->contextManager->getActiveReactions($reaction_type) as $reaction) {
      $reaction->execute($term);
    }
  }

  /**
   * Executes derivative reactions for a Media and Node.
   *
   * @param string $reaction_type
   *   Reaction type.
   * @param \Drupal\node\NodeInterface $node
   *   Node to pass to reaction.
   * @param \Drupal\media\MediaInterface $media
   *   Media to evaluate contexts.
   */
  public function executeDerivativeReactions($reaction_type, NodeInterface $node, MediaInterface $media) {
    $provider = new MediaContextProvider($media);
    $provided = $provider->getRuntimeContexts([]);
    $this->contextManager->evaluateContexts($provided);

    // Fire off index reactions.
    foreach ($this->contextManager->getActiveReactions($reaction_type) as $reaction) {
      $reaction->execute($node);
    }
  }

  /**
   * Evaluates if fields have changed between two instances of a ContentEntity.
   *
   * @param \Drupal\Core\Entity\ContentEntityInterface $entity
   *   The updated entity.
   * @param \Drupal\Core\Entity\ContentEntityInterface $original
   *   The original entity.
   *
   * @return bool
   *   TRUE if the fields have changed.
   */
  public function haveFieldsChanged(ContentEntityInterface $entity, ContentEntityInterface $original) {

    $field_definitions = $this->entityFieldManager->getFieldDefinitions($entity->getEntityTypeId(), $entity->bundle());

    $ignore_list = ['vid' => 1, 'changed' => 1, 'path' => 1];
    $field_definitions = array_diff_key($field_definitions, $ignore_list);

    foreach ($field_definitions as $field_name => $field_definition) {
      $langcodes = array_keys($entity->getTranslationLanguages());

      if ($langcodes !== array_keys($original->getTranslationLanguages())) {
        // If the list of langcodes has changed, we need to save.
        return TRUE;
      }

      foreach ($langcodes as $langcode) {
        $items = $entity
          ->getTranslation($langcode)
          ->get($field_name)
          ->filterEmptyItems();
        $original_items = $original
          ->getTranslation($langcode)
          ->get($field_name)
          ->filterEmptyItems();

        // If the field items are not equal, we need to save.
        if (!$items->equals($original_items)) {
          return TRUE;
        }
      }
    }

    return FALSE;
  }

  /**
   * Returns a list of all available filesystem schemes.
   *
   * @return String[]
   *   List of all available filesystem schemes.
   */
  public function getFilesystemSchemes() {
    $schemes = ['public'];
    if (!empty(Settings::get('file_private_path'))) {
      $schemes[] = 'private';
    }
    return array_merge($schemes, $this->flysystemFactory->getSchemes());
  }

  /**
   * Get array of media ids that have fields that reference $node and $term.
   *
   * @param \Drupal\node\NodeInterface $node
   *   The node to reference.
   * @param \Drupal\taxonomy\TermInterface $term
   *   The term to reference.
   *
   * @return array|int|null
   *   Array of media IDs or NULL.
   */
  public function getMediaReferencingNodeAndTerm(NodeInterface $node, TermInterface $term) {
    $term_fields = $this->getReferencingFields('media', 'taxonomy_term');
    if (count($term_fields) <= 0) {
      \Drupal::logger("No media fields reference a taxonomy term");
      return NULL;
    }
    $node_fields = $this->getReferencingFields('media', 'node');
    if (count($node_fields) <= 0) {
      \Drupal::logger("No media fields reference a node.");
      return NULL;
    }

    $remove_entity = function (&$o) {
      $o = substr($o, strpos($o, '.') + 1);
    };
    array_walk($term_fields, $remove_entity);
    array_walk($node_fields, $remove_entity);

    $query = $this->entityTypeManager->getStorage('media')->getQuery();
    $taxon_condition = $this->getEntityQueryOrCondition($query, $term_fields, $term->id());
    $query->condition($taxon_condition);
    $node_condition = $this->getEntityQueryOrCondition($query, $node_fields, $node->id());
    $query->condition($node_condition);
    // Does the tags field exist?
    try {
      $mids = $query->execute();
    }
    catch (QueryException $e) {
      $mids = [];
    }
    return $mids;
  }

  /**
   * Get the fields on an entity of $entity_type that reference a $target_type.
   *
   * @param string $entity_type
   *   Type of entity to search for.
   * @param string $target_type
   *   Type of entity the field references.
   *
   * @return array
   *   Array of fields.
   */
  public function getReferencingFields($entity_type, $target_type) {
    $fields = $this->entityTypeManager->getStorage('field_storage_config')->getQuery()
      ->condition('entity_type', $entity_type)
      ->condition('settings.target_type', $target_type)
      ->execute();
    if (!is_array($fields)) {
      $fields = [$fields];
    }
    return $fields;
  }

  /**
   * Make an OR condition for an array of fields and a value.
   *
   * @param \Drupal\Core\Entity\Query\QueryInterface $query
   *   The QueryInterface for the query.
   * @param array $fields
   *   The array of field names.
   * @param string $value
   *   The value to search the fields for.
   *
   * @return \Drupal\Core\Entity\Query\ConditionInterface
   *   The OR condition to add to your query.
   */
  private function getEntityQueryOrCondition(QueryInterface $query, array $fields, $value) {
    $condition = $query->orConditionGroup();
    foreach ($fields as $field) {
      $condition->condition($field, $value);
    }
    return $condition;
  }

  /**
   * Gets the id URL of an entity.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity whose URL you want.
   *
   * @return string
   *   The entity URL.
   *
   * @throws \Drupal\Core\Entity\Exception\UndefinedLinkTemplateException
   *   Thrown if the given entity does not specify a "canonical" template.
   * @throws \Drupal\Core\Entity\EntityMalformedException
   */
  public function getEntityUrl(EntityInterface $entity) {
    $undefined = $this->languageManager->getLanguage('und');
    return $entity->toUrl('canonical', [
      'absolute' => TRUE,
      'language' => $undefined,
    ])->toString();
  }

  /**
   * Gets the downloadable URL for a file.
   *
   * @param \Drupal\file\FileInterface $file
   *   The file whose URL you want.
   *
   * @return string
   *   The file URL.
   */
  public function getDownloadUrl(FileInterface $file) {
    return $file->createFileUrl(FALSE);
  }

  /**
   * Gets the URL for an entity's REST endpoint.
   *
   * @param \Drupal\Core\Entity\EntityInterface $entity
   *   The entity whose REST endpoint you want.
   * @param string $format
   *   REST serialization format.
   *
   * @return string
   *   The REST URL.
   */
  public function getRestUrl(EntityInterface $entity, $format = '') {
    $undefined = $this->languageManager->getLanguage('und');
    $entity_type = $entity->getEntityTypeId();
    $rest_url = Url::fromRoute(
      "rest.entity.$entity_type.GET",
      [$entity_type => $entity->id()],
      ['absolute' => TRUE, 'language' => $undefined]
    )->toString();
    if (!empty($format)) {
      $rest_url .= "?_format=$format";
    }
    return $rest_url;
  }

}
