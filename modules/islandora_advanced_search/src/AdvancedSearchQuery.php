<?php

namespace Drupal\islandora_advanced_search;

use Drupal\block\Entity\Block;
use Drupal\Core\EventSubscriber\MainContentViewSubscriber;
use Drupal\Core\Form\FormBuilderInterface;
use Drupal\Core\Url;
use Drupal\islandora_advanced_search\Form\SettingsForm;
use Drupal\islandora_advanced_search\Plugin\Block\AdvancedSearchBlock;
use Drupal\search_api\Query\QueryInterface as DrupalQueryInterface;
use Drupal\views\ViewExecutable;
use Solarium\Core\Query\QueryInterface as SolariumQueryInterface;
use Symfony\Component\HttpFoundation\Request;

/**
 * Alter current search query / view from using URL parameters.
 */
class AdvancedSearchQuery {

  use GetConfigTrait;

  // User can set this configuration for the module.
  const DEFAULT_QUERY_PARAM = 'a';
  const DEFAULT_RECURSE_PARAM = 'r';

  /**
   * The query parameter is how terms are passed to the query.
   *
   * @var string
   */
  protected $queryParameter;

  /**
   * The recurse parameter indicates the search should be recursive or not.
   *
   * @var string
   */
  protected $recurseParameter;

  /**
   * Constructs a FacetBlockAjaxController object.
   *
   * @param string $query_parameter
   *   The field to search against.
   * @param string $recurse_parameter
   *   The field that signifies the search should be recursive.
   */
  public function __construct(string $query_parameter = self::DEFAULT_QUERY_PARAM, string $recurse_parameter = self::DEFAULT_RECURSE_PARAM) {
    $this->queryParameter = $query_parameter;
    $this->recurseParameter = $recurse_parameter;
  }

  /**
   * Gets the query parameter to use that stores the search terms.
   *
   * @return string
   *   The query parameter to use that stores the search terms.
   */
  public static function getQueryParameter() {
    return self::getConfig(SettingsForm::SEARCH_QUERY_PARAMETER, self::DEFAULT_QUERY_PARAM);
  }

  /**
   * Gets the query parameter to use that stores the search terms.
   *
   * @return string
   *   The recurse parameter used to indicate that the search should be
   *   recursive.
   */
  public static function getRecurseParameter() {
    return self::getConfig(SettingsForm::SEARCH_RECURSIVE_PARAMETER, self::DEFAULT_RECURSE_PARAM);
  }

  /**
   * Extracts a list of AdvancedSearchQueryTerms from the given request.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The request to parse terms from.
   *
   * @return \Drupal\islandora_advanced_search\AdvancedSearchQueryTerm[]
   *   A list of search terms.
   */
  public function getTerms(Request $request) {
    $terms = [];
    if ($request->query->has($this->queryParameter)) {
      $query_params = $request->query->get($this->queryParameter);
      if (is_array($query_params)) {
        foreach ($query_params as $params) {
          $terms[] = AdvancedSearchQueryTerm::fromQueryParams($params);
        }
      }
    }
    return array_filter($terms);
  }

  /**
   * Checks if the query should recursively include sub-collections.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The request to parse.
   *
   * @return bool
   *   TRUE if the search should recurse FALSE otherwise.
   */
  public function shouldRecurse(Request $request) {
    if ($request->query->has($this->recurseParameter)) {
      $recurse_param = $request->query->get($this->recurseParameter);
      return filter_var($recurse_param, FILTER_VALIDATE_BOOLEAN);
    }
    return FALSE;
  }

