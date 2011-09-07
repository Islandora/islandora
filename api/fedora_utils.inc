<?php

// $Id$

/**
 * @file
 * Base utilities used by the Islandora fedora module.
 */

/*
 * Functions that emulate php5.3 functionality for backwards compatiablity
 */
if (!function_exists('str_getcsv')) {

  function str_getcsv($input, $delimiter=',', $enclosure='"', $escape=NULL, $eol=NULL) {
    $temp = fopen("php://memory", "rw");
    fwrite($temp, $input);
    fseek($temp, 0);
    $r = fgetcsv($temp, 4096, $delimiter, $enclosure);
    fclose($temp);
    return $r;
  }

}

/*
 * Functions that emulate php5.3 functionality for backwards compatiablity
 */

/*
 * Static functions used by the Fedora PHP API.
 */

/**
 * do curl
 * @global type $user
 * @param type $url
 * @param type $return_to_variable
 * @param type $number_of_post_vars
 * @param type $post
 * @return type 
 */
function do_curl($url, $return_to_variable = 1, $number_of_post_vars = 0, $post = NULL) {
  global $user;
  // Check if we are inside Drupal and there is a valid user.
  if ((!isset($user)) || $user->uid == 0) {
    $fedora_user = 'anonymous';
    $fedora_pass = 'anonymous';
  }
  else {
    $fedora_user = $user->name;
    $fedora_pass = $user->pass;
  }

  if (function_exists("curl_init")) {
    $ch = curl_init();
    $user_agent = "Mozilla/4.0 pp(compatible; MSIE 5.01; Windows NT 5.0)";
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
    curl_setopt($ch, CURLOPT_FAILONERROR, TRUE); // Fail on errors
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1); // allow redirects
    curl_setopt($ch, CURLOPT_TIMEOUT, 90); // times out after 90s
    curl_setopt($ch, CURLOPT_USERAGENT, $user_agent);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, $return_to_variable); // return into a variable
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_USERPWD, "$fedora_user:$fedora_pass");
    //curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    if ($number_of_post_vars > 0 && $post) {
      curl_setopt($ch, CURLOPT_POST, $number_of_post_vars);
      curl_setopt($ch, CURLOPT_POSTFIELDS, "$post");
    }
    return curl_exec($ch);
  }
  else {
    if (function_exists(drupal_set_message)) {
      drupal_set_message(t('No curl support.'), 'error');
    }
    return NULL;
  }
}

/**
 * Fedora available
 * @return type 
 */
function fedora_available() {

  $response = do_curl(variable_get('fedora_base_url', 'http://localhost:8080/fedora') . '/describe');
  return strstr($response, 'Repository Information HTML Presentation') !== FALSE;
}

/**
 * Returns a UTF-8-encoded transcripiton of the string given in $in_str.
 * @param string $in_str
 * @return string A UTF-8 encoded string.
 */
function fix_encoding($in_str) {
  $cur_encoding = mb_detect_encoding($in_str);
  if ($cur_encoding == "UTF-8" && mb_check_encoding($in_str, "UTF-8")) {
    return $in_str;
  }
  else {
    return utf8_encode($in_str);
  }
}

/**
 * valid pid ??
 * @param type $pid
 * @return boolean 
 */
function valid_pid($pid) {
  $valid = FALSE;
  if (strlen(trim($pid)) <= 64 && preg_match('/^([A-Za-z0-9]|-|\.)+:(([A-Za-z0-9])|-|\.|~|_|(%[0-9A-F]{2}))+$/', trim($pid))) {
    $valid = TRUE;
  }

  return $valid;
}

/**
 * Valid Dsid ??
 * @param type $dsid
 * @return boolean 
 */
function valid_dsid($dsid) {
  $valid = FALSE;
  if (strlen(trim($dsid)) <= 64 && preg_match('/^[a-zA-Z0-9\_\-\.]+$/', trim($dsid))) {
    $valid = TRUE;
  }

  return $valid;
}

/**
 * fixDsid ??
 * @param type $dsid
 * @return string 
 */
function fix_dsid($dsid) {
  $new_dsid = trim($dsid);

  $find = '/[^a-zA-Z0-9\.\_\-]/';
  $replace = '';
  $new_dsid = preg_replace($find, $replace, $new_dsid);

  if (strlen($new_dsid) > 63)
    $new_dsid = substr($new_dsid, -63);

  if (preg_match('/^[^a-zA-Z]/', $dsid))
    $new_dsid = 'x' . $new_dsid;

  if (strlen($new_dsid) == 0)
    $new_dsid = 'item' . rand(1, 100);

  return $new_dsid;
}

