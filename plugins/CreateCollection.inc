<?php



/**
 * @file
 * Create Collection Class
 */

/**
 * This Class implements the methods defined in the STANDARD_IMAGE content model
 */
class CreateCollection {

  /**
   * Constructor
   */
  function CreateCollection() {
    
  }

  /**
   * ingest collection policy ???
   * @param type $parameterArray
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return type 
   */
  function ingestCollectionPolicy($parameterArray = NULL, $dsid, $file, $file_ext = NULL) {
    return TRUE; //nothing needed here as we are not transforming any files
  }

}
