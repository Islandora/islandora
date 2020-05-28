<?php

namespace Drupal\islandora\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\taxonomy\Entity\Vocabulary;
use Drupal\taxonomy\Entity\Term;

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
    $entity_types = ['node', 'media'];

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

    $markup .= '<h2>' . t('Field mappings') . '</h2>';
    foreach ($entity_types as $entity_type) {
      $bundles = \Drupal::service('entity_type.bundle.info')->getBundleInfo($entity_type);
      foreach ($bundles as $name => $attr) {
        $markup .= '<h3>' . $attr['label'] . ' (' . $entity_type . ')' . '</h3>';
        $rdf_mappings = rdf_get_mapping($entity_type, $name);
        $fields = \Drupal::entityManager()->getFieldDefinitions($entity_type, $name);
        $mappings_table_rows = [];
        foreach ($fields as $field_name => $field_object) {
          $field_mappings = $rdf_mappings->getPreparedFieldMapping($field_name);
          if (array_key_exists('properties', $field_mappings)) {
            $mappings_table_rows[] = [$field_object->getLabel() . ' (' . $field_name . ')', $field_mappings['properties'][0]];
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

    // Taxonomy terms with external URIs.
    $markup .= '<h2>' . t('Taxonomy terms with external URIs') . '</h2>';
    $utils = \Drupal::service('islandora.utils');
    $vocabs = Vocabulary::loadMultiple();
    foreach ($vocabs as $vid => $vocab) {
      $vocab_table_header = [];
      $vocab_table_rows = [];
      $markup .= '<h3>' . $vocab->label() . ' (' . $vid . ')</h3>';
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
        $vocab_table_header = [t('Term'), t('Term ID'), t('External URI')];
        $vocab_table_rows = [];
        foreach ($terms as $t) {
          $term = Term::load($t->tid);
          $uri = $utils->getUriForTerm($term);
          if (is_null($uri)) {
            $vocab_table_rows[] = [$term->getName(), $term->id(), t('None')];
          }
          else {
            $vocab_table_rows[] = [$term->getName(), $term->id(), $uri];
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
