#!/bin/bash

# Setup phpcs and drupal sniffs.
mkdir $TOOLS_DIR
cd $TOOLS_DIR
composer require drupal/coder
composer require dealerdirect/phpcodesniffer-composer-installer
$TOOLS_DIR/vendor/bin/phpcs -i
$TOOLS_DIR/vendor/bin/phpcs --version
