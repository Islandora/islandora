#!/bin/bash

# Wrapper for the test executing function so we only have to change it in one place.
# The module name gets passed in as a command line arg.
COMMAND="$DRUPAL_DIR/vendor/bin/phpunit --verbose --testsuite $1"
echo $COMMAND
$COMMAND
