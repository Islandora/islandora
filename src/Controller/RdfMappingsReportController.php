<?php

namespace Drupal\islandora\Controller;

use Drupal\Core\Controller\ControllerBase;

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

    $markup .= '<h3>RDF namespaces</h3>';
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

    $markup .= $namespaces_table_markup;

    return ['#markup' => $markup];
  }

}
