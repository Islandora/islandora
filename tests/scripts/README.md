USING THE TRAVIS AFTER-FAILURE SCRIPT
*************************************

The Travis after-failure script dumps several files to per-repository branches
on https://github.com/Islandora/islandora_travis_logs if one of the Travis
builds fails. Most of this process is automated, but it still requires some
work to implement in a repository:

- An encrypted global variable in .travis.yml representing the password of the
islandora-logger Github user.
- A line to run travis_after_failure.sh in the after-failure section of
.travis.yml.
- A new branch of islandora_travis_logger, using the repository name.

THE ENCRYPTED GLOBAL VARIABLE
*****************************

Encrypted global variables can be generated using the travis ruby gem. To find
the appropriate encryption key for your repository, run the following:

```bash
sudo gem install travis # Assuming travis isn't installed
travis encrypt -r account/repository LOGGER_PW=islandora-logger-password
```

Where:

- `account/repository` represents the repository slug, e.g. Islandora/islandora.
- `islandora-logger-password` represents the password for the islandora-logger
account, in plaintext.

The output of this will be a string of encrypted data in quotes, preceded by the
word 'secure', looking something like:

```
secure: ".... a giant string of encrypted data ...."
```

Paste this line into .travis.yml's `env/global` section, so that it looks like:

```
env:
  matrix:
    - MATRIX_VARIABLES="GO_HERE"
  global:
    - secure: ".... a giant string of encrypted data ...."
```

THE LINE TO RUN THE AFTER-FAILURE SCRIPT
****************************************

At the end of your .travis.yml, after the `script` section, add the following:

```
after-failure:
 - $ISLANDORA_DIR/tests/scripts/travis_after_failure.sh
 # Where $ISLANDORA_DIR represents the path to the Islandora module; this has
 # likely already been set in before_install in order to run travis_setup.sh
```

THE NEW ISLANDORA_TRAVIS_LOGS BRANCH
************************************

Really very simple ... just add a new branch to
git://github.com/Islandora/islandora_travis_logs/ with your repository name.