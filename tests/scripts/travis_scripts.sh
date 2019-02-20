#!/bin/bash

# Common checks to get run during the 'script' section in Travis.
OUTPUT=0

# Make OUTPUT equal return code if return code is not 0
function checkReturn {
  if [ $1 -ne 0 ]; then
    OUTPUT=$1
  fi
}

# Lint
echo "PHP Lint"
find $TRAVIS_BUILD_DIR -type f \( -name '*.php' -o -name '*.inc' -o -name '*.module' -o -name '*.install' -o -name '*.test' \) -print0 | xargs -0 -n1 php -l
checkReturn $?

# Check line endings
echo "Check line endings"
$ISLANDORA_DIR/tests/scripts/line_endings.sh $TRAVIS_BUILD_DIR
checkReturn $?

# Coding standards
echo "Drush coder-review"
drush coder-review --reviews=production,security,style,i18n,potx $TRAVIS_BUILD_DIR
checkReturn $?

# Skip code sniffer for PHP 5.3.3
if [ "$(phpenv version-name)" != "5.3.3" ]; then
  # Code sniffer
  if [ -d "$HOME/.composer/vendor/drupal/coder/coder_sniffer/Drupal" ]; then
    DRUPAL_SNIFFS=$HOME/.composer/vendor/drupal/coder/coder_sniffer/Drupal
  elif [ -d "$HOME/.config/composer/vendor/drupal/coder/coder_sniffer/Drupal" ]; then
    DRUPAL_SNIFFS=$HOME/.config/composer/vendor/drupal/coder/coder_sniffer/Drupal
  else
    DRUPAL_SNIFFS="Drupal"
  fi
  if [ -z "$CODESNIFFER_IGNORE" ]; then
    CODESNIFFER_IGNORE="vendor,*.info,*.txt,*.md"
  fi
  echo "PHP Codesniffer"
  /usr/bin/phpcs --standard=$DRUPAL_SNIFFS --extensions="php,module,inc,install,test" --ignore="$CODESNIFFER_IGNORE" $TRAVIS_BUILD_DIR
  checkReturn $?
else
  echo "Skipping PHP Codesniffer for PHP 5.3.3"
fi

# Copy/paste detection
echo "PHP Copy Paste Detection"
phpcpd --names *.module,*.inc,*.test $TRAVIS_BUILD_DIR
checkReturn $?

exit $OUTPUT
