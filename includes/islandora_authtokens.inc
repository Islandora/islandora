<?php
/**
 * @file
 *   Library functions for handling authentication tokens.
 *   These are to be used when dealing with applications
 *   such as Djatoka that do not pass through credentials.
 */

// Token lifespan(seconds): after this duration the token expires.
// 5 minutes.
define('TOKEN_TIMEOUT', 300);

/**
 * Request Islandora to construct an object/datastream authentication token.
 * This token can later be turned in for access to the requested object or
 * datastream.
 *
 * @param string $pid
 *   The Fedora PID to generate the token for.
 * @param string $dsid
 *   The Fedora datastream ID to generate the token for.
 * @param integer $uses
 *   Defaults to 1.
 *   The number of uses the token should be used for.  There are
 *   times when this should be greater than 1: ie. Djatoka needs
 *   to make two calls.
 *
 * @return string
 *   The generated authentication token.
 */
function islandora_get_object_token($pid, $dsid, $uses = 1) {
  global $user;
  $time = time();
  // The function mt_rand is not considered cryptographically secure
  // and openssl_rando_pseudo_bytes() is only available in PHP > 5.3.
  // We might be safe in this case because mt_rand should never be using
  // the same seed, but this is still more secure.
  $token = hash("sha256", mt_rand() . $time);

  $id = db_insert("islandora_authtokens")->fields(
    array(
      'token' => $token,
      'uid' => $user->uid,
      'pid' => $pid,
      'dsid' => $dsid,
      'time' => $time,
      'remaining_uses' => $uses,
    ))->execute();

  return $token;
}

/**
 * Submit a token to islandora for authentication. Supply islandora with the
 * token and the object/datastream it is for and you will receive access if
 * authentication passes. Tokens can only be redeemed in a short window after
 * their creation.
 *
 * @param string $pid
 *   The PID of the object to retrieve.
 * @param string $dsid
 *   The datastream id to retrieve.
 * @param string $token
 *   The registered token that allows access to this object.
 *
 * @return mixed
 *   The user credentials for access if the token validation passes,
 *   FALSE otherwise
 */
function islandora_validate_object_token($pid, $dsid, $token) {
  // Check for database token.
  $time = time();
  $query = db_select('islandora_authtokens', 'tokens');
  $query->join('users', 'u', 'tokens.uid = u.uid');
  // The results will look like user objects.
  $result = $query
    ->fields('u', array('uid', 'name', 'pass'))
    ->fields('tokens', array('remaining_uses'))
    ->condition('token', $token, '=')
    ->condition('pid', $pid, '=')
    ->condition('dsid', $dsid, '=')
    ->condition('time', $time, '<=')
    ->condition('time', $time - TOKEN_TIMEOUT, '>')
    ->execute()
    ->fetchAll();
  if ($result) {
    $remaining_uses = $result[0]->remaining_uses;
    $remaining_uses--;
    // Remove the authentication token so it can't be used again.
    if ($remaining_uses == 0) {
      db_delete("islandora_authtokens")
        ->condition('token', $token, '=')
        ->condition('pid', $pid, '=')
        ->condition('dsid', $dsid, '=')
        ->execute();
    }
    // Decrement authentication token uses.
    else {
      db_update("islandora_authtokens")
      ->fields(array('remaining_uses' => $remaining_uses))
      ->condition('token', $token, '=')
      ->condition('pid', $pid, '=')
      ->condition('dsid', $dsid, '=')
      ->execute();
    }
    unset($result[0]->remaining_uses);
    return $result[0];
  }
  else {
    return FALSE;
  }
}

/**
 * Will remove any expired authentication tokens.
 */
function islandora_remove_expired_tokens() {
  $time = time();
  db_delete("islandora_authtokens")
    ->condition('time', $time - TOKEN_TIMEOUT, '<')
    ->execute();
}
