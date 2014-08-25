#!/bin/bash

mysql -u root -e 'create database drupal;'
mysql -u root -e "create database fedora;"
mysql -u root -e "GRANT ALL PRIVILEGES ON fedora.* To 'fedora'@'localhost' IDENTIFIED BY 'fedora';"
mysql -u root -e "GRANT ALL PRIVILEGES ON drupal.* To 'drupal'@'localhost' IDENTIFIED BY 'drupal';"
cd $HOME
git clone git://github.com/Islandora/tuque.git
wget http://alpha.library.yorku.ca/islandora_tomcat.$FEDORA_VERSION.tar.gz
tar xf islandora_tomcat.$FEDORA_VERSION.tar.gz
cd islandora_tomcat
export CATALINA_HOME='.'
export JAVA_OPTS="-Xms1024m -Xmx1024m -XX:MaxPermSize=512m -XX:+CMSClassUnloadingEnabled -Djavax.net.ssl.trustStore=$CATALINA_HOME/fedora/server/truststore -Djavax.net.ssl.trustStorePassword=tomcat"
./bin/startup.sh
cd $HOME

wget http://alpha.library.yorku.ca/drush-6.3.tar.gz
tar xf drush-6.3.tar.gz
mv drush-6.3 /opt/
ln -s /opt/drush/drush-6.3 /usr/bin/drush

wget http://alpha.library.yorku.ca/PHP_CodeSniffer-1.5.4.tgz
pear install PHP_CodeSniffer-1.5.4.tgz

wget http://alpha.library.yorku.ca/phpcpd.phar
mv phpcpd.phar /usr/local/bin/phpcpd

phpenv rehash
drush dl --yes drupal
cd drupal-*
drush si minimal --db-url=mysql://drupal:drupal@localhost/drupal --yes
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
# The shebang in this file is a bogeyman that is haunting the web test cases.
rm /home/travis/.phpenv/rbenv.d/exec/hhvm-switcher.bash
sleep 20
