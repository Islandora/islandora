#!/bin/bash

# Wrapper for the test executing function so we only have to change it in one place.
# The module name gets passed in as a command line arg.
if [ "$1" == "unit" ]; then
  TYPE="--type=IslandoraKernelTestBase"
elif [ "$1" == "functional" ]; then
  TYPE="--type=IslandoraFunctionalTestBase,WebDriverTestBase"
fi
php core/scripts/run-tests.sh --suppress-deprecations --concurrency=2 --url http://127.0.0.1:8282 --verbose --php `which php` --module "$2" $TYPE
