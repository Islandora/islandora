#!/bin/bash

# Wrapper for the test executing function so we only have to change it in one place.
# The module name gets passed in as a command line arg.
php scripts/run-tests.sh --php `phpenv which php` --url http://localhost:8081 --verbose "$1"
