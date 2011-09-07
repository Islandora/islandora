<?php

// $Id$

/**
 * @file 
 * Connection Helper Class
 */
module_load_include('inc', 'ConnectionHelper', '');

/**
 * Connection Helper Class ??
 */
class ConnectionHelper {

  /**
   * Constructor
   */
  function ConnectionHelper() {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  /**
   * fixURL ??
   * @param type $url
   * @param type $_name
   * @param type $_pass
   * @return string 
   */
  function _fixURL($url, $_name, $_pass) {
    if (empty($url)) {
      $url = variable_get('fedora_soap_url', 'http://localhost:8080/fedora/services/access?wsdl');
    }

    $creds = urlencode($_name) . ':' . urlencode($_pass);

    if (strpos($url, 'http://') === 0) {
      $new_url = 'http://' . $creds . '@' . substr($url, 7);
    }
    elseif (strpos($url, 'https://') === 0) {
      $new_url = 'https://' . $creds . '@' . substr($url, 8);
    }
    else {
      drupal_set_message(t('Invalid URL: !url', array('!url' => $url)));
      return NULL;
    }

    return $new_url;
  }

  /**
   * getSoapClient
   * @global type $user
   * @param type $url
   * @param type $exceptions
   * @return SoapClient 
   */
  function getSoapClient($url = NULL, $exceptions = TRUE) {
    if (empty($url)) {
      $url = variable_get('fedora_soap_url', 'http://localhost:8080/fedora/services/access?wsdl');
    }

    global $user;
    if ($user->uid == 0) {
      //anonymous user.  We will need an entry in the fedora users.xml file
      //with the appropriate entry for a username of anonymous password of anonymous
      try {
        $client = new SoapClient($this->_fixURL($url, 'anonymous', 'anonymous'), array(
            'login' => 'anonymous',
            'password' => 'anonymous',
            'exceptions' => $exceptions,
          ));
      } catch (SoapFault $e) {
        drupal_set_message(t("!e", array('!e' => $e->getMessage())));
        return NULL;
      }
    }
    else {
      try {
        $client = new SoapClient($this->_fixURL($url, $user->name, $user->pass), array(
            'login' => $user->name,
            'password' => $user->pass,
            'exceptions' => TRUE,
          ));
      } catch (SoapFault $e) {
        drupal_set_message(t("!e", array('!e' => $e->getMessage())));
        return NULL;
      }
    }
    return $client;
  }

}

