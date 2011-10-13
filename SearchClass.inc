<?php



/**
 * @file 
 * SearchClass Class
 */

/**
 * SearchClass ??
 */
class SearchClass {

  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;

  /**
   * solr_search ??
   * @param type $query
   * @param type $startPage
   * @param type $fq
   * @param type $dismax
   * @return type 
   */
  function solr_search($query, $startPage=1, $fq = NULL, $dismax = NULL) {
    $solrFile = trim(variable_get('islandora_solr_search_block_handler_file', 'plugins/SolrResults.inc'));

    // Don't let us bust out of fedora_repository modules directory when looking for a handler
    if (strpos($solrField, '../')) {  
      drupal_set_message(t('You have illegal characters in your solr handler function in the Islandora solr block config.'), 'error');
    }
    
    $solrClass = trim(variable_get('islandora_solr_search_block_handler_class', 'SolrResults'));
    $solrFunction = trim(variable_get('islandora_solr_search_block_handler_function', 'SearchAndDisplay'));
    require_once(drupal_get_path('module', 'fedora_repository') . '/' . $solrFile);
    try {
      $implementation = new $solrClass();
    } catch (Exception $e) {
      watchdog(t("Fedora_Repository"), "Error getting solr search results class: !message", array('!message' => $e->getMessage()), NULL, WATCHDOG_ERROR);
      return 'Error getting solr search results class.  Check watchdog for more info.';
    }
    return $implementation->$solrFunction($query, $startPage, $fq, $dismax);
  }

