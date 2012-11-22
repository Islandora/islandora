<?php

/**
 * @file
 */

define('TOKEN_TIMEOUT', 30000);

function islandora_get_object_token($pid, $dsid) {
  global $user;
  $time = time();
  $token = hash("sha256", $user->uid . $pid . $dsid . $time);

  $id = db_insert("islandora_authtokens")
    ->fields(array(
      'token' => $token,
      'uid' => $user->uid,
      'pid' => $pid,
      'dsid' => $dsid,
      'time' => $time,
    ))
    ->execute();
  return $token;
}

function islandora_validate_object_token($pid, $dsid, $token) {
  global $user;
  // check for database token
  $time = time();
  $result = db_select("islandora_authtokens", "id")
    ->fields("id")
    ->condition('token', $token, '=')
    ->condition('uid', $user->uid, '=')
    ->condition('pid', $pid, '=')
    ->condition('dsid', $dsid, '=')
    ->condition('time', $time, '<=')
    ->condition('time', $time-TOKEN_TIMEOUT, '>')
    ->execute()
    ->rowCount();

  //** this is for one-time use tokens **//
  // remove the authtoken (if it exists) so it can't be used again
  db_delete("islandora_authtokens")
    ->condition('token', $token, '=')
    ->condition('uid', $user->uid, '=')
    ->condition('pid', $pid, '=')
    ->condition('dsid', $dsid, '=')
    ->execute();

//  print_r($result);
  return $result > 0;
}
