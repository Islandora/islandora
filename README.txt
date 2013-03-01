CONTENTS OF THIS FILE
---------------------

 * summary
 * requirements
 * installation
 * configuration
 * customization
 * troubleshooting
 * faq
 * contact
 * sponsors


SUMMARY
-------

Islandora Fedora Repository Module

For installation and customization instructions please see the documentation
and the DuraSpace Wiki:

https://wiki.duraspace.org/display/ISLANDORA/Islandora

All bugs, feature requests and improvement suggestions are tracked at the
DuraSpace JIRA:

https://jira.duraspace.org/browse/ISLANDORA

REQUIREMENTS
------------
The contents of 'tuque' must be added to the sites/all/libraries (libraries directory
may need to be created) directory to make fields in the islandora configuration page
viewable and editable.

INSTALLATION
------------

Before installing Islandora the XACML policies located in the policies folder
should be copied into the Fedora global XACML policies folder. This will allow
"authenticated users" in Drupal to access Fedora API-M functions. Also, the contents
of 'tuque' must be added to the sites/all/libraries directory of your local 
drupal installation (the libraries directory may need to be created). This is a
requirement in 7.x builds to view editable fields in the islandora configuration page
(ex: http://localhost/drupal/admin/islandora/configure).

CONFIGURATION
-------------

The islandora_drupal_filter passes the username of 'anonymous' through to 
Fedora for unauthenticated Drupal Users.  A user with the name of 'anonymous'
may have XACML policies applied to them that are meant to be applied to Drupal
users that are not logged in or vice-versa.  This is a potential security issue
that can be plugged by creating a user named 'anonymous' and restricting access
to the account.

Drupal's cron will can be ran to remove expired authentication tokens.

CUSTOMIZATION
-------------


TROUBLESHOOTING
---------------


F.A.Q.
------


CONTACT
-------


SPONSORS
--------
