<?php

namespace Drupal\islandora_advanced_search\Plugin\Block;

use Drupal\Core\Block\BlockBase;
use Drupal\Core\Form\FormBuilderInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\search_api\Display\DisplayPluginManager;
use Drupal\views\Entity\View;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\Request;

/**
 * Provides an Islandora Advanced Search block.
 *
 * @Block(
 *  id = "islandora_advanced_search_block",
 *  deriver = "Drupal\islandora_advanced_search\Plugin\Block\AdvancedSearchBlockDeriver",
 *  admin_label = @Translation("Islandora Advanced Search"),
 *  category = @Translation("Islandora"),
 * )
 */
class AdvancedSearchBlock extends BlockBase implements ContainerFactoryPluginInterface {
  use ViewAndDisplayIdentifiersTrait;

  // CSS classes used to bind table-drag behavior to.
  const WEIGHT_FIELD_CLASS = 'field-weight';
  const DISPLAY_FIELD_CLASS = 'field-display';

  // Regions in the table which denote if a given field
  // is visible in the Advanced Search Form or not.
  const REGION_VISIBLE = 'visible';
  const REGION_HIDDEN = 'hidden';

  // Keys for settings.
  const SETTING_FIELDS = 'fields';
  const SETTING_CONTEXTUAL_FILTER = 'context_filter';

  /**
   * The display plugin manager.
   *
   * @var \Drupal\search_api\Display\DisplayPluginManager
   */
  protected $displayPluginManager;

  /**
   * The clone of the current request object.
   *
   * @var \Symfony\Component\HttpFoundation\Request
   */
  protected $request;

  /**
   * The view this block affects.
   *
   * @var \Drupal\views\Entity\View
   */
  protected $view;

  /**
   * The view display this block affects.
   *
   * @var array
   */
  protected $display;

  /**
   * Form Builder.
   *
   * @var \Drupal\Core\Form\FormBuilderInterface
   */
  protected $formBuilder;

  /**
   * Construct a AdvancedSearchBlock instance.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param string $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\search_api\Display\DisplayPluginManager $display_plugin_manager
   *   The display plugin manager.
   * @param \Drupal\Core\Form\FormBuilderInterface $form_builder
   *   The form builder service used to build the search form.
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   A request object for the current request.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, DisplayPluginManager $display_plugin_manager, FormBuilderInterface $form_builder, Request $request) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->displayPluginManager = $display_plugin_manager;
    list($view_id, $display_id) = preg_split('/__/', $this->getDerivativeId(), 2);
    $this->view = View::Load($view_id);
    $this->display = $this->view->getDisplay($display_id);
    $this->formBuilder = $form_builder;
    $this->request = clone $request;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('plugin.manager.search_api.display'),
      $container->get('form_builder'),
      $container->get('request_stack')->getMasterRequest()
    );
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return [
      self::SETTING_FIELDS => [],
      self::SETTING_CONTEXTUAL_FILTER => NULL,
    ];
  }

  /**
   * Fields which can be enabled / disabled for display in the search form.
   *
   * @return \Drupal\search_api\Item\FieldInterface[]
   *   The $fields sorted by label.
   */
  protected function getFields() {
    $fields = $this->getIndex()->getFields();
    // First pass sort on label, secondary sort will be used
    // when looking at existing configuration for this block.
    uasort($fields, function ($a, $b) {
      return strcmp($a->getLabel(), $b->getLabel());
    });
    return $fields;
  }

  /**
   * Get regions of table to display.
   *
   * @return array
   *   The properties of each region used for building the table of fields.
   */
  protected function getRegions() {
    // Classes for select fields like 'weight' and 'display' are hard-coded
    // and used in js/islandora-advanced-search.admin.js.
    return [
      'visible' => [
        'title' => $this->t('Visible'),
        'invisible' => TRUE,
        'message' => $this->t('No search field is visible.'),
        'weight' => self::WEIGHT_FIELD_CLASS . '-visible',
        'display' => self::DISPLAY_FIELD_CLASS . '-visible',
      ],
      'hidden' => [
        'title' => $this->t('Hidden'),
        'invisible' => FALSE,
        'message' => $this->t('No search field is hidden.'),
        'weight' => self::WEIGHT_FIELD_CLASS . '-hidden',
        'display' => self::DISPLAY_FIELD_CLASS . '-hidden',
      ],
    ];
  }

