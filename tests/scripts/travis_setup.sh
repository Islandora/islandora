#!/bin/bash

# Database creation and priveleges.
mysql -u root -e 'create database drupal;'
mysql -u root -e "create database fedora;"
mysql -u root -e "GRANT ALL PRIVILEGES ON fedora.* To 'fedora'@'localhost' IDENTIFIED BY 'fedora';"
mysql -u root -e "GRANT ALL PRIVILEGES ON drupal.* To 'drupal'@'localhost' IDENTIFIED BY 'drupal';"

# Java 8, if needed.
if [ $FEDORA_VERSION = "3.8.1" ]; then
  sudo add-apt-repository -y ppa:webupd8team/java
  sudo apt-get update
  sudo apt-get install -y oracle-java8-installer oracle-java8-set-default
  sudo update-java-alternatives -s java-8-oracle
  export JAVA_HOME=/usr/lib/jvm/java-8-oracle
fi

# Islandora Tomcat installation.
cd $HOME
git clone git://github.com/Islandora/tuque.git
wget http://alpha.library.yorku.ca/islandora_tomcat.$FEDORA_VERSION.tar.gz
tar xf islandora_tomcat.$FEDORA_VERSION.tar.gz
cd islandora_tomcat
export CATALINA_HOME='.'
export JAVA_OPTS="-Xms1024m -Xmx1024m -XX:MaxPermSize=512m -XX:+CMSClassUnloadingEnabled -Djavax.net.ssl.trustStore=$CATALINA_HOME/fedora/server/truststore -Djavax.net.ssl.trustStorePassword=tomcat"
# TODO: roll a Fedora 3.8.1 islandora_tomcat that doesn't require a rebuild.
if [ $FEDORA_VERSION = "3.8.1" ]; then
  export FEDORA_HOME=$HOME/islandora_tomcat/fedora
  ./fedora/server/bin/fedora-rebuild.sh -r org.fcrepo.server.utilities.rebuild.SQLRebuilder
fi
./bin/startup.sh

cd $HOME

# Drush and PHPCS installation.
if [ "$(phpenv version-name)" == "5.3.3" ]; then
  # No mod_openssl in Travis 5.3.3 boxes
  composer config -g disable-tls true
  composer config -g secure-http false
  composer self-update
  composer global require drush/drush:6.*
  composer global require drupal/coder:7.*
else
  composer global require drush/drush:7.*
  composer global require drupal/coder
fi

# Symlink the things
if [ -f "$HOME/.config/composer/vendor/bin/drush" ]; then
  sudo ln -s $HOME/.config/composer/vendor/bin/drush /usr/bin/drush
elif [ -f "$HOME/.composer/vendor/bin/drush" ]; then
  sudo ln -s $HOME/.composer/vendor/bin/drush /usr/bin/drush
else
  echo "Could not find drush"
  exit 1
fi
/usr/bin/drush version

if [ -f "$HOME/.config/composer/vendor/bin/phpcs" ]; then
  sudo ln -s $HOME/.config/composer/vendor/bin/phpcs /usr/bin/phpcs
elif [ -f "$HOME/.composer/vendor/bin/phpcs" ]; then
  sudo ln -s $HOME/.composer/vendor/bin/phpcs /usr/bin/phpcs
else
  echo "Did not find phpcs"
  exit 1
fi
/usr/bin/phpcs --version

# PHP Copy-Paste Detection installation.
composer global require --dev sebastian/phpcpd
if [ -f "$HOME/.config/composer/vendor/bin/phpcpd" ]; then
  sudo ln -s $HOME/.config/composer/vendor/bin/phpcpd /usr/local/bin/phpcpd
elif [ -f "$HOME/.composer/vendor/bin/phpcpd" ]; then
  sudo ln -s $HOME/.composer/vendor/bin/phpcpd /usr/local/bin/phpcpd
else
  echo "Did not find phpcpd"
  exit 1
fi
/usr/local/bin/phpcpd --version

# Drupal installation.
phpenv rehash
drush dl --yes drupal
cd drupal-*
drush si minimal --db-url=mysql://drupal:drupal@localhost/drupal --yes
if [ "$(phpenv version-name)" == "5.3.3" ]; then
  drush runserver --php-cgi=$HOME/.phpenv/shims/php-cgi localhost:8081 &>/tmp/drush_webserver.log &
else
  drush runserver localhost:8081 &>/tmp/drush_webserver.log &
fi
# Add Islandora to the list of symlinked modules.
ln -s $ISLANDORA_DIR sites/all/modules/islandora
# Use our custom Travis test config for Simpletest.
mv sites/all/modules/islandora/tests/travis.test_config.ini sites/all/modules/islandora/tests/test_config.ini
# Grab Tuque.
mkdir sites/all/libraries
ln -s $HOME/tuque sites/all/libraries/tuque
# Grab and enable other modules.
drush dl --yes coder-7.x-2.4
drush dl --yes potx-7.x-1.0
drush en --yes coder_review
drush en --yes simpletest
drush en --yes potx
drush en --user=1 --yes islandora
drush cc all
# The shebang in this file is a bogeyman that is haunting the web test cases.
rm /home/travis/.phpenv/rbenv.d/exec/hhvm-switcher.bash
sleep 20
