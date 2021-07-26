<?php

namespace Drupal\islandora_advanced_search\Form;

use Drupal\Component\Utility\Html;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Routing\RouteMatchInterface;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\islandora_advanced_search\AdvancedSearchQuery;
use Drupal\islandora_advanced_search\AdvancedSearchQueryTerm;
use Drupal\islandora_advanced_search\GetConfigTrait;
use Drupal\views\DisplayPluginCollection;
use Drupal\views\Entity\View;
use Drupal\views\Plugin\views\display\PathPluginBase;
use Drupal\views\Views;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\Request;

/**
 * Form for building and Advanced Search Query.
 */
class AdvancedSearchForm extends FormBase {
  use GetConfigTrait;

  // Users can customize the operator to use font-awesome or some other icons.
  // Its a limitation in the use of `input type=submit` rather than buttons in
  // Drupal that we couldn't just rely on CSS.
  // This is exposed in the module settings.
  // @see https://www.drupal.org/project/drupal/issues/1671190
  const DEFAULT_ADD_OP = '+';
  const DEFAULT_REMOVE_OP = '-';

  const AND_OP = 'AND';
  const IS_OP = 'IS';
  const NOT_OP = 'NOT';
  const OR_OP = 'OR';

  // These are also hard-coded in islandora_advanced_search.form.js.
  const CONJUNCTION_FORM_FIELD = 'conjunction';
  const SEARCH_FORM_FIELD = 'search';
  const INCLUDE_FORM_FIELD = 'include';
  const VALUE_FORM_FIELD = 'value';

  const AJAX_WRAPPER = 'advanced-search-ajax';

  /**
   * The current request.
   *
   * @var \Symfony\Component\HttpFoundation\Request
   */
  protected $request;

  /**
   * The current route match.
   *
   * @var \Drupal\Core\Routing\RouteMatchInterface
   */
  protected $currentRouteMatch;

  /**
   * Class constructor.
   */
  public function __construct(Request $request, RouteMatchInterface $current_route_match) {
    $this->request = $request;
    $this->currentRouteMatch = $current_route_match;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('request_stack')->getMasterRequest(),
      $container->get('current_route_match')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_advanced_search_form';
  }

  /**
   * Get the character to use for adding a facet to the query.
   *
   * @return string
   *   The character to use for adding an facet to the query.
   */
  public static function getAddOperator() {
    return self::getConfig(SettingsForm::SEARCH_ADD_OPERATOR, self::DEFAULT_ADD_OP);
  }

  /**
   * Get the character to use for removing a facet from the query.
   *
   * @return string
   *   The character to use for removing an facet to the query.
   */
  public static function getRemoveOperator() {
    return self::getConfig(SettingsForm::SEARCH_REMOVE_OPERATOR, self::DEFAULT_REMOVE_OP);
  }

  /**
   * Convert the list of fields to select options.
   *
   * @param \Drupal\search_api\Item\FieldInterface[] $fields
   *   The fields to convert to select options.
   *
   * @return array
   *   Array of fields which can be searched where the key is the search field
   *   identifier and the value is its human readable label.
   */
  protected function fieldOptions(array $fields) {
    $options = [];
    foreach ($fields as $field) {
      $id = $field->getFieldIdentifier();
      $options[$id] = $field->getLabel();
    }
    return $options;
  }

  /**
   * Gets possible include options for the given conjunction.
   */
  protected function includeOptions(string $conjunction) {
    switch ($conjunction) {
      case self::AND_OP:
        return;

      case self::OR_OP:
        return [
          self::IS_OP => $this->t('is'),
        ];
    }
  }

  /**
   * Default values to for a term.
   */
  protected function defaultTermValues(array $options) {
    return [
      self::CONJUNCTION_FORM_FIELD => self::AND_OP,
      // First item in list is default.
      self::SEARCH_FORM_FIELD => key($options),
      self::INCLUDE_FORM_FIELD => self::IS_OP,
      self::VALUE_FORM_FIELD => NULL,
    ];
  }