  /**
   * Checks if the all of the given terms are negations or not.
   *
   * @param \Drupal\islandora_advanced_search\AdvancedSearchQueryTerm[] $terms
   *   The terms to search for.
   *
   * @return bool
   *   TRUE if all terms are to be excluded otherwise FALSE.
   */
  protected function negativeQuery(array $terms) {
    foreach ($terms as $term) {
      if ($term->getInclude()) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Alters the given query using search terms provided in the given request.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The request to parse terms from.
   * @param \Solarium\Core\Query\QueryInterface $solarium_query
   *   The solr query to modify.
   * @param \Drupal\search_api\Query\QueryInterface $search_api_query
   *   The search api query from which the solr query was build.
   */
  public function alterQuery(Request $request, SolariumQueryInterface &$solarium_query, DrupalQueryInterface $search_api_query) {
    // Only apply if a Advanced Search Query was made.
    $terms = $this->getTerms($request);
    if (!empty($terms)) {
      $index = $search_api_query->getIndex();
      /** @var \Drupal\search_api_solr\Plugin\search_api\backend\SearchApiSolrBackend $backend */
      $backend = $index->getServerInstance()->getBackend();
      $language_ids = $search_api_query->getLanguages();
      $field_mapping = $backend->getSolrFieldNamesKeyedByLanguage($language_ids, $index);
      $q[] = "{!boost b=boost_document}";
      // To support negative queries we must first bring in all documents.
      $q[] = $this->negativeQuery($terms) ? "*:*" : "";
      $term = array_shift($terms);
      $q[] = $term->toSolrQuery($field_mapping);
      foreach ($terms as $term) {
        $q[] = $term->getConjunction();
        $q[] = $term->toSolrQuery($field_mapping);
      }
      $q = implode(' ', $q);
      /** @var Solarium\QueryType\Select\Query\Query $solarium_query */
      $solarium_query->setQuery($q);
    }
  }

  /**
   * Alters the given view to be recursive if applicable.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The request to parse terms from.
   * @param \Drupal\views\ViewExecutable $view
   *   The view to modify.
   * @param string $display_id
   *   The view display to potentially alter.
   */
  public function alterView(Request $request, ViewExecutable $view, $display_id) {
    $views = Utilities::getAdvancedSearchViewDisplays();
    // Only specify contextual filters for views which the advanced search
    // blocks are derived from.
    $block_id = array_search([$view->id(), $display_id], $views);
    if ($block_id !== FALSE) {
      $block = Block::load($block_id);
      $settings = $block->get('settings');
      // Ignore the immediate children contextual filter in the query to allow
      // for recursive search.
      if (isset($settings[AdvancedSearchBlock::SETTING_CONTEXTUAL_FILTER])) {
        $display = $view->getDisplay();
        $display_arguments = $display->getOption('arguments');
        $immediate_children_contextual_filter = $settings[AdvancedSearchBlock::SETTING_CONTEXTUAL_FILTER];
        $index = array_search($immediate_children_contextual_filter, array_keys($display_arguments));
        if ($this->shouldRecurse($request)) {
          // Change the argument to the exception value which should cause the
          // contextual filter to be ignored.
          $view->args[$index] = $display_arguments[$immediate_children_contextual_filter]['exception']['value'];
        }
        else {
          // Explicitly set the default argument for AJAX requests.
          // We need to restore the default as that functionality is currently
          // broken. @see https://www.drupal.org/project/drupal/issues/3173778
          //
          // We fake the current request from the refer only to set the default
          // argument in case it is build from the URL. If this is not an AJAX
          // request this logic can be ignored.
          if ($request->isXmlHttpRequest()) {
            $view->initHandlers();
            $request_stack = \Drupal::requestStack();
            $refer = Request::create($request->server->get('HTTP_REFERER'));
            $refer->getPathInfo();
            $refer->attributes->add(\Drupal::getContainer()->get('router')->matchRequest($refer));
            $request_stack->push($refer);
            $plugin = $view->argument[$immediate_children_contextual_filter]->getPlugin('argument_default');
            if ($plugin) {
              $view->args[$index] = $plugin->getArgument();
            }
            $request_stack->pop();
          }
        }
      }
    }
  }

  /**
   * Get query parameter for all search terms.
   *
   * @return \Drupal\Core\Url
   *   Url for the given request combined with search query parameters.
   */
  public function toUrl(Request $request, array $terms, bool $recurse, $route = NULL) {
    $query_params = $request->query->all();
    if ($route) {
      $url = Url::fromRoute($route);
      // The form that built the url may use AJAX, but we are redirecting to a
      // new page, so it should be disabled.
      unset($query_params[FormBuilderInterface::AJAX_FORM_REQUEST]);
      unset($query_params[MainContentViewSubscriber::WRAPPER_FORMAT]);
    }
    else {
      $url = Url::createFromRequest($request);
    }
    unset($query_params[$this->queryParameter]);
    foreach ($terms as $term) {
      $query_params[$this->queryParameter][] = $term->toQueryParams();
    }
    if ($recurse) {
      $query_params[$this->recurseParameter] = '1';
    }
    else {
      unset($query_params[$this->recurseParameter]);
    }
    $url->setOptions(['query' => $query_params]);
    return $url;
  }

}
