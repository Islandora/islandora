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
 * Map URI to predicate context reaction.
 *
 * @ContextReaction(
 *   id = "islandora_map_uri_predicate",
 *   label = @Translation("Map URI to predicate")
 * )
 */
class MappingUriPredicateReaction extends NormalizerAlterReaction {

  const URI_PREDICATE = 'drupal_uri_predicate';

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
    return $this->t('Map Drupal URI to configured predicate.');
  }

  /**
   * {@inheritdoc}
   */
  public function execute(EntityInterface $entity = NULL, array &$normalized = NULL, array $context = NULL) {
    $config = $this->getConfiguration();
    $drupal_predicate = $config[self::URI_PREDICATE];
    if (!is_null($drupal_predicate) && !empty($drupal_predicate)) {
      $url = $this->getSubjectUrl($entity);
      if ($context['needs_jsonldcontext'] === FALSE) {
        $drupal_predicate = NormalizerBase::escapePrefix($drupal_predicate, $context['namespaces']);
      }
      if (isset($normalized['@graph']) && is_array($normalized['@graph'])) {
        foreach ($normalized['@graph'] as &$graph) {
          if (isset($graph['@id']) && $graph['@id'] == $url) {
            // Swap media and file urls.
            if ($entity instanceof MediaInterface) {
              $file = $this->mediaSource->getSourceFile($entity);
              $graph['@id'] = $this->utils->getDownloadUrl($file);
            }
            if (isset($graph[$drupal_predicate])) {
              if (!is_array($graph[$drupal_predicate])) {
                if ($graph[$drupal_predicate] == $url) {
                  // Don't add it if it already exists.
                  return;
                }
                $tmp = $graph[$drupal_predicate];
                $graph[$drupal_predicate] = [$tmp];
              }
              elseif (array_search($url, array_column($graph[$drupal_predicate], '@id'))) {
                // Don't add it if it already exists.
                return;
              }
            }
            else {
              $graph[$drupal_predicate] = [];
            }
            $graph[$drupal_predicate][] = ["@id" => $url];
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
    $form[self::URI_PREDICATE] = [
      '#type' => 'textfield',
      '#title' => $this->t('Drupal URI predicate'),
      '#description' => $this->t("The Drupal object's URI will be added to the resource with this predicate. Must use a defined prefix."),
      '#default_value' => isset($config[self::URI_PREDICATE]) ? $config[self::URI_PREDICATE] : '',
      '#size' => 35,
    ];
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigurationForm(array &$form, FormStateInterface $form_state) {
    $drupal_predicate = $form_state->getValue(self::URI_PREDICATE);
    if (!is_null($drupal_predicate) and !empty($drupal_predicate)) {
      if (preg_match('/^https?:\/\//', $drupal_predicate)) {
        // Can't validate all URIs so we have to trust them.
        return;
      }
      elseif (preg_match('/^([^\s:]+):/', $drupal_predicate, $matches)) {
        $predicate_prefix = $matches[1];
        $rdf = rdf_get_namespaces();
        $rdf_prefixes = array_keys($rdf);
        if (!in_array($predicate_prefix, $rdf_prefixes)) {
          $form_state->setErrorByName(
            self::URI_PREDICATE,
            $this->t('Namespace prefix @prefix is not registered.',
              ['@prefix' => $predicate_prefix]
            )
          );
        }
      }
      else {
        $form_state->setErrorByName(
          self::URI_PREDICATE,
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
    $this->setConfiguration([self::URI_PREDICATE => $form_state->getValue(self::URI_PREDICATE)]);
  }

}
