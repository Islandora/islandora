<?php
// $Id$

/**
 * @package Islandora
 * @subpackage plugins
 */

/*
 *
 *
 * This Class implements the methods defined in the STANDARD_IMAGE content model
 */

class CreateCollection {
  function CreateCollection() {


  }
  
  function ingestCollectionPolicy($parameterArray = NULL, $dsid, $file, $file_ext = NULL) {
    return TRUE; //nothing needed here as we are not transforming any files
  }



}
