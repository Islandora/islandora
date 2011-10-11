There are a few things to set up to get the Simpletests to run properly.

Fedora Installation

The test sets up the islandora module with the default server settings.
This means it will expect a Fedora server to be running on localhost:8080
with the usual defaults. The tests also expect the Islandora demo objects, i.e.,
islandora:root, islandora:demos collections, etc. to be installed. (Go to
Administer -> Site Configuration -> Fedora Colleciton List and click the Install
Demos tab and follow the instructions.)


Fedora User

Add the following entry to the fedora users file located at
$FEDORA_HOME/server/config/fedora-users.xml:

   <user name="simpletestuser" password="41fe63c9636c6649f0a4747400f0f95e">
      <attribute name="fedoraRole">
        <value>administrator</value>
      </attribute>
    </user>

If you look in the fedora_repository.test file we see that we are creating
a user with a password set to 'simpletestpass'. Fedora requires the hashed
version of this password to do a servlet filter-based authentication.

Drupal Module Setup

The Drupal Simpletest module http://drupal.org/project/simpletest requires a
patch be applied to your Drupal core. See the module's README file for
instructions.

To run the tests go to Administration -> Site Building -> Testing, expand
Fedora Repository and check Fedora Repository, leaving Fedora API unchecked
unless you want to test that as well.  The tests as they are right now should
return with 0 Fails and 0 Exceptions.  If you add new code or functionality
to the module it is your responsibility to add corresponding tests so that other
people making future changes to the codebase don't break your additions.

See the SimpleTest Tutorial on Drupal.org here http://drupal.org/simpletest-tutorial
for a good introduction to creating Simpletests.