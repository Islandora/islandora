<?php

namespace Drupal\islandora\Form;

use Drupal\Core\Access\AccessResult;
use Drupal\Core\Config\ImmutableConfig;
use Drupal\Core\Database\Connection;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityTypeBundleInfoInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Routing\RouteMatch;
use Drupal\Core\Routing\RouteMatchInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\Url;
use Drupal\Core\Utility\Token;
use Drupal\islandora\IslandoraUtils;
use Drupal\islandora\MediaSource\MediaSourceService;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\RedirectResponse;
use Symfony\Component\HttpKernel\Exception\HttpException;

/**
 * Form that lets users upload one or more files as children to a resource node.
 */
class AddMediaForm extends FormBase {

  /**
   * Entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * Entity field manager.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface
   */
  protected $entityFieldManager;

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSource;

  /**
   * Islandora utils.
   *
   * @var \Drupal\islandora\IslandoraUtils
   */
  protected $utils;

  /**
   * Islandora settings.
   *
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  protected $config;

  /**
   * Token service.
   *
   * @var \Drupal\Core\Utility\Token
   */
  protected $token;

  /**
   * Current user account.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $account;

  /**
   * Parent ID, cached to survive between batch operations.
   *
   * @var int
   */
  protected $parentId;

  /**
   * The route match object.
   *
   * @var \Drupal\Core\Routing\RouteMatchInterface
   */
  protected $routeMatch;

  /**
   * Database connection.
   *
   * @var \Drupal\Core\Database\Connection
   */
  protected $database;

  /**
   * To list the available bundle types.
   *
   * @var \Drupal\Core\Entity\EntityTypeBundleInfo
   */
  protected $entityTypeBundleInfo;

  /**
   * Constructs a new IslandoraUploadForm object.
   */
  public function __construct(
    EntityTypeManagerInterface $entity_type_manager,
    EntityFieldManagerInterface $entity_field_manager,
    IslandoraUtils $utils,
    MediaSourceService $media_source,
    ImmutableConfig $config,
    Token $token,
    AccountInterface $account,
    RouteMatchInterface $route_match,
    Connection $database,
    EntityTypeBundleInfoInterface $entity_type_bundle_info,
  ) {
    $this->entityTypeManager = $entity_type_manager;
    $this->entityFieldManager = $entity_field_manager;
    $this->utils = $utils;
    $this->mediaSource = $media_source;
    $this->config = $config;
    $this->token = $token;
    $this->account = $account;
    $this->routeMatch = $route_match;
    $this->database = $database;
    $this->entityTypeBundleInfo = $entity_type_bundle_info;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity_type.manager'),
      $container->get('entity_field.manager'),
      $container->get('islandora.utils'),
      $container->get('islandora.media_source_service'),
      $container->get('config.factory')->get('islandora.settings'),
      $container->get('token'),
      $container->get('current_user'),
      $container->get('current_route_match'),
      $container->get('database'),
      $container->get('entity_type.bundle.info')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'add_media_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $upload_pattern = $this->config->get(IslandoraSettingsForm::UPLOAD_FORM_LOCATION);
    $upload_location = $this->token->replace($upload_pattern);

    $valid_extensions = $this->config->get(IslandoraSettingsForm::UPLOAD_FORM_ALLOWED_MIMETYPES);

    $this->parentId = $this->routeMatch->getParameter('node');
    $parent = $this->entityTypeManager->getStorage('node')->load($this->parentId);

    // File upload widget.
    $form['upload'] = [
      '#type' => 'managed_file',
      '#title' => $this->t('File'),
      '#description' => $this->t("Upload one or more files to create media for @title", ['@title' => $parent->getTitle()]),
      '#upload_location' => $upload_location,
      '#upload_validators' => [
        'file_validate_extensions' => [$valid_extensions],
      ],
      '#required' => TRUE,
      '#multiple' => TRUE,
    ];

    $this->addMediaType($form);

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Submit'),
    ];

