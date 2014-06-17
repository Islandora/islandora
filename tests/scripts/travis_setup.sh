#!/bin/bash

mysql -u root -e 'create database drupal;'
mysql -u root -e "create database fedora;"
mysql -u root -e "GRANT ALL PRIVILEGES ON fedora.* To 'fedora'@'localhost' IDENTIFIED BY 'fedora';"
mysql -u root -e "GRANT ALL PRIVILEGES ON drupal.* To 'drupal'@'localhost' IDENTIFIED BY 'drupal';"
cd $HOME
git clone git://github.com/Islandora/tuque.git
git clone -b $FEDORA_VERSION git://github.com/Islandora/islandora_tomcat.git
cd islandora_tomcat
git fsck
export CATALINA_HOME='.'
export JAVA_OPTS="-Xms1024m -Xmx1024m -XX:MaxPermSize=512m -XX:+CMSClassUnloadingEnabled -Djavax.net.ssl.trustStore=$CATALINA_HOME/fedora/server/truststore -Djavax.net.ssl.trustStorePassword=tomcat"
./bin/startup.sh
cd $HOME
pear channel-discover pear.drush.org
pear upgrade --force Console_Getopt
pear upgrade --force pear
pear channel-discover pear.drush.org

# "prefer-source" required due to SSL shenanigans on the Travis boxes...
composer global require --prefer-source 'squizlabs/php_codesniffer=*' 'sebastian/phpcpd=*' 'drush/drush:6.3.0'
# Because we can't add to the PATH here and this file is used in many repos,
# let's just throw symlinks in.
find $HOME/.composer/vendor/bin -executable \! -type d -exec sudo ln -s {}  /usr/local/sbin/ \;

phpenv rehash
drush dl --yes drupal
cd drupal-*
drush si minimal --db-url=mysql://drupal:drupal@localhost/drupal --yes
# Needs to make things from Composer be available (PHP CS, primarily)
sudo chmod a+w sites/default/settings.php
echo "include_once '$HOME/.composer/vendor/autoload.php';" >> sites/default/settings.php
sudo chmod a-w sites/default/settings.php
drush runserver --php-cgi=$HOME/.phpenv/shims/php-cgi localhost:8081 &>/tmp/drush_webserver.log &
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
