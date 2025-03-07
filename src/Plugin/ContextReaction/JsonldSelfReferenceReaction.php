<?php

namespace Drupal\islandora\Plugin\ContextReaction;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\islandora\ContextReaction\NormalizerAlterReaction;
use Drupal\islandora\MediaSource\MediaSourceService;
use Drupal\jsonld\Normalizer\NormalizerBase;
use Drupal\media\MediaInterface;
use Drupal\islandora\IslandoraUtils;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Create a self-reference in RDF when creating JSON-LD.
 *
 * Formerly called "Map URI to predicate". Renamed for clarity.
 *
 * @ContextReaction(
 *   id = "islandora_map_uri_predicate",
 *   label = @Translation("JSON-LD self-reference")
 * )
 */
class JsonldSelfReferenceReaction extends NormalizerAlterReaction {

  const SELF_REFERENCE_PREDICATE = 'drupal_uri_predicate';

  /**
   * Media source service.
   *
   * @var \Drupal\islandora\MediaSource\MediaSourceService
   */
  protected $mediaSource;

  /**
   * {@inheritdoc}
   */
  public function __construct(array $configuration,
                              $plugin_id,
                              $plugin_definition,
                              ConfigFactoryInterface $config_factory,
                              IslandoraUtils $utils,
                              MediaSourceService $media_source) {

    parent::__construct(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $config_factory,
      $utils
    );
    $this->mediaSource = $media_source;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('config.factory'),
      $container->get('islandora.utils'),
      $container->get('islandora.media_source_service')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    return $this->t('When creating the JSON-LD for this Drupal entity, add a relationship to itself using this predicate.');
  }

  /**
   * {@inheritdoc}
   */
  public function execute(EntityInterface $entity = NULL, array &$normalized = NULL, array $context = NULL) {
    $config = $this->getConfiguration();
    $self_ref_predicate = $config[self::SELF_REFERENCE_PREDICATE];
    if (!is_null($self_ref_predicate) && !empty($self_ref_predicate)) {
      $url = $this->getSubjectUrl($entity);
      if ($context['needs_jsonldcontext'] === FALSE) {
        $self_ref_predicate = NormalizerBase::escapePrefix($self_ref_predicate, $context['namespaces']);
      }
      if (isset($normalized['@graph']) && is_array($normalized['@graph'])) {
        foreach ($normalized['@graph'] as &$graph) {
          if (isset($graph['@id']) && $graph['@id'] == $url) {
            // If a file URL is available, swap it in as the ID.
            if ($entity instanceof MediaInterface) {
              if ($file = $this->mediaSource->getSourceFile($entity)) {
                $graph['@id'] = $this->utils->getDownloadUrl($file);
              }
            }
            if (isset($graph[$self_ref_predicate])) {
              if (!is_array($graph[$self_ref_predicate])) {
                if ($graph[$self_ref_predicate] == $url) {
                  // Don't add it if it already exists.
                  return;
                }
                $tmp = $graph[$self_ref_predicate];
                $graph[$self_ref_predicate] = [$tmp];
              }
              elseif (array_search($url, array_column($graph[$self_ref_predicate], '@id'))) {
                // Don't add it if it already exists.
                return;
              }
            }
            else {
              $graph[$self_ref_predicate] = [];
            }
            $graph[$self_ref_predicate][] = ["@id" => $url];
            return;
          }
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $config = $this->getConfiguration();
    $form[self::SELF_REFERENCE_PREDICATE] = [
      '#type' => 'textfield',
      '#title' => $this->t('Self-reference predicate'),
      '#description' => $this->t("When creating the JSON-LD for this Drupal entity, add a relationship from the entity to itself using this predicate. It must use a defined RDF namespace prefix."),
      '#default_value' => isset($config[self::SELF_REFERENCE_PREDICATE]) ? $config[self::SELF_REFERENCE_PREDICATE] : '',
      '#size' => 35,
    ];
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigurationForm(array &$form, FormStateInterface $form_state) {
    $self_ref_predicate = $form_state->getValue(self::SELF_REFERENCE_PREDICATE);
    if (!is_null($self_ref_predicate) and !empty($self_ref_predicate)) {
      if (preg_match('/^https?:\/\//', $self_ref_predicate)) {
        // Can't validate all URIs so we have to trust them.
        return;
      }
      elseif (preg_match('/^([^\s:]+):/', $self_ref_predicate, $matches)) {
        $predicate_prefix = $matches[1];
        $rdf = rdf_get_namespaces();
        $rdf_prefixes = array_keys($rdf);
        if (!in_array($predicate_prefix, $rdf_prefixes)) {
          $form_state->setErrorByName(
            self::SELF_REFERENCE_PREDICATE,
            $this->t('Namespace prefix @prefix is not registered.',
              ['@prefix' => $predicate_prefix]
            )
          );
        }
      }
      else {
        $form_state->setErrorByName(
          self::SELF_REFERENCE_PREDICATE,
          $this->t('Predicate must use a defined prefix or be a full URI')
        );
      }
    }
    parent::validateConfigurationForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    $this->setConfiguration([self::SELF_REFERENCE_PREDICATE => $form_state->getValue(self::SELF_REFERENCE_PREDICATE)]);
  }

}