  /**
   * Process input to the from either URL parameters or from the form input.
   */
  protected function processInput(FormStateInterface $form_state, array $term_default_values) {
    $input = $form_state->getUserInput();
    $recursive = isset($input['recursive']) ? $input['recursive'] : NULL;
    $term_values = isset($input['terms']) && is_array($input['terms']) ? $input['terms'] : [];
    // Form was not submitted see if we can rebuild from query parameters.
    $advanced_search_query = new AdvancedSearchQuery();
    if (empty($term_values)) {
      $terms = $advanced_search_query->getTerms($this->request);
      foreach ($terms as $term) {
        $term_values[] = $term->toUserInput();
      }
    }
    if (!isset($input['recursive'])) {
      $recursive = $advanced_search_query->shouldRecurse($this->request);
    }
    // Form was submitted via +/- operators.
    $trigger = $form_state->getTriggeringElement();
    if ($trigger != NULL) {
      $term_index = $trigger['#term_index'] ?? 0;
      $value = $trigger['#value'] instanceof TranslatableMarkup ?
        $trigger['#value']->getUntranslatedString() :
        $trigger['#value'];
      switch ($value) {
        case $this->getAddOperator():
          // Insert after the term listed.
          array_splice($term_values, $term_index + 1, 0, [$term_default_values]);
          break;

        case $this->getRemoveOperator():
          array_splice($term_values, $term_index, 1);
          break;

        case "Reset":
          $recursive = FALSE;
          $term_values = [];
          break;

        // Ignore unknown value for trigger.
      }
      // Place user input with updated values.
      $input['terms'] = $term_values;
      $input['recursive'] = $recursive;
      $form_state->setUserInput($input);
    }
    return [$recursive, $term_values];
  }

