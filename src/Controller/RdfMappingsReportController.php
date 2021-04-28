<?php

namespace Drupal\islandora\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\taxonomy\Entity\Vocabulary;
use Drupal\taxonomy\Entity\Term;
use Drupal\Core\Link;
use Drupal\Core\Url;

/**
 * RDF mappings report controller.
 */
class RdfMappingsReportController extends ControllerBase {

  /**
   * Output the RDF mappings report.
   *
   * @return string
   *   Markup of the tables.
   */
  public function main() {
    $markup = '';

    // Configured namespaces.
    $namespaces = rdf_get_namespaces();
    $namespaces_table_rows = [];
    foreach ($namespaces as $alias => $namespace_uri) {
      $namespaces_table_rows[] = [$alias, $namespace_uri];
    }
    $namespaces_table_header = [t('Namespace alias'), t('Namespace URI')];
    $namespaces_table = [
      '#theme' => 'table',
      '#header' => $namespaces_table_header,
      '#rows' => $namespaces_table_rows,
    ];
    $namespaces_table_markup = \Drupal::service('renderer')->render($namespaces_table);

    $markup .= '<details><summary>' . t('RDF namespaces used in field mappings') .
      '</summary><div class="details-wrapper">' . $namespaces_table_markup . '</div></details>';

    // Node and media field to RDF property mappings.
    $entity_types = ['node', 'media'];
    $markup .= '<h2>' . t('Field mappings') . '</h2>';
    foreach ($entity_types as $entity_type) {
      $bundles = \Drupal::service('entity_type.bundle.info')->getBundleInfo($entity_type);
      foreach ($bundles as $name => $attr) {
        $rdf_mappings = rdf_get_mapping($entity_type, $name);
        $rdf_types = $rdf_mappings->getPreparedBundleMapping();
        if (array_key_exists('types', $rdf_types) && count($rdf_types['types']) > 0) {
          $rdf_types = implode(', ', $rdf_types['types']);
          $markup .= '<h3>' . $attr['label'] . ' (' . $entity_type . ')' . ', mapped to RDF type ' . $rdf_types . '</h3>';
        }
        else {
          $markup .= '<h3>' . $attr['label'] . ' (' . $entity_type . ') - no RDF type mapping</h3>';
        }
        $fields = \Drupal::entityManager()->getFieldDefinitions($entity_type, $name);
        $mappings_table_rows = [];
        foreach ($fields as $field_name => $field_object) {
          $field_mappings = $rdf_mappings->getPreparedFieldMapping($field_name);
          if (array_key_exists('properties', $field_mappings)) {
            $properties = implode(', ', $field_mappings['properties']);
            $mappings_table_rows[] = [
              $field_object->getLabel() .
              ' (' . $field_name . ')',
              $properties,
            ];
          }
        }

        $mappings_header = [t('Drupal field'), t('RDF property')];

        if (count($mappings_table_rows) == 0) {
          $mappings_header = [];
          $mappings_table_rows[] = [t('No RDF mappings configured for @bundle.', ['@bundle' => $attr['label']])];
        }

        $mappings_table = [
          '#theme' => 'table',
          '#header' => $mappings_header,
          '#rows' => $mappings_table_rows,
        ];
        $mappings_table_markup = \Drupal::service('renderer')->render($mappings_table);
        $markup .= $mappings_table_markup;
      }
    }

    // Taxonomy terms with external URIs or authority links.
    $markup .= '<h2>' . t('Taxonomy terms with external URIs or authority links') . '</h2>';
    $utils = \Drupal::service('islandora.utils');
    $uri_fields = $utils->getUriFieldNamesForTerms();

    $vocabs = Vocabulary::loadMultiple();
    foreach ($vocabs as $vid => $vocab) {
      $rdf_mappings = rdf_get_mapping('taxonomy_term', $vid);
      $rdf_types = $rdf_mappings->getPreparedBundleMapping();
      $vocab_table_header = [];
      $vocab_table_rows = [];
      if (array_key_exists('types', $rdf_types) && count($rdf_types['types']) > 0) {
        $rdf_types = implode(', ', $rdf_types['types']);
        $markup .= '<h3>' . $vocab->label() . ' (' . $vid . ')' . ', mapped to RDF type ' . $rdf_types . '</h3>';
      }
      else {
        $markup .= '<h3>' . $vocab->label() . ' (' . $vid . ') - no RDF type mapping</h3>';
      }
      $terms = \Drupal::entityTypeManager()->getStorage('taxonomy_term')->loadTree($vid);
      if (count($terms) == 0) {
        $vocab_table_rows[] = [t('No terms in this vocabulary.')];
        $vocab_table = [
          '#theme' => 'table',
          '#header' => $vocab_table_header,
          '#rows' => $vocab_table_rows,
        ];
        $vocab_table_markup = \Drupal::service('renderer')->render($vocab_table);
        $markup .= $vocab_table_markup;
      }
      else {
        $vocab_table_header = [
          t('Term'),
          t('Term ID'),
          t('External URI or Authority link'),
        ];
        $vocab_table_rows = [];
        foreach ($terms as $t) {
          $ld_uri = NULL;
          $term = Term::load($t->tid);
          foreach ($uri_fields as $uri_field) {
            if ($term->hasField($uri_field) && !$term->get($uri_field)->isEmpty()) {
              $ld_uri = $term->get($uri_field)->first()->getValue();
              continue;
            }
          }
          if (is_array($ld_uri) && array_key_exists('uri', $ld_uri)) {
            $term_link = Link::fromTextAndUrl($term->getName(), Url::fromUri('internal:/taxonomy/term/' . $term->id()));
            $vocab_table_rows[] = [
              $term_link,
              $term->id(),
              $ld_uri['uri'],
            ];
          }
          else {
            $term_link = Link::fromTextAndUrl($term->getName(), Url::fromUri('internal:/taxonomy/term/' . $term->id()));
            $vocab_table_rows[] = [$term_link, $term->id(), t('None')];
          }
        }
        $vocab_table = [
          '#theme' => 'table',
          '#header' => $vocab_table_header,
          '#rows' => $vocab_table_rows,
        ];
      }
      $vocab_table_markup = \Drupal::service('renderer')->render($vocab_table);
      $markup .= $vocab_table_markup;
    }

    return ['#markup' => $markup];
  }

}