    return $form;
  }

  /**
   * Helper function to add media use checkboxes to the form.
   *
   * @param array $form
   *   Form array.
   */
  protected function addMediaType(array &$form) {
    // Drop down to select media type.
    $options = [];
    foreach ($this->entityTypeBundleInfo->getBundleInfo('media') as $bundle_id => $bundle) {
      $options[$bundle_id] = $bundle['label'];
    }
    $form['media_type'] = [
      '#type' => 'select',
      '#title' => $this->t('Media type'),
      '#description' => $this->t('Each media created will have this type.'),
      '#options' => $options,
      '#required' => TRUE,
    ];

    // Find bundles that don't have field_media_use.
    $bundles_with_media_use = [];
    foreach (array_keys($options) as $bundle) {
      $fields = $this->entityFieldManager->getFieldDefinitions('media', $bundle);
      if (isset($fields[IslandoraUtils::MEDIA_USAGE_FIELD])) {
        $bundles_with_media_use[] = $bundle;
      }
    }

    // Media use drop down.
    // Only shows up if the selected bundle has field_media_use.
    $options = [];
    $terms = $this->entityTypeManager->getStorage('taxonomy_term')->loadTree('islandora_media_use', 0, NULL, TRUE);
    foreach ($terms as $term) {
      $options[$term->id()] = $term->getName();
    }
    $form['use'] = [
      '#type' => 'checkboxes',
      '#title' => $this->t('Usage'),
      '#description' => $this->t("Defined by Portland Common Data Model: Use Extension https://pcdm.org/2015/05/12/use. ''Original File'' will trigger creation of derivatives."),
      '#options' => $options,
      '#states' => [
        'visible' => [],
        'required' => [],
      ],
    ];

    if (!empty($bundles_with_media_use)) {
      foreach ($bundles_with_media_use as $bundle) {
        $form['use']['#states']['visible'][] = [':input[name="media_type"]' => ['value' => $bundle]];
        $form['use']['#states']['required'][] = [':input[name="media_type"]' => ['value' => $bundle]];
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    // Get the parent.
    $parent_id = $this->routeMatch->getParameter('node');
    $parent = $this->entityTypeManager->getStorage('node')->load($parent_id);

    // Hack values out of the form.
    $fids = $form_state->getValue('upload');
    $media_type = $form_state->getValue('media_type');
    $tids = $form_state->getValue('use');

    // Create an operation for each uploaded file.
    $operations = [];
    foreach ($fids as $fid) {
      $operations[] = [
        [$this, 'buildMediaForFile'],
        [$fid, $parent_id, $media_type, $tids],
      ];
    }

    // Set up and trigger the batch.
    $batch = [
      'title' => $this->t("Creating Media for @title", ['@title' => $parent->getTitle()]),
      'operations' => $operations,
      'progress_message' => $this->t('Processed @current out of @total. Estimated time: @estimate.'),
      'error_message' => $this->t('The process has encountered an error.'),
      'finished' => [$this, 'buildMediaFinished'],
    ];
    batch_set($batch);
  }

  /**
   * Wires up a file/media combo for a file upload.
   *
   * @param int $fid
   *   Uploaded file id.
   * @param int $parent_id
   *   Id of the parent node.
   * @param string $media_type
   *   Meida type for the new media.
   * @param int[] $tids
   *   Array of Media Use term ids.
   * @param array $context
   *   Batch context.
   *
   * @throws \Symfony\Component\HttpKernel\Exception\HttpException
   */
  public function buildMediaForFile($fid, $parent_id, $media_type, array $tids, array &$context) {
    // Since we make 2 different entities, do this in a transaction.
    $transaction = $this->database->startTransaction();

    try {
      // Set the file to permanent.
      $file = $this->entityTypeManager->getStorage('file')->load($fid);
      $file->setPermanent();
      $file->save();

      // Make the media and assign it to the parent resource node.
      $parent = $this->entityTypeManager->getStorage('node')->load($parent_id);

      $source_field = $this->mediaSource->getSourceFieldName($media_type);

      $terms = $this->entityTypeManager->getStorage('taxonomy_term')->loadMultiple($tids);
      $media = $this->entityTypeManager->getStorage('media')->create([
        'bundle' => $media_type,
        'uid' => $this->account->id(),
        $source_field => $fid,
        'name' => $file->getFileName(),
        IslandoraUtils::MEDIA_OF_FIELD => $parent,
      ]);
      if ($media->hasField(IslandoraUtils::MEDIA_USAGE_FIELD)) {
        $media->set(IslandoraUtils::MEDIA_USAGE_FIELD, $terms);
      }
      $media->save();
    }
    catch (HttpException $e) {
      $transaction->rollBack();
      throw $e;
    }
    catch (\Exception $e) {
      $transaction->rollBack();
      throw new HttpException(500, $e->getMessage());
    }
  }

  /**
   * Batch finished callback.
   *
   * $success bool
   *   Success status
   * $results mixed
   *   The 'results' from the batch context.
   * $operations array
   *   Remaining operations.
   */
  public function buildMediaFinished($success, $results, $operations) {
    return new RedirectResponse(
      Url::fromRoute('view.media_of.page_1', ['node' => $this->parentId])->toString()
    );
  }

  /**
   * Check if the user can create any "Islandora" media.
   *
   * @param \Drupal\Core\Routing\RouteMatch $route_match
   *   The current routing match.
   *
   * @return \Drupal\Core\Access\AccessResultAllowed|\Drupal\Core\Access\AccessResultForbidden
   *   Whether we can or can't show the "thing".
   */
  public function access(RouteMatch $route_match) {
    if ($this->utils->canCreateIslandoraEntity('media', 'media_type')) {
      return AccessResult::allowed();
    }

    return AccessResult::forbidden();
  }

}