  /**
   * Gets the route name for the view display used to derive this forms block.
   *
   * @return string|null
   *   The route name for the view display that was used to create this
   *   forms block.
   */
  protected function getRouteName(FormStateInterface $form_state) {
    $view = $form_state->get('view');
    $display = $form_state->get('display');
    $display_handlers = new DisplayPluginCollection($view->getExecutable(), Views::pluginManager('display'));
    $display_handler = $display_handlers->get($display['id']);
    if ($display_handler instanceof PathPluginBase) {
      return $display_handler->getRouteName();
    }
    return NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, View $view = NULL, array $display = [], array $fields = [], string $context_filter = NULL) {
    // Keep reference to view and display as the submit handler may use them
    // to redirect the user to the search page.
    $form_state->set('view', $view);
    $form_state->set('display', $display);
    $route_name = $this->getRouteName($form_state);
    $requires_redirect = $route_name ? $this->currentRouteMatch->getRouteName() !== $route_name : FALSE;

    $form['#attached']['library'][] = 'islandora_advanced_search/advanced.search.form';
    $form['#attached']['drupalSettings']['islandora_advanced_search_form'] = [
      'id' => Html::getId($this->getFormId()),
      'redirect' => $requires_redirect,
      'query_parameter' => AdvancedSearchQuery::getQueryParameter(),
      'recurse_parameter' => AdvancedSearchQuery::getRecurseParameter(),
      'mapping' => [
        self::CONJUNCTION_FORM_FIELD => AdvancedSearchQueryTerm::CONJUNCTION_QUERY_PARAMETER,
        self::SEARCH_FORM_FIELD => AdvancedSearchQueryTerm::FIELD_QUERY_PARAMETER,
        self::INCLUDE_FORM_FIELD => AdvancedSearchQueryTerm::INCLUDE_QUERY_PARAMETER,
        self::VALUE_FORM_FIELD => AdvancedSearchQueryTerm::VALUE_QUERY_PARAMETER,
      ],
    ];

    $options = $this->fieldOptions($fields);
    $term_default_values = $this->defaultTermValues($options);
    list($recursive, $term_values) = $this->processInput($form_state, $term_default_values);
    $i = 0;
    $term_elements = [];
    $total_terms = count($term_values);
    $block_class_prefix = str_replace('_', '-', $this->getFormId());
    do {
      // Either specified by the user in the request or use the default.
      $first = $i == 0;
      $term_value = !empty($term_values) ? array_shift($term_values) : $term_default_values;
      $conjunction = isset($term_value[self::CONJUNCTION_FORM_FIELD]) ? $term_value[self::CONJUNCTION_FORM_FIELD] : $term_default_values[self::CONJUNCTION_FORM_FIELD];
      $term_elements[] = [
        // Only show on terms after the first.
        self::CONJUNCTION_FORM_FIELD => $first ? NULL : [
          '#type' => 'select',
          '#options' => [
            self::AND_OP => $this->t('and'),
            self::OR_OP => $this->t('or'),
          ],
          '#default_value' => $conjunction,
        ],
        self::SEARCH_FORM_FIELD => [
          '#type' => 'select',
          '#options' => $options,
          '#default_value' => $term_value[self::SEARCH_FORM_FIELD],
        ],
        self::INCLUDE_FORM_FIELD => [
          '#type' => 'select',
          '#options' => [
            self::IS_OP => $this->t('is'),
            self::NOT_OP => $this->t('is not'),
          ],
          '#default_value' => $term_value[self::INCLUDE_FORM_FIELD],
          // Show only when conjunction is 'AND' as 'OR NOT' is not supported
          // by solr and will be converted to 'AND NOT'.
          '#states' => [
            'visible' => [
              ':input[name="terms[' . $i . '][' . self::CONJUNCTION_FORM_FIELD . ']"]' => ['value' => self::AND_OP],
            ],
          ],
        ],
        // Just markup to show when 'include' is not alterable due to the
        // selected 'conjunction'. Hide for the first term.
        'is' => $first ? NULL : [
          '#type' => 'container',
          '#attributes' => ['style' => 'display:inline;'],
          '#states' => [
            'visible' => [
              ':input[name="terms[' . $i . '][' . self::CONJUNCTION_FORM_FIELD . ']"]' => ['value' => self::OR_OP],
            ],
          ],
          'content' => [
            '#markup' => $this->t('is'),
          ],
        ],
        self::VALUE_FORM_FIELD => [
          '#type' => 'textfield',
          '#default_value' => $term_value[self::VALUE_FORM_FIELD],
        ],
        'actions' => [
          '#type' => 'container',
          'add' => [
            '#type' => 'button',
            '#value' => $this->getAddOperator(),
            '#name' => 'add-term-' . $i,
            '#term_index' => $i,
            '#attributes' => [
              'class' => [$block_class_prefix . '__add', 'fa'],
            ],
            '#ajax' => [
              'callback' => [$this, 'ajaxCallback'],
              'wrapper' => self::AJAX_WRAPPER,
              'progress' => [
                'type' => 'none',
              ],
            ],
          ],
          'remove' => $total_terms <= 1 ? NULL : [
            '#type' => 'button',
            '#value' => $this->getRemoveOperator(),
            '#name' => 'remove-term-' . $i,
            '#term_index' => $i,
            '#attributes' => [
              'class' => [$block_class_prefix . '__remove', 'fa'],
            ],
            '#ajax' => [
              'callback' => [$this, 'ajaxCallback'],
              'wrapper' => self::AJAX_WRAPPER,
              'progress' => [
                'type' => 'none',
              ],
            ],
          ],
        ],
      ];
      $i++;
    } while (!empty($term_values));

    $form['ajax'] = [
      '#type' => 'container',
      '#attributes' => ['id' => self::AJAX_WRAPPER],
      'terms' => array_merge([
        '#tree' => TRUE,
        '#type' => 'container',
      ], $term_elements),
    ];

    if ($context_filter != NULL) {
      $form['ajax']['recursive'] = [
        '#type' => 'checkbox',
        '#title' => $this->t('Include Sub-Collections'),
        '#default_value' => $recursive,
      ];
    }
    $form['reset'] = [
      '#type' => 'button',
      '#value' => $this->t('Reset'),
      '#attributes' => [
        'class' => [$block_class_prefix . '__reset'],
      ],
      '#ajax' => [
        'callback' => [$this, 'ajaxCallback'],
        'wrapper' => self::AJAX_WRAPPER,
        'progress' => [
          'type' => 'none',
        ],
      ],
    ];
    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Search'),
      '#attributes' => [
        'class' => [$block_class_prefix . '__search'],
      ],
    ];
    return $form;
  }

  /**
   * Builds an Advanced Search Query Url from the submitted form values.
   */
  protected function buildUrl(FormStateInterface $form_state) {
    $terms = [];
    $values = $form_state->getValues();
    foreach ($values['terms'] as $term) {
      $terms[] = AdvancedSearchQueryTerm::fromUserInput($term);
    }
    $terms = array_filter($terms);
    $recurse = filter_var(isset($values['recursive']) ? $values['recursive'] : FALSE, FILTER_VALIDATE_BOOLEAN);
    $route = $this->getRouteName($form_state);
    $advanced_search_query = new AdvancedSearchQuery();
    return $advanced_search_query->toUrl($this->request, $terms, $recurse, $route);
  }

  /**
   * Callback for adding / removing terms from the search.
   */
  public function ajaxCallback(array &$form, FormStateInterface $form_state) {
    return $form['ajax'];
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $trigger = (string) $form_state->getTriggeringElement()['#value'];
    switch ($trigger) {
      case $this->t('Search'):
        $form_state->setRedirectUrl($this->buildUrl($form_state));
        break;

      default:
        $form_state->setRebuild();
    }
  }

}
