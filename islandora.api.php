<?php

function hook_islandora_purge_datastream ($object_id, $datastream_id){}

function hook_islandora_purge_object($object_id) {} 

function hook_islandora_view_object($object_id){}

function hook_islandora_get_types(){}

function hook_islandora_add_datastream($object_id) {}

/**
 * creates and populates a php Fedora object. 
 */
function hook_islandora_preingest_alter(){}

function hook_islandora_postingest($object){}

function hook_islandora_datastream_edit($object, $dsid){}