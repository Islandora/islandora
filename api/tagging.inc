<?php



/**
 * @file
 * TagSet Class
 */

/**
 * TagSet Class
 */
class TagSet {

  public $tags = array();
  public $item = NULL;
  public $tagsDSID = 'TAGS';

  /**
   * Constructor
   * @param type $item 
   */
  function TagSet($item = NULL) {
    if (!empty($item) && get_class($item) == 'Fedora_Item') {
      $this->item = $item;
      $this->load();
    }
  }

  /**
   * Load ??
   * @return type 
   */
  function load() {
    $tagsxml = isset($this->item->datastreams[$this->tagsDSID]) ? $this->item->get_datastream_dissemination($this->tagsDSID) : NULL;
    if (empty($tagsxml)) {
      $this->tags = array();
      return FALSE;
    }
    $tagsdoc = new DOMDocument();
    $tagsdoc->loadXML($tagsxml);
    $tags = $tagsdoc->getElementsByTagName('tag');
    foreach ($tags as $tag) {
      $this->tags[] = array(
        'name' => $tag->nodeValue,
        'creator' => $tag->getAttribute('creator')
      );
    }
  }

  /**
   * Saves an associative array of tags to a datastream.
   */
  function save() {
    $tagdoc = new DomDocument();
    $e_tags = new DomElement('tags');
    $tagdoc->appendChild($e_tags);
    foreach ($this->tags as $tag) {
      $e_tag = $tagdoc->createElement('tag', $tag['name']);
      $e_tag->setAttribute('creator', (!empty($tag['creator'])) ? $tag['creator'] : '');
      $e_tags->appendChild($e_tag);
    }
    try {
      $datastreams = $this->item->get_datastreams_list_as_array();
      if (empty($datastreams[$this->tagsDSID])) {
        $this->item->add_datastream_from_string($tagdoc->saveXML(), $this->tagsDSID, 'Tags', 'text/xml', 'X');
      }
      else {
        $this->item->modify_datastream_by_value($tagdoc->saveXML(), $this->tagsDSID, 'Tags', 'text/xml', 'X');
      }
    } catch (exception $e) {
      drupal_set_message(t('There was an error saving the tags datastream: @e'), array('@e' => $e), 'error');
      return FALSE;
    }
    return TRUE;
  }

}
