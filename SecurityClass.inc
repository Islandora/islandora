<?php



/**
 * @file 
 * SecurityClass Class
 */

/**
 * SecurityClass ??
 */
class SecurityClass {

  public static $SECURITY_CLASS_SECURITY_STREAM = 'POLICY';

  /**
   * Constructor
   */
  function SecurityClass() {
    module_load_include('inc', 'SecurityClass', '');
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  /**
   * canIngestHere ??
   * @global type $user
   * @param type $collection_pid
   * @return type 
   */
  function canIngestHere($collection_pid) {
    global $user;
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $objectHelper = new ObjectHelper();
    // get the childsecurity policy from the collection.
    $policyStream = $objectHelper->getStream($collection_pid, SECURITYCLASS :: $SECURITY_CLASS_SECURITY_STREAM, FALSE);
    if ($policyStream == NULL) {
      // no child policy stream so collection is wide open to anyone to ingest, that has the permission ingest in Drupal.
      // maybe we should return FALSE here?? would be more secure.
      return TRUE;
    }
    $allowedUsersAndRoles = $this->getAllowedUsersAndRoles($policyStream);
    if (!$allowedUsersAndRoles) {
      // error processing stream so don't let them ingest here.
      return FALSE;
    }
    $allowedUsers = $allowedUsersAndRoles["users"];
    $allowedRoles = $allowedUsersAndRoles["roles"];

    foreach ($user->roles as $role) {
      if (in_array($role, $allowedRoles)) {
        return TRUE;
      }
    }

    if (in_array($user->name, $allowedUsers)) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * parses our simple xacml policies checking for users or roles that are allowed to ingest
   * @param type $policyStream
   * @return type 
   */
  function getAllowedUsersAndRoles($policyStream) {
    $allowedRoles = array();
    $allowedUsers = array();
    $usersAndRoles = array();
    try {
      $xml = new SimpleXMLElement($policyStream);
    } catch (Exception $e) {
      watchdog(t("Fedora_Repository"), "No roles found in security policy, could not parse policy stream.", NULL, WATCHDOG_ERROR);
      //we may not want to send this to the screen.
      drupal_set_message(t('No roles found in security policy, could not parse policy stream: !message', array('!message' => check_plain($e->getMessage()))), 'error');
      return NULL;
    }
    $xml->registerXPathNamespace('default', 'urn:oasis:names:tc:xacml:1.0:policy');

    $roles = $xml->xpath('//default:SubjectAttributeDesignator[@AttributeId="fedoraRole"]/../default:Apply/default:AttributeValue');
    $users = $xml->xpath('//default:SubjectAttributeDesignator[@AttributeId="urn:fedora:names:fedora:2.1:subject:loginId"]/../default:Apply/default:AttributeValue');

    if($roles) {
      foreach($roles as $role) {
        $allowedRoles[] = (string)$role;
      }
    }
    if($users) {
      foreach($users as $user) {
        $allowedUsers[] = (string)$user;
      }
    }

    $usersAndRoles['users'] = $allowedUsers;
    $usersAndRoles['roles'] = $allowedRoles;

    return $usersAndRoles;
  }

  /**
   * When a user's profile is saved in drupal we will attempt to create a collection for them in Fedora
   * this will be their personal space.  In the IR it is editable by users with the same role in the VRE
   * it probably would not be.
   * @param type $user
   * @return DOMDocument 
   */
  function createPersonalPolicy($user) {
    $doc = new DOMDocument();
    try {
      $doc->load(drupal_get_path('module', 'Fedora_Repository') . '/policies/noObjectEditPolicy.xml');
    } catch (exception $e) {
      watchdog(t("Fedora_Repository"), "Problem loading policy file.", NULL, WATCHDOG_ERROR);
    }
    $conditions = $doc->getElementsByTagName('Condition');
    foreach ($conditions as $condition) {
      $designator = $condition->getElementsByTagName('SubjectAttributeDesignator');
      foreach ($designator as $des) {
        $attributeId = $des->getAttribute('AttributeId');
        if ($attributeId == 'fedoraRole') {
          $applies = $condition->getElementsByTagName('Apply');
          foreach ($applies as $apply) {
            $functionId = $apply->getAttribute('FunctionId');
            if ($functionId == 'urn:oasis:names:tc:xacml:1.0:function:string-bag') {
              foreach ($user->roles as $role) {
                if (!($role == 'authenticated user' || $role == 'administrator')) { //don't want authenticated user included administrator already is included'
                  $newAttributeValue = $doc->createElement('AttributeValue', '<![CDATA[' . $role . ']]>');
                  $newAttributeValue->setAttribute('DataType', 'http://www.w3.org/2001/XMLSchema#string');
                  // $newAttributeValue->setAttribute('MustBePresent', 'FALSE');
                  $apply->appendChild($newAttributeValue);
                }
              }
            }
          }
        }

        if ($attributeId == 'urn:fedora:names:fedora:2.1:subject:loginId') {
          $applies = $condition->getElementsByTagName('Apply');
          foreach ($applies as $apply) {
            $functionId = $apply->getAttribute('FunctionId');
            if ($functionId == 'urn:oasis:names:tc:xacml:1.0:function:string-bag') {
              $newAttributeValue = $doc->createElement('AttributeValue', $user->name);
              $newAttributeValue->setAttribute('DataType', 'http://www.w3.org/2001/XMLSchema#string');
              //$newAttributeValue->setAttribute('MustBePresent', 'FALSE');
              $apply->appendChild($newAttributeValue);
            }
          }
        }
      }
    }

    return $doc; //NULL; //$xml;
  }

  /**
   * Add a list of allowed users and roles to the given policy stream and return it.
   *
   * @param string $policy_stream
   * @param array $users_and_roles
   * @return DOMDocument
   */
  function set_allowed_users_and_roles(&$policy_stream, $users_and_roles) {
    $allowed_roles = $users_and_roles['roles'];
    $allowed_users = $users_and_roles['users'];
    $dom = new DOMDocument();
    $dom->loadXML($policy_stream);
    $conditions = $dom->getElementsByTagName('Condition');
    foreach ($conditions as $condition) {
      $designator = $condition->getElementsByTagName('SubjectAttributeDesignator');
      foreach ($designator as $des) {
        $attributeId = $des->getAttribute('AttributeId');
        if ($attributeId == 'fedoraRole') {
          // $applies = $condition->getElementsByTagName('Apply');
          $applies = $des->parentNode->getElementsByTagName('Apply');
          foreach ($applies as $apply) {
            $functionId = $apply->getAttribute('FunctionId');
            if ($functionId == 'urn:oasis:names:tc:xacml:1.0:function:string-bag') {
              foreach ($allowed_roles as $role) {
                if (!($role == 'authenticated user' || $role == 'administrator')) { //don't want authenticated user included administrator already is included'
                  $newAttributeValue = $dom->createElement('AttributeValue', $role);
                  $newAttributeValue->setAttribute('DataType', 'http://www.w3.org/2001/XMLSchema#string');
                  //$newAttributeValue->setAttribute('MustBePresent', 'FALSE');
                  $apply->appendChild($newAttributeValue);
                }
              }
            }
          }
        }

        if ($attributeId == 'urn:fedora:names:fedora:2.1:subject:loginId') {
          // $applies = $condition->getElementsByTagName('Apply');
          $applies = $des->parentNode->getElementsByTagName('Apply');
          foreach ($applies as $apply) {
            $functionId = $apply->getAttribute('FunctionId');
            if ($functionId == 'urn:oasis:names:tc:xacml:1.0:function:string-bag') {
              foreach ($allowed_users as $username) {
                $newAttributeValue = $dom->createElement('AttributeValue', $username);
                $newAttributeValue->setAttribute('DataType', 'http://www.w3.org/2001/XMLSchema#string');
                //$newAttributeValue->setAttribute('MustBePresent', 'FALSE');
                $apply->appendChild($newAttributeValue);
              }
            }
          }
        }
      }
    }
    // $this->collection_policy_stream = $dom->saveXML();
    return $dom->saveXML();
  }

}
