<?php

/**
 * @file
 *
 * Implements a simple class for working with Dublin Core data and exporting it
 * back to XML. Inspiration and design shamelessly stolen from the pyfedora
 * project at http://pypi.python.org/pypi/pyfedora/0.1.0
 */

/**
 * Dublin Core Class
 */
class DublinCore {

  public $dc = array(
    'dc:title' => array(),
    'dc:creator' => array(),
    'dc:subject' => array(),
    'dc:description' => array(),
    'dc:publisher' => array(),
    'dc:contributor' => array(),
    'dc:date' => array(),
    'dc:type' => array(),
    'dc:format' => array(),
    'dc:identifier' => array(),
    'dc:source' => array(),
    'dc:language' => array(),
    'dc:relation' => array(),
    'dc:coverage' => array(),
    'dc:rights' => array(),
  );
  public $owner;

  /**
   * Constructs a DublinCore object from a Fedora_Item object and populates the
   * $dc array.
   *
   * @param string $dc_xml
   */
  function DublinCore($dc_xml = NULL) {
    if (!empty($dc_string)) {
      $this->dc = self::import_from_xml_string($dc_xml);
    }
  }

  /**
   * Add Elements
   * @param <type> $element_name
   * @param <type> $value
   */
  function add_element($element_name, $value) {
    if (is_string($value) && is_array($this->dc[$element_name])) {
      $this->dc[$element_name][] = $value;
    }
  }

  /**
   * Replace the given DC element with the values in $values
   * @param string $elemnt_name
   * @param array $values
   */
  function set_element($element_name, $values) {
    if (is_array($values)) {
      $this->dc[$element_name] = $values;
    }
    elseif (is_string($values)) {
      $this->dc[$element_name] = array($values);
    }
  }

  /**
   * Serialize this object to XML and return it.
   * @param type $with_preamble
   * @return type
   */
  function as_xml($with_preamble = FALSE) {
    $dc_xml = new DomDocument();
    $oai_dc = $dc_xml->createElementNS('http://www.openarchives.org/OAI/2.0/oai_dc/', 'oai_dc:dc');
    $oai_dc->setAttribute('xmlns:dc', 'http://purl.org/dc/elements/1.1/');
    foreach ($this->dc as $dc_element => $values) {
      if (is_array($values) && !empty($values)) {
        foreach ($values as $value) {
          $new_item = $dc_xml->createElement($dc_element, $value);
          $oai_dc->appendChild($new_item);
        }
      }
      else {
        $new_item = $dc_xml->createElement($dc_element);
        $oai_dc->appendChild($new_item);
      }
    }
    $dc_xml->appendChild($oai_dc);
    return $dc_xml->saveXML();
  }

  /**
   * Create dc from dict ( does nothing )
   */
  static function create_dc_from_dict() {

  }

  function as_formatted_array() {
    $dc_array = array();
    foreach ($this as $element) {
      if (!empty($element)) {
        foreach ($element as $field => $values) {
          // split value if the result value is an array
          if (is_array($values)) {
            $value = '';
            $i = 0;
            foreach ($values as $piece) {
              if (!empty($piece)) {
                if ($i++) {
                  $value .= ", ";
                }
                $value .= $piece;
              }
            }
          }
          else {
            $value = $values;
          }
          $dc_label = explode(':', $field);
          $element_label = drupal_ucfirst($dc_label[1]);
          $dc_array[$field]['label'] = $element_label;
          $dc_array[$field]['value'] = $value;
          $dc_array[$field]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));
        }
      }
    }
    return $dc_array;
  }


  /**
   * Creates a new instance of the class by parsing dc_xml
   *
   * @param string $dc_xml
   * @return DublinCore
   */
  static function import_from_xml_string($dc_xml) {
    $dc_doc = new DomDocument();
    if ($dc_doc->loadXML($dc_xml)) {
      $oai_dc = $dc_doc->getElementsByTagNameNS('http://purl.org/dc/elements/1.1/', '*');
      $new_dc = new DublinCore();
      foreach ($oai_dc as $child) {
        array_push($new_dc->dc[$child->nodeName], $child->nodeValue);
      }
      return $new_dc;
    }
    else {
      return NULL;
    }
  }

}
