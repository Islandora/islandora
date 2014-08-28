#!/bin/bash

# Get the end portion of the TRAVIS_REPO_SLUG for the branch name.
IFS=/ read -a DELIMITED_SLUG <<< "$TRAVIS_REPO_SLUG"
export CURRENT_REPO=${DELIMITED_SLUG[1]}

git config --global user.name islandora-logger
git config --global user.email noreply@islandora.ca

# Git business
cd $HOME/drupal-*
export VERBOSE_DIR=`pwd`/sites/default/files/simpletest/verbose
git clone -b $CURRENT_REPO https://github.com/Islandora/islandora_travis_logs.git
cd islandora_travis_logs
git checkout -B $CURRENT_REPO

# Out with the old, in with the new
rm -rf ./*
cp $VERBOSE_DIR/* ./
cp $HOME/islandora_tomcat/logs/* ./
cp /tmp/drush_webserver.log ./
git add -A
git commit -m "Job: $TRAVIS_JOB_NUMBER Commit: $TRAVIS_COMMIT"
git push origin $CURRENT_REPO --quiet
