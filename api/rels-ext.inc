<?php



/**
 * @file
 * RelsExt class
 */

/**
 * RelsExt class
 */
class RelsExt {

  // Instance variables
  public $relsExtArray = array();
  private $originalRelsExtArray = array(); // Used to determine the result of modified() funciton.

  /**
   * Constructor that builds itself by retrieving the RELS-EXT stream from
   * the repository for the given Fedora_Item.
   * @param Fedora_Item $item
   */
  function RelsExt($item) {
    $relsextxml = $item->get_datastream_dissemination('RELS-EXT');
  }

  /**
   * modified 
   * @return type 
   */
  function modified() {
    return!(empty(array_diff($this->relsExtArray, $this->originalRelsExtArray)) &&
    empty(array_diff($this->originalRelsExtArray, $this->relsExtArray)));
  }

  /**
   * Save the current state of the RELS-EXT array out to the repository item
   * as a datastream.
   */
  function save() {
    
  }

}

