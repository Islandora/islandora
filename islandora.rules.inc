<?php

/**
 * @file
 * Does rule type stuff,.
 */

/**
 * Implements hook_rules_event_info().
 */
function islandora_rules_event_info() {
  return array(
    'islandora_object_ingested' => array(
      'group' => t('Islandora'),
      'label' => t('Object ingested'),
      'variables' => array(
        'object' => array(
          'type' => 'islandora_object',
          'label' => t('The ingested object'),
          'description' => t('A Tuque object for the ingested Fedora object, as an entity.'),
        ),
      ),
    ),
    'islandora_datastream_ingested' => array(
      'group' => t('Islandora'),
      'label' => t('Datastream ingested'),
      'variables' => array(
        'object' => array(
          'type' => 'islandora_object',
          'label' => t('The ingested object'),
          'description' => t('A Tuque object for the Fedora object on which the datastream exists, as an entity.'),
        ),
        'datastream' => array(
          'type' => 'islandora_datastream',
          'label' => t('Datastream ID'),
          'description' => t('The ID of the ingested datastream.'),
        ),
      ),
    ),
    'islandora_object_modified' => array(
      'group' => t('Islandora'),
      'label' => t('Object modified'),
      'variables' => array(
        'object' => array(
          'type' => 'islandora_object',
          'label' => t('The modified object'),
          'description' => t('A Tuque object for the modified Fedora object, as an entity.'),
        ),
      ),
    ),
    'islandora_datastream_modified' => array(
      'group' => t('Islandora'),
      'label' => t('Datastream modified'),
      'variables' => array(
        'object' => array(
          'type' => 'islandora_object',
          'label' => t('The modified object'),
          'description' => t('A Tuque object for the Fedora object on which the datastream exists, as an entity.'),
        ),
        'datastream' => array(
          'type' => 'islandora_datastream',
          'label' => t('Datastream'),
          'description' => t('The modified datastream.'),
        ),
      ),
    ),
    'islandora_object_purged' => array(
      'group' => t('Islandora'),
      'label' => t('Object purged'),
      'variables' => array(
        'object' => array(
          'type' => 'text',
          'label' => t('Object ID'),
          'description' => t('The ID of the purged object.'),
        ),
      ),
    ),
    'islandora_datastream_purged' => array(
      'group' => t('Islandora'),
      'label' => t('Datastream purged'),
      'variables' => array(
        'object' => array(
          'type' => 'islandora_object',
          'label' => t('Object'),
          'description' => t('A Tuque object for the Fedora object on which the datastream existed, as an entity.'),
        ),
        'datastream' => array(
          'type' => 'text',
          'label' => t('Datastream ID'),
          'description' => t('The identifier of the purged datastream.'),
        ),
      ),
    ),
  );
}

/**
 * Helper function to get reused "parameter" array.
 */
function islandora_rules_relationship_parameter_array() {
  return array(
    'subject' => array(
      'type' => 'islandora_object',
      'label' => t('Subject'),
      'description' => t('An object of which we should check the relationships (The "subject" of the relationship).'),
    ),
    'pred_uri' => array(
      'type' => 'text',
      'label' => t('Predicate URI'),
      'description' => t('The URI namespace to which the predicate belongs.'),
    ),
    'pred' => array(
      'type' => 'text',
      'label' => t('Predicate'),
      'description' => t('The predicate of the relationship.'),
    ),
    'object' => array(
      'type' => 'text',
      'label' => t('Object'),
      'description' => t('The object of the relationship.'),
      'allow null' => TRUE,
      'default value' => NULL,
    ),
    'type' => array(
      'type' => 'integer',
      'label' => t('Object type in the relationship'),
      'description' => t('0=URI, 1=plain literal'),
      'default value' => 0,
    ),
  );
}

/**
 * Helper function; get default parameters for the XPath condition and action.
 */
function islandora_rules_base_xpath_parameters() {
  return array(
    'object' => array(
      'type' => 'islandora_object',
      'label' => t('Object'),
      'description' => t('The object containing the datastream to check.'),
    ),
    'datastream_id' => array(
      'type' => 'text',
      'label' => t('Datastream'),
      'description' => t('The identifier of the XML datastream to check.'),
    ),
    'xpath' => array(
      'type' => 'text',
      'label' => t('XPath'),
      'description' => t('An XPath to evaluate.'),
    ),
    'xpath_namespaces' => array(
      'type' => 'taxonomy_vocabulary',
      'label' => t('XPath Namespace Taxonomy'),
      'description' => t('A flat taxonomy of which the terms are namespace prefixes and the description contains the URI for the namespace.'),
    ),
  );
}