  /**
   * build solr search form ??
   * @param type $repeat
   * @param type $pathToSearchTerms
   * @param type $query
   * @return type 
   */
  function build_solr_search_form($repeat = NULL, $pathToSearchTerms = NULL, $query = NULL) {
    $types = $this->get_search_terms_array(NULL, 'solrSearchTerms.xml');
    $queryArray = NULL;
    if (isset($query)) {
      $queryArray = explode('AND', $query);
    }

    $andOrArray = array(
      'AND' => 'and',
      //'OR' => 'or' //removed or for now as it would be a pain to parse
    );
    $form = array();

    if (!isset($repeat)) {
      $repeat = variable_get('islandora_solr_search_block_repeat', t('3'));
    }
    $var0 = explode(':', $queryArray[0]);
    $var1 = explode(':', $queryArray[1]);
    $form['search_type']['type1'] = array(
      '#title' => t(''),
      '#type' => 'select',
      '#options' => $types,
      '#default_value' => trim($var0[0])
    );
    $form['fedora_terms1'] = array(
      '#size' => '24',
      '#type' => 'textfield',
      '#title' => t(''),
      '#required' => TRUE,
      '#default_value' => (count($var0) >= 2 ? trim($var0[1]) : ''),
    );
    $form['andor1'] = array(
      '#title' => t(''),
      '#type' => 'select',
      '#default_value' => 'AND',
      '#options' => $andOrArray
    );
    $form['search_type']['type2'] = array(
      '#title' => t(''),
      '#type' => 'select',
      '#options' => $types,
      '#default_value' => (count($var1) >= 2 ? trim($var1[0]) : ''),
    );
    $form['fedora_terms2'] = array(
      '#size' => '24',
      '#type' => 'textfield',
      '#title' => t(''),
      '#default_value' => (count($var1) >= 2 ? $var1[1] : ''),
    );
    if ($repeat > 2 && $repeat < 9) { //don't want less then 2 or more then 9
      for ($i = 3; $i < $repeat + 1; $i++) {
        $t = $i - 1;
        $field_and_term = explode(':', $queryArray[$t]);
        $form["andor$t"] = array(
          '#title' => t(''),
          '#type' => 'select',
          '#default_value' => 'AND',
          '#options' => $andOrArray
        );
        $form['search_type']["type$i"] = array(
          '#title' => t(''),
          '#type' => 'select',
          '#options' => $types,
          '#default_value' => trim($field_and_term[0])
        );
        $form["fedora_terms$i"] = array(
          '#size' => '24',
          '#type' => 'textfield',
          '#title' => t(''),
          '#default_value' => (count($field_and_term) >= 2 ? trim($field_and_term[1]) : ''),
        );
      }
    }

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('search')
    );
    return $form;
  }

  /**
   * build simple solr form ??
   * @return string 
   */
  function build_simple_solr_form() {
    //$form = array();
    $form["search_query"] = array(
      '#size' => '30',
      '#type' => 'textfield',
      '#title' => t(''),
      // '#default_value' => (count($field_and_term) >= 2 ? trim($field_and_term[1]) : ''),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('search')
    );
    return $form;
  }

  /**
   * theme solr search form ??
   * @param type $form
   * @return type 
   */
  function theme_solr_search_form($form) {
    if (!isset($repeat)) {
      $repeat = variable_get('islandora_solr_search_block_repeat', t('3'));
    }

    $output = drupal_render($form['search_type']['type1']);
    $output .= drupal_render($form['fedora_terms1']);
    $output .= drupal_render($form['andor1']) . drupal_render($form['search_type']['type2']);
    $output .= drupal_render($form['fedora_terms2']);
    if ($repeat > 2 && $repeat < 9) {
      for ($i = 3; $i < $repeat + 1; $i++) {
        $t = $i - 1;
        $output .= drupal_render($form["andor$t"]) . drupal_render($form['search_type']["type$i"]);
        $output .= drupal_render($form["fedora_terms$i"]);
      }
    }
    $output .= drupal_render($form['submit']);
    $output .= drupal_render($form);
    return $output;
  }

  /**
   * quick search ??
   * @param type $type
   * @param type $query
   * @param type $showForm
   * @param type $orderBy
   * @param type $userArray
   * @return type 
   */
  function quickSearch($type, $query, $showForm = 1, $orderBy = 0, & $userArray) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    if (user_access('view fedora collection')) {
      $numberOfHistPerPage = '5000'; //hack for IR they do not want next button
      $luceneQuery = NULL;
      // Demo search string ?operation=gfindObjects&indexName=DemoOnLucene&query=fgs.DS.first.text%3Achristmas&hitPageStart=11&hitPageSize=10
      $keywords = explode(' ', $query);

      foreach ($keywords as $keyword) {
        $luceneQuery .= $type . ':' . $keyword . '+AND+';
      }
      $luceneQuery = substr($luceneQuery, 0, strlen($luceneQuery) - 5);

      $indexName = variable_get('fedora_index_name', 'DemoOnLucene');
      $keys = htmlentities(urlencode($query));
      $searchUrl = variable_get('fedora_fgsearch_url', 'http://localhost:8080/fedoragsearch/rest');
      $searchString = '?operation=gfindObjects&indexName=' . $indexName . '&restXslt=copyXml&query=' . $luceneQuery;
      $searchString .= '&hitPageSize=' . $numberOfHistPerPage . '&hitPageStart=1';
      //$searchString = htmlentities($searchString);
      $searchUrl .= $searchString;

      // $objectHelper = new ObjectHelper();

      $resultData = do_curl($searchUrl, 1);
      if (isset($userArray)) {
        $doc = new DOMDocument();
        $doc->loadXML($resultData);
        $xPath = new DOMXPath($doc);
        // Add users to department list.  This is a hack as not all users will be in dupal
        $nodeList = $xPath->query('//field[@name="refworks.u1"]');
        foreach ($nodeList as $node) {
          if (!in_array($node->nodeValue, $userArray)) {
            $userArray[] = $node->nodeValue;
          }
        }
      }
      if ($showForm) {
        $output = '<Strong>Quick Search</strong><br /><table class="table-form"><tr>' . drupal_get_form('fedora_repository_quick_search_form') . '</tr></table>';
      }
      $output .= $this->applyXSLT($resultData, $orderBy);
      return $output;
    }
  }

  /**
   * gets term from a lucene index and displays them in a list
   * @param type $fieldName
   * @param type $startTerm
   * @param type $displayName
   * @return type 
   */
  function getTerms($fieldName, $startTerm, $displayName = NULL) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    $indexName = variable_get('fedora_index_name', 'DemoOnLucene');
    $searchUrl = variable_get('fedora_fgsearch_url', 'http://localhost:8080/fedoragsearch/rest');
    if ($startTerm == NULL) {
      $startTerm = "";
    }
    $startTerm = drupal_urlencode($startTerm);
    $query = 'operation=browseIndex&startTerm=' . $startTerm . '&fieldName=' . $fieldName . '&termPageSize=20&indexName=' . $indexName . '&restXslt=copyXml&resultPageXslt=copyXml';
    // $query=drupal_urlencode($query);
    $query = '?' . $query;
    $searchString = $searchUrl . $query;

    $objectHelper = new ObjectHelper();

    $resultData = do_curl($searchString, 1);
    $path = drupal_get_path('module', 'Fedora_Repository');

    $output .= $this->applySpecifiedXSLT($resultData, $path . '/xsl/browseIndexToResultPage.xslt', $displayName);
    //$output .= '<br />'.$alpha_out;
    return $output;
  }

  /**
   * custom search ??
   * @param type $query
   * @param type $startPage
   * @param type $xslt
   * @param type $numberOfHistPerPage
   * @return type 
   */
  function custom_search($query, $startPage=1, $xslt= '/xsl/advanced_search_results.xsl', $numberOfHistPerPage = 50) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

    if (user_access('view fedora collection')) {
      //$numberOfHistPerPage = '50';//hack for IR they do not want next button
      $luceneQuery = NULL;
      $indexName = variable_get('fedora_index_name', 'DemoOnLucene');
      $copyXMLFile = 'copyXml';
      // if($indexName=='ilives' || $indexName=='BasicIndex'){
      //   $copyXMLFile = 'copyXmliLives';
      // }
      $query = trim($query);
      $query = htmlentities(urlencode($query));
      $searchUrl = variable_get('fedora_fgsearch_url', 'http://localhost:8080/fedoragsearch/rest');
      $searchString = '?operation=gfindObjects&indexName=' . $indexName . '&restXslt=' . $copyXMLFile . '&query=' . $query;
      $searchString .= '&hitPageSize=' . $numberOfHistPerPage . '&hitPageStart=' . $startPage;
      //$searchString = htmlentities($searchString);
      $searchUrl .= $searchString;

      //$objectHelper = new ObjectHelper();

      $resultData = do_curl($searchUrl, 1);
      //var_dump($resultData);exit(0);
      //	$doc = new DOMDocument();
      //	$doc->loadXML($resultData);

      $output .= $this->applyLuceneXSLT($resultData, $startPage, $xslt, $query);
      return $output;
    }
  }

  /**
   * apply specified xslt ??
   * @global type $user
   * @param type $resultData
   * @param type $pathToXSLT
   * @param type $displayName
   * @return type 
   */
  function applySpecifiedXSLT($resultData, $pathToXSLT, $displayName = NULL) {
    $proc = NULL;
    global $user;
    if (!$resultData) {
      drupal_set_message(t('No data found!'));
      return ' '; //no results
    }
    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('Error loading ' . $pathToXSLT . ' xslt!') . $e->getMessage());
      return ' ';
    }

    //$proc->setParameter('', 'searchUrl', url('search') . '/fedora_repository'); //needed in our xsl
    $proc->setParameter('', 'objectsPage', base_path());
    $proc->setParameter('', 'userID', $user->uid);
    if (isset($displayName)) {
      $proc->setParameter('', 'displayName', $displayName);
    }
    else {
      $proc->setParameter('', 'displayName', "test");
    }

    $xsl = new DomDocument();

    $test = $xsl->load($pathToXSLT);

    if (!isset($test)) {
      drupal_set_message(t('Error loading ' . $pathToXSLT . ' xslt!'));
      return t('Error loading !pathToXSLT xslt.', array('!pathToXSLT' => $pathToXSLT));
    }

    $input = new DomDocument();

    $didLoadOk = $input->loadXML($resultData);

    if (!isset($didLoadOk)) {
      drupal_set_message(t('Error loading XML data!'));
      return t('Error loading XML data.');
    }
    else {
      $proc->importStylesheet($xsl);
      $newdom = $proc->transformToDoc($input);
      return $newdom->saveXML();
    }
  }

  //default function for lucene results

  /**
   * apply an xslt to lucene gsearch search results
   *
   * @param <type> $resultData
   * @param <type> $startPage
   * @param <type> $xslt_file
   * @param <type> $query the query that was executed.  May want to pass this on.
   */
  function applyLuceneXSLT($resultData, $startPage = 1, $xslt_file = '/xsl/results.xsl', $query=NULL) {
    $path = drupal_get_path('module', 'Fedora_Repository');
    $test = $xslt_file;
    $isRestricted = variable_get('fedora_namespace_restriction_enforced', TRUE);
    if (!isRestricted && $xslt_file == NULL) {
      $xslt_file = '/xsl/unfilteredresults.xsl';
    }
    $proc = NULL;
    if (!$resultData) {
      //drupal_set_message(t('No Results!'));
      return ' '; //no results
    }
    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('Error loading results xslt!') . $e->getMessage());
      return ' ';
    }
    if (isset($query)) {
      $proc->setParameter('', 'fullQuery', $query);
    }
    //inject into xsl stylesheet
    global $user;
    $proc->setParameter('', 'userID', $user->uid);
    $proc->setParameter('', 'searchToken', drupal_get_token('fedora_repository_advanced_search')); //token generated by Drupal, keeps tack of what tab etc we are on
    $proc->setParameter('', 'searchUrl', url('search') . '/fedora_repository'); //needed in our xsl
    $proc->setParameter('', 'objectsPage', base_path());
    $proc->setParameter('', 'allowedPidNameSpaces', variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '));
    $proc->setParameter('', 'hitPageStart', $startPage);
    $proc->registerPHPFunctions();
    $xsl = new DomDocument();

    $test = $xsl->load($path . $xslt_file);
    if (!isset($test)) {
      drupal_set_message(t('Error loading search results xslt!'));
      return t('Error loading search results XSLT.');
    }

    $input = new DomDocument();
    $didLoadOk = $input->loadXML($resultData);

    if (!isset($didLoadOk)) {
      drupal_set_message(t('Error loading search results!'));
      return t('Error loading search results.');
    }
    else {
      $proc->importStylesheet($xsl);
      $newdom = $proc->transformToDoc($input);
      return $newdom->saveXML();
    }
  }

  /**
   * xslt for islandscholar these xslt functions can probably be pulled into one
   * @param type $resultData
   * @param type $orderBy
   * @return type 
   */
  function applyXSLT($resultData, $orderBy = 0) {
    $path = drupal_get_path('module', 'Fedora_Repository');
    $proc = NULL;
    if (!$resultData) {
      //drupal_set_message(t('No Results!'));
      return ' '; //no results
    }
    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('Error loading results xslt!') . " " . $e->getMessage());
      return ' ';
    }

    //inject into xsl stylesheet
    //$proc->setParameter('', 'searchToken', drupal_get_token('search_form')); //token generated by Drupal, keeps tack of what tab etc we are on
    $proc->setParameter('', 'userID', $user->uid);
    $proc->setParameter('', 'searchUrl', url('search') . '/fedora_repository'); //needed in our xsl
    $proc->setParameter('', 'objectsPage', base_path());
    $proc->setParameter('', 'allowedPidNameSpaces', variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: ilives: islandora-book: books: newspapers: '));
    $proc->setParameter('', 'orderBy', $orderBy);
    $xsl = new DomDocument();

    $test = $xsl->load($path . '/ir/xsl/results.xsl');
    if (!isset($test)) {
      drupal_set_message(t('Error loading search results xslt!'));
      return t('Error loading search results XSLT.');
    }

    $input = new DomDocument();
    $didLoadOk = $input->loadXML($resultData);

    if (!isset($didLoadOk)) {
      drupal_set_message(t('Error loading search results!'));
      return t('Error loading search results.');
    }
    else {
      $xsl = $proc->importStylesheet($xsl);
      $newdom = $proc->transformToDoc($input);
      return $newdom->saveXML();
    }
  }

  /**
   * theme advanced search form ??
   * @param type $form
   * @param type $repeat
   * @return type 
   */
  function theme_advanced_search_form($form, $repeat=NULL) {
    if (!isset($repeat)) {
      $repeat = variable_get('fedora_repository_advanced_block_repeat', t('3'));
    }

    $output = drupal_render($form['search_type']['type1']);
    $output .= drupal_render($form['fedora_terms1']);
    $output .= drupal_render($form['andor1']) . drupal_render($form['search_type']['type2']);
    $output .= drupal_render($form['fedora_terms2']);
    if ($repeat > 2 && $repeat < 9) {
      for ($i = 3; $i < $repeat + 1; $i++) {
        $t = $i - 1;
        $output .= drupal_render($form["andor$t"]) . drupal_render($form['search_type']["type$i"]);
        $output .= drupal_render($form["fedora_terms$i"]);
      }
    }
    $output .= drupal_render($form['submit']);
    $output .= drupal_render($form);
    return $output;
  }

  /**
   * build search form, custom blocks should set the number of repeats or it will use the default
   * @param type $repeat
   * @param type $pathToSearchTerms
   * @param type $query
   * @return string 
   */
  function build_advanced_search_form($repeat = NULL, $pathToSearchTerms = NULL, $query = NULL) {
    $types = $this->get_search_terms_array($pathToSearchTerms);
    $queryArray = NULL;
    if (isset($query)) {
      $queryArray = explode('AND', $query);
    }

    $andOrArray = array(
      'AND' => 'and',
      //'OR' => 'or' //removed or for now as it would be a pain to parse
    );
    $form = array();

    if (!isset($repeat)) {
      $repeat = variable_get('fedora_repository_advanced_block_repeat', t('3'));
    }
    $var0 = explode(':', $queryArray[0]);
    $var1 = explode(':', $queryArray[1]);
    $form['search_type']['type1'] = array(
      '#title' => t(''),
      '#type' => 'select',
      '#options' => $types,
      '#default_value' => trim($var0[0])
    );
    $form['fedora_terms1'] = array(
      '#size' => '24',
      '#type' => 'textfield',
      '#title' => t(''),
      '#required' => TRUE,
      '#default_value' => (count($var0) >= 2 ? trim($var0[1]) : ''),
    );
    $form['andor1'] = array(
      '#title' => t(''),
      '#type' => 'select',
      '#default_value' => 'AND',
      '#options' => $andOrArray
    );
    $form['search_type']['type2'] = array(
      '#title' => t(''),
      '#type' => 'select',
      '#options' => $types,
      '#default_value' => (count($var1) >= 2 ? trim($var1[0]) : ''),
    );
    $form['fedora_terms2'] = array(
      '#size' => '24',
      '#type' => 'textfield',
      '#title' => t(''),
      '#default_value' => (count($var1) >= 2 ? $var1[1] : ''),
    );
    if ($repeat > 2 && $repeat < 9) { //don't want less then 2 or more then 9
      for ($i = 3; $i < $repeat + 1; $i++) {
        $t = $i - 1;
        $field_and_term = explode(':', $queryArray[$t]);
        $form["andor$t"] = array(
          '#title' => t(''),
          '#type' => 'select',
          '#default_value' => 'AND',
          '#options' => $andOrArray
        );
        $form['search_type']["type$i"] = array(
          '#title' => t(''),
          '#type' => 'select',
          '#options' => $types,
          '#default_value' => trim($field_and_term[0])
        );
        $form["fedora_terms$i"] = array(
          '#size' => '24',
          '#type' => 'textfield',
          '#title' => t(''),
          '#default_value' => (count($field_and_term) >= 2 ? trim($field_and_term[1]) : ''),
        );
      }
    }

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('search')
    );
    return $form;
  }

  /**
   * get search terms array 
   * @param type $path
   * @param string $file
   * @return type 
   */
  function get_search_terms_array($path = NULL, $file = NULL) {
    if (!isset($path)) {
      $path = drupal_get_path('module', 'Fedora_Repository');
    }
    $xmlDoc = new DomDocument();
    if (!isset($file)) {
      $file = 'searchTerms.xml';
    }
    $xmlDoc->load($path . '/' . $file);
    $nodeList = $xmlDoc->getElementsByTagName('term');
    $types = array();
    for ($i = 0; $i < $nodeList->length; $i++) {
      $field = $nodeList->item($i)->getElementsByTagName('field');
      $value = $nodeList->item($i)->getElementsByTagName('value');
      $fieldValue = $field->item(0)->nodeValue;
      $valueValue = $value->item(0)->nodeValue;
      $types["$fieldValue"] = "$valueValue";
    }
    return $types;
  }

}
