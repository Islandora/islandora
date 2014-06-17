#!/bin/bash

# Get the end portion of the TRAVIS_REPO_SLUG for the branch name.
IFS=/ read -a DELIMITED_SLUG <<< "$TRAVIS_REPO_SLUG"
export CURRENT_REPO=${DELIMITED_SLUG[1]}

git config --global user.name islandora-logger
git config --global user.email noreply@islandora.ca

# Git business
cd $HOME/drupal-*
export VERBOSE_DIR=`pwd`/sites/default/files/simpletest/verbose
git clone -b $CURRENT_REPO https://islandora-logger:$LOGGER_PW@github.com/Islandora/islandora_travis_logs.git
cd islandora_travis_logs
git checkout -B $CURRENT_REPO

# Out with the old (anything older than a week)
#for i in `find ./ -maxdepth 1 -type d -mtime +7 -print`
#do
#  rm -rf $i
#done
# In with the new
#mkdir $TRAVIS_JOB_NUMBER
rm -rf ./*
cp $VERBOSE_DIR/* ./
cp $HOME/islandora_tomcat/logs/* ./
cp /tmp/drush_webserver.log ./
git add -A
git commit -m "Job: $TRAVIS_JOB_NUMBER Commit: $TRAVIS_COMMIT"
git push origin $CURRENT_REPO --quiet