/**
 * Implements hook_rules_condition_info().
 */
function islandora_rules_condition_info() {
  $cond = array();

  $cond['islandora_object_has_relationship'] = array(
    'label' => t('Check object for a relationship'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_relationship_parameter_array(),
  );
  $cond['islandora_object_has_datastream'] = array(
    'label' => t('Check object for existence of a datastream'),
    'group' => t('Islandora'),
    'parameter' => array(
      'object' => array(
        'type' => 'islandora_object',
        'label' => t('Object'),
        'description' => t('The object containing the datastream to check.'),
      ),
      'datastream_id' => array(
        'type' => 'text',
        'label' => t('Datastream'),
        'description' => t('The identifier of the datastream to check.'),
      ),
    ),
  );
  $cond['islandora_rules_datastream_has_xpath'] = array(
    'label' => t('Check for an XPath match in an XML datastream'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_base_xpath_parameters(),
  );

  return $cond;
}

/**
 * Implements hook_rules_action_info().
 */
function islandora_rules_action_info() {
  $cond = array();

  $cond['islandora_object_remove_relationship'] = array(
    'label' => t('Remove a relationship from an object'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_relationship_parameter_array(),
  );

  $cond['islandora_object_add_relationship'] = array(
    'label' => t('Add a relationship to an object'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_relationship_parameter_array(),
  );

  $cond['islandora_rules_datastream_load'] = array(
    'label' => t('Load a datastream from an object.'),
    'group' => t('Islandora'),
    'parameter' => array(
      'object' => array(
        'type' => 'islandora_object',
        'label' => t('Object'),
        'description' => t('A Tuque object for the Fedora object from which to load the datastream, as an entity.'),
      ),
      'datastream_id' => array(
        'type' => 'text',
        'label' => t('Datastream ID'),
        'description' => t('A string containing the identity of the datastream to load from the object.'),
      ),
    ),
    'provides' => array(
      'datastream' => array(
        'type' => 'islandora_datastream',
        'label' => t('Loaded datastream instance'),
      ),
    ),
  );
  $cond['islandora_rules_datastream_load_domxpath'] = array(
    'label' => t('Load a DOMXPath for a given XML.'),
    'group' => t('Islandora DOMXPath'),
    'parameter' => array(
      'datastream' => array(
        'type' => 'text',
        'label' => t('XML'),
        'description' => t('A string containing the XML to load.'),
      ),
    ),
    'provides' => array(
      'islandora_domxpath' => array(
        'type' => 'islandora_domxpath',
        'label' => t('Loaded DOMXPath instance'),
      ),
    ),
  );
  $cond['islandora_rules_datastream_load_xpath'] = array(
    'label' => t('Load a DOMXPath from a datastream.'),
    'group' => t('Islandora DOMXPath'),
    'parameter' => array(
      'datastream' => array(
        'type' => 'islandora_datastream',
        'label' => t('Datastream'),
        'description' => t('A datastream containing the XML to load.'),
      ),
    ),
    'provides' => array(
      'islandora_domxpath' => array(
        'type' => 'islandora_domxpath',
        'label' => t('Loaded DOMXPath instance'),
      ),
    ),
  );
  $cond['islandora_rules_datastream_load_namespace_vocab'] = array(
    'label' => t('Register namespaces on a DOMXPath instance.'),
    'group' => t('Islandora DOMXPath'),
    'parameter' => array(
      'value' => array(
        'type' => 'islandora_domxpath',
        'label' => t('DOMXPath instance'),
        'description' => t('The DOMXPath instance on which to register the namespaces.'),
      ),
      'xpath_namespaces' => array(
        'type' => 'taxonomy_vocabulary',
        'label' => t('XPath Namespace Taxonomy'),
        'description' => t('A flat taxonomy of which the terms are namespace prefixes and the description contains the URI for the namespace.'),
      ),
    ),
  );
  $cond['islandora_rules_datastream_query_xpath'] = array(
    'label' => t('Query nodes from DOMXPath instance.'),
    'group' => t('Islandora DOMXPath'),
    'parameter' => array(
      'xpath' => array(
        'type' => 'islandora_domxpath',
        'label' => t('DOMXPath instance'),
        'description' => t('The DOMXPath instance on which to perform the query.'),
      ),
      'query' => array(
        'type' => 'text',
        'label' => t('XPath query'),
        'description' => t('The XPath query to perform.'),
      ),
      'context_node' => array(
        'type' => 'islandora_domnode',
        'label' => t('Context Node'),
        'description' => t('If provided, the query will be performed relative to the provided node.'),
        'optional' => TRUE,
        'default value' => NULL,
        'allow null' => TRUE,
      ),
    ),
    'provides' => array(
      'nodes' => array(
        'type' => 'list<islandora_domnode>',
        'label' => t('Queried DOMNode elements'),
      ),
    ),
  );

  $cond['islandora_rules_datastream_set_xpath'] = array(
    'label' => t('Set value in elements matched by an XPath in an XML datastream'),
    'group' => t('Islandora'),
    'parameter' => islandora_rules_base_xpath_parameters() + array(
      'value' => array(
        'type' => 'text',
        'label' => t('Value'),
        'description' => t('The value to set in the XML on elements matched by the XPath.'),
      ),
    ),
  );

  return $cond;
}

/**
 * Rules action callback; grab a datastream from an object.
 */
function islandora_rules_datastream_load(AbstractObject $object, $datastream_id) {
  return array('datastream' => $object[$datastream_id]);
}

/**
 * Checks that there is a relationship match on the given object.
 *
 * Takes a subject (either a AbstractObject or a FedoraDatastream), as well as
 * the parameters for FedoraRelsExt::get() or FedoraRelsInt::get(), to try to
 * find a match.
 *
 * @see FedoraRelsExt::get()
 */
function islandora_object_has_relationship($sub, $pred_uri, $pred, $object, $type) {
  $relationships = $sub->relationships->get($pred_uri, $pred, $object, $type);
  return !empty($relationships);
}

/**
 * Remove a relationship from the given object.
 *
 * Takes a subject (either a AbstractObject or a FedoraDatastream), as well as
 * the parameters for FedoraRelsExt::remove() or FedoraRelsInt::remove(), to
 * try to find a match.
 *
 * @see FedoraRelsExt::get()
 */
function islandora_object_remove_relationship($sub, $pred_uri, $pred, $object, $type) {
  $sub->relationships->remove($pred_uri, $pred, $object, $type);
}

/**
 * Add a relationship to the given object.
 *
 * Takes a subject (either a AbstractObject or a FedoraDatastream), as well as
 * the parameters for FedoraRelsExt::add() or FedoraRelsInt::add(), and adds
 * the represented relationship.
 *
 * @see FedoraRelsExt::get()
 */
function islandora_object_add_relationship($sub, $pred_uri, $pred, $object, $type) {
  $sub->relationships->add($pred_uri, $pred, $object, $type);
}

/**
 * Rules Action callback; instantiate a DOMXPath with some XML.
 */
function islandora_rules_datastream_load_domxpath($string) {
  $doc = new DOMDocument();
  $doc->loadXML($string);
  $xpath = new DOMXPath($doc);
  return array('islandora_domxpath' => $xpath);
}

/**
 * Rules Action callback; load namespaces onto a DOMXPath instance.
 *
 * @param DOMXPath $xpath
 *   A DOMXPath instance.
 * @param object $xpath_vocab
 *   A loaded Drupal taxonomy vocabulary object, in which terms are understood
 *   to be namespace prefixes and descriptions are the namespace URIs.
 */
function islandora_rules_datastream_load_namespace_vocab(DOMXPath $xpath, $xpath_vocab) {
  foreach (taxonomy_get_tree($xpath_vocab->vid, 0, 1, FALSE) as $term) {
    $xpath->registerNamespace($term->name, $term->description);
  }
}

/**
 * Rules XPath helper; grab the datastream content and build a DOMXPath.
 */
function islandora_rules_datastream_load_xpath(AbstractDatastream $datastream, $xpath_vocab = NULL) {
  $result = islandora_rules_datastream_load_domxpath($datastream->content);
  if (is_object($xpath_vocab)) {
    islandora_rules_datastream_load_namespace_vocab($result['islandora_domxpath'], $xpath_vocab);
  }
  return $result;
}

/**
 * Rules Condition callback; test that an XPath returns a non-empty result set.
 */
function islandora_rules_datastream_has_xpath(AbstractObject $object, $datastream_id, $search_xpath, $xpath_vocab) {
  $datastream = $object[$datastream_id];
  $xpath = islandora_rules_datastream_load_xpath($datastream, $xpath_vocab);
  $result = $xpath['islandora_domxpath']->query($search_xpath);
  return $result->length > 0;
}

/**
 * Rules Action callback; set the value of all matched nodes.
 */
function islandora_rules_datastream_set_xpath(AbstractObject $object, $datastream_id, $search_xpath, $xpath_vocab, $value) {
  $datastream = $object[$datastream_id];
  $xpath = islandora_rules_datastream_load_xpath($datastream, $xpath_vocab);
  $result = $xpath['islandora_domxpath']->query($search_xpath);
  foreach ($result as $node) {
    $node->nodeValue = $value;
  }
  $datastream->content = $xpath['islandora_domxpath']->document->saveXML();
}

/**
 * Implements hook_rules_data_info().
 */
function islandora_rules_data_info() {
  return array(
    'islandora_domxpath' => array(
      'label' => t('DOMXPath instance'),
      'group' => t('Islandora'),
      'property info' => array(
        'content' => array(
          'type' => 'text',
          'label' => t('XML Content'),
          'computed' => TRUE,
          'getter callback' => 'islandora_rules_get_domxpath_document_content',
          'token type' => 'string',
        ),
      ),
      'wrap' => TRUE,
    ),
    'islandora_domnode' => array(
      'label' => t('DOMNode instance'),
      'group' => t('Islandora'),
      'property info' => array(
        'node_value' => array(
          'type' => 'text',
          'label' => t('Node value'),
          'computed' => TRUE,
          'getter callback' => 'islandora_rules_property_get',
          'setter callback' => 'islandora_rules_property_set',
          'property' => 'nodeValue',
          'token type' => 'string',
        ),
        'text_content' => array(
          'type' => 'text',
          'label' => t('Text content'),
          'computed' => TRUE,
          'getter callback' => 'islandora_rules_property_get',
          'property' => 'textContent',
          'token type' => 'string',
        ),
      ),
      'wrap' => TRUE,
    ),
    'islandora_domelement' => array(
      'label' => t('DOMElement instance'),
      'group' => t('Islandora'),
      'parent' => 'islandora_domnode',
      'wrap' => TRUE,
    ),
  );
}

/**
 * Property setter helper; set a property on an object.
 *
 * In Rules, properties can contain lowercase and numeric characters. Since
 * we want to refer to "nodeValue", we have to step around the Rules constraint.
 *
 * @param object $data
 *   The object on which to set the property, described by $info['property'].
 * @param array $options
 *   An array of options... Not sure how it's used? Not touched by us, in any
 *   case. :P.
 * @param string $name
 *   The name of the property to set, as used by Rules.
 * @param string $type
 *   The type of object on which the property is being set, as used by Rules.
 * @param array $info
 *   An associative array describing this property. In addition to that
 *   required by Rules/the Entity API, should contain:
 *   - property: A string indicate the actual property on the $data we wish to
 *     set.
 */
function islandora_rules_property_get($data, array $options, $name, $type, array $info) {
  return $data->$info['property'];
}

/**
 * Property setter helper; set a property on an object.
 *
 * In Rules, properties can contain lowercase and numeric characters. Since
 * we want to refer to "nodeValue", we have to step around the Rules constraint.
 *
 * @param object $data
 *   The object on which to set the property, described by $info['property'].
 * @param string $name
 *   The name of the property to set, as used by Rules.
 * @param mixed $value
 *   The value to set on the property.
 * @param string $langcode
 *   A string indicating the language being set, or NULL.
 * @param string $type
 *   The type of object on which the property is being set, as used by Rules.
 * @param array $info
 *   An associative array describing this property. In addition to that
 *   required by Rules/the Entity API, should contain:
 *   - property: A string indicate the actual property on the $data we wish to
 *     set.
 */
function islandora_rules_property_set(&$data, $name, $value, $langcode, $type, array $info) {
  $data->$info['property'] = $value;
}

/**
 * Rules property "get" callback; get XML contained in a DOMXPath instance.
 *
 * @param DOMXPath $xpath
 *   A DOMXPath instance.
 *
 * @return string
 *   The XML contained inside of the DOMXPath instance.
 */
function islandora_rules_get_domxpath_document_content(DOMXPath $xpath) {
  return $xpath->document->saveXML();
}

/**
 * Rules action callback; perform a query on a DOMXPath instance.
 *
 * @param DOMXPath $xpath
 *   A DOMXPath instance.
 * @param string $query
 *   An XPath query.
 * @param DOMNode $context_node
 *   An optional DOMNode. If provided, the query will be performed relative to
 *   the given node.
 *
 * @return array
 *   An array containing:
 *   - nodes: An array containing the results of the query.
 */
function islandora_rules_datastream_query_xpath(DOMXPath $xpath, $query, DOMNode $context_node = NULL) {
  return array('nodes' => iterator_to_array($xpath->query($query, $context_node)));
}

/**
 * Rules condition callback; check for the datastream on an object.
 */
function islandora_object_has_datastream(AbstractObject $object, $datastream_id) {
  return isset($object[$datastream_id]);
}