  /**
   * Options for field display derived from the available regions.
   *
   * @return array
   *   Display select field options.
   */
  protected function getDisplayOptions() {
    $options = [];
    foreach ($this->getRegions() as $region => $settings) {
      $options[$region] = $settings['title'];
    }
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function blockForm($form, FormStateInterface $form_state) {
    // At most we will have one row per field.
    $fields = $this->getFields();
    $weight_delta = round(count($fields) / 2);

    // Group each field into a region given our current configuration.
    $visible_fields = $this->configuration[self::SETTING_FIELDS];
    $regions = $this->getRegions();
    $display_options = $this->getDisplayOptions();

    // Field rows are grouped by the region in which they are displayed.
    $field_rows = array_fill_keys(array_keys($regions), []);
    foreach ($fields as $field) {
      // If a field exists in the blocks configuration than it is 'visible' and
      // its weight is equivalent to its order in the configuration,
      // i.e. its index.
      $identifier = $field->getFieldIdentifier();
      $weight = array_search($identifier, $visible_fields);
      $visible = $weight !== FALSE;
      $region = $visible ? self::REGION_VISIBLE : self::REGION_HIDDEN;
      $field_rows[$region][$identifier] = [
        '#attributes' => [
          'class' => ['draggable'],
        ],
        'label' => ['#plain_text' => $field->getLabel()],
        'identifier' => ['#plain_text' => $identifier],
        'weight' => [
          '#type' => 'weight',
          '#title' => $this->t('Weight'),
          '#title_display' => 'invisible',
          '#default_value' => $visible ? $weight : 0,
          '#delta' => $weight_delta,
          '#attributes' => [
            'class' => [self::WEIGHT_FIELD_CLASS, $regions[$region]['weight']],
          ],
        ],
        'display' => [
          '#type' => 'select',
          '#title' => $this->t('Display'),
          '#title_display' => 'invisible',
          '#options' => $display_options,
          '#default_value' => $region,
          '#attributes' => [
            'class' => [self::DISPLAY_FIELD_CLASS, $regions[$region]['display']],
          ],
        ],
      ];
    }
    // Sort the visible rows by their weight.
    uasort($field_rows[self::REGION_VISIBLE], function ($a, $b) {
      $a = $a['weight']['#default_value'];
      $b = $b['weight']['#default_value'];
      if ($a == $b) {
        return 0;
      }
      return ($a < $b) ? -1 : 1;
    });

    // Build Rows.
    $rows = [];
    $table_drag = [];
    foreach ($regions as $region => $properties) {
      $rows += [
        // Conditionally display region title as a row.
        "region-$region" => $properties['invisible'] ? NULL : [
          '#attributes' => [
            'class' => ['region-title', "region-title-$region"],
          ],
          'label' => [
            '#plain_text' => $properties['title'],
            '#wrapper_attributes' => [
              'colspan' => 4,
            ],
          ],
        ],
        // Will dynamically display if the region has fields or not controlled
        // by Drupal behaviors in js/islandora-advanced-search.admin.js.
        "region-$region-message" => [
          '#attributes' => [
            'class' => [
              'region-message',
              "region-$region-message",
              empty($field_rows[$region]) ? 'region-empty' : 'region-populated',
            ],
          ],
          'message' => [
            '#markup' => '<em>' . $properties['message'] . '</em>',
            '#wrapper_attributes' => [
              'colspan' => 4,
            ],
          ],
        ],
      ];

      // Include field rows in this region.
      $rows += $field_rows[$region];

      // Configure order by weight field in region.
      $table_drag[] = [
        'action' => 'order',
        'relationship' => 'sibling',
        'group' => self::WEIGHT_FIELD_CLASS,
        'subgroup' => $properties['weight'],
        'source' => self::WEIGHT_FIELD_CLASS,
      ];

      // Configure drag action for display field in region.
      $table_drag[] = [
        'action' => 'match',
        'relationship' => 'sibling',
        'group' => self::DISPLAY_FIELD_CLASS,
        'subgroup' => $properties['display'],
        'source' => self::DISPLAY_FIELD_CLASS,
      ];
    }

    $form[self::SETTING_FIELDS] = [
      '#type' => 'table',
      '#attributes' => [
        // Identifier is hard-coded and used in
        // js/islandora-advanced-search.admin.js.
        'id' => 'advanced-search-fields',
      ],
      '#header' => [
        $this->t('Label'),
        $this->t('Field'),
        $this->t('Weight'),
        $this->t('Display'),
      ],
      '#empty' => $this->t('No search fields, please check search index configuration.'),
      '#tabledrag' => $table_drag,
    ] + $rows;

    // If there is contextual filters associated with the display that means
    // we can filter on collection / sub-collection. Allow the user to choose
    // which filters collections.
    $id = NULL;
    $field = NULL;
    $options = [];
    if (isset($this->display['display_options']['arguments'])) {
      foreach ($this->display['display_options']['arguments'] as $context_filter) {
        $id = $context_filter['id'];
        $field = $context_filter['field'];
        if (isset($fields[$field])) {
          $options[$id] = $fields[$field]->getLabel() . ':' . $id;
        }
      }
    }
    if (count($options) > 1) {
      $form[self::SETTING_CONTEXTUAL_FILTER] = [
        '#type' => 'select',
        '#title' => $this->t('Context Filter'),
        '#description' => $this->t('If more than one <strong>Context Filter</strong> is defined, specify which is used to <strong>include</strong> only <strong>direct children</strong> of the Collection as it will disabled to allow recursive searching.'),
        '#options' => $options,
        '#default_value' => $this->configuration[self::SETTING_CONTEXTUAL_FILTER],
        '#multiple' => FALSE,
        '#required' => TRUE,
        '#size' => count($options) + 1,
      ];
    }
    $form['#attributes']['class'][] = 'clearfix';
    $form['#attached']['library'][] = 'islandora_advanced_search/advanced.search.admin';
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function blockSubmit($form, FormStateInterface $form_state) {
    $values = $form_state->getValues();
    $fields = array_filter($values[self::SETTING_FIELDS], function ($field) {
      return $field['display'] == 'visible';
    });
    uasort($fields, '\Drupal\Component\Utility\SortArray::sortByWeightElement');
    $this->configuration[self::SETTING_FIELDS] = array_keys($fields);
    if (isset($values[self::SETTING_CONTEXTUAL_FILTER])) {
      $this->configuration[self::SETTING_CONTEXTUAL_FILTER] = $values[self::SETTING_CONTEXTUAL_FILTER];
    }
  }

  /**
   * {@inheritdoc}
   */
  public function build() {
    $fields = $this->getIndex()->getFields();
    $configured_fields = [];
    foreach ($this->configuration[self::SETTING_FIELDS] as $identifier) {
      $configured_fields[$identifier] = $fields[$identifier];
    }
    return $this->formBuilder->getForm('Drupal\islandora_advanced_search\Form\AdvancedSearchForm', $this->view, $this->display, $configured_fields, $this->configuration[self::SETTING_CONTEXTUAL_FILTER]);
  }

  /**
   * {@inheritdoc}
   */
  public function getCacheMaxAge() {
    // The block cannot be cached, because it must always match the current
    // search results.
    return 0;
  }

  /**
   * Get Search Index.
   */
  protected function getIndex() {
    $id = $this->getDerivativeId();
    return $this->displayPluginManager->createInstance("views_{$this->display['display_plugin']}:{$id}")->getIndex();
  }

}
