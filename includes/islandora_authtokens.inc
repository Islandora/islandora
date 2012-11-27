<?php

/**
 * @file
 */

// Token lifespan: after this duration the token expires.
define('TOKEN_TIMEOUT', 30000);

/**
 * Request islandora to construct an object/datastream authentication token.
 * This token can later be turned in for access to the requested object or
 * datastream.
 * @param string $pid
 * @param string $dsid
 * @return The generated authentication token.
 */
function islandora_get_object_token($pid, $dsid) {
  global $user;
  $time = time();
  $token = hash("sha256", $user->uid . $pid . $dsid . $time);

/* optional block to check if this request is allowed
// CURRENTLY DISABLED
  module_load_include("inc", "islandora", "includes/tuque");
  // test if this is a valid request
  $validator = new IslandoraTuque($user);
  try {
    $result = $validator->connection->getRequest("objects/$pid/datastreams/$dsid/content", true);
  }
  catch (RepositoryException $rx) {
    //print_r("authentication failed");
    return FALSE;
  }
*/

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

/**
 * Submit a token to islandora for authentication.  Supply islandora with the
 * token and the object/datastream it is for and you will receive access if
 * authentication passes.  Tokens can only be redeemed in a short window after
 * their creation.
 * @param string $pid
 *  The pid of the object to retrieve.
 * @param string @dsid
 *  The datastream id to retrieve.
 * @param string $token
 *  The registered token that allows access to this object.
 * @return The user credentials for access if the token validation passes,
 *  FALSE otherwise
 */
function islandora_validate_object_token($pid, $dsid, $token) {
  global $user;
  // check for database token
  $time = time();
  $query = db_select('islandora_authtokens', 'tokens');
  $query->join('users', 'u', 'tokens.uid = u.uid');
  $result = $query
    ->fields('u', array('uid', 'name', 'pass'))
    ->condition('token', $token, '=')
    ->condition('pid', $pid, '=')
    ->condition('dsid', $dsid, '=')
    ->condition('time', $time, '<=')
    ->condition('time', $time-TOKEN_TIMEOUT, '>')
    ->execute()
    ->fetchAll();

  //** this is for one-time use tokens **//
  // remove the authtoken (if it exists) so it can't be used again
  db_delete("islandora_authtokens")
    ->condition('token', $token, '=')
    ->condition('pid', $pid, '=')
    ->condition('dsid', $dsid, '=')
    ->execute();
  //** **//

  if ($result) {
    return $result[0];
  }
  else {
    return FALSE;
  }
}

