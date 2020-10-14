#!/bin/bash

# Wrapper for the test executing function so we only have to change it in one place.
# The module name gets passed in as a command line arg.
if [ "$1" == "unit" ]; then
  TYPE="--types \"PHPUnit-Unit,PHPUnit-Kernel,Simpletest\""
elif [ "$1" == "functional" ]; then
  TYPE="--types \"PHPUnit-FunctionalJavascript,PHPUnit-Functional\""
fi
php core/scripts/run-tests.sh --suppress-deprecations --concurrency 2 --url http://127.0.0.1:8282 --verbose --php `which php` --module "$2" $TYPE
