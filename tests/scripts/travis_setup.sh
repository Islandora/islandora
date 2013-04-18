#!/bin/bash     

mysql -u root -e 'create database drupal;'
mysql -u root -e "create database fedora;"
mysql -u root -e "GRANT ALL PRIVILEGES ON fedora.* To 'fedora'@'localhost' IDENTIFIED BY 'fedora';"
mysql -u root -e "GRANT ALL PRIVILEGES ON drupal.* To 'drupal'@'localhost' IDENTIFIED BY 'drupal';"
cd $HOME
git clone git://github.com/Islandora/tuque.git
git clone -b $FEDORA_VERSION git://github.com/Islandora/islandora_tomcat.git
cd islandora_tomcat
export CATALINA_HOME='.'
./bin/startup.sh
cd $HOME
pyrus channel-discover pear.drush.org
pyrus channel-discover pear.phpqatools.org
pyrus channel-discover pear.netpirates.net
pyrus install drush/drush
pyrus install pear/PHP_CodeSniffer
pyrus install pear.phpunit.de/phpcpd
phpenv rehash
drush dl --yes drupal
cd drupal-*
drush si standard --db-url=mysql://drupal:drupal@localhost/drupal --yes
drush runserver --php-cgi=$HOME/.phpenv/shims/php-cgi localhost:8081 &>/dev/null &
ln -s $ISLANDORA_DIR sites/all/modules/islandora
mv sites/all/modules/islandora/tests/travis.test_config.ini sites/all/modules/islandora/tests/test_config.ini
mkdir sites/all/libraries
ln -s $HOME/tuque sites/all/libraries/tuque
drush dl --yes coder
drush dl --yes potx
drush en --yes coder_review
drush en --yes simpletest
drush en --yes potx
drush en --user=1 --yes islandora
drush cc all
sleep 4
