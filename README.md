# Islandora [![Build Status](https://travis-ci.org/Islandora/islandora.png?branch=7.x)](https://travis-ci.org/Islandora/islandora)

## Introduction

Islandora Fedora Repository Module

For installation and customization instructions please see the [documentation and the DuraSpace Wiki](https://wiki.duraspace.org/display/ISLANDORA/Islandora).

All bugs, feature requests and improvement suggestions are tracked at the [DuraSpace JIRA](https://jira.duraspace.org/browse/ISLANDORA).


## Requirements

This module requires the following modules/libraries:

* [Tuque](https://github.com/islandora/tuque)

Tuque is expected to be in one of two paths:

* sites/all/libraries/tuque (libraries directory may need to be created)
* islandora_folder/libraries/tuque

More detailed requirements are outlined in the [Installing the Islandora Essential Modules](https://wiki.duraspace.org/display/ISLANDORA/milestone+5+-++Installing+the+Islandora+Essential+Modules) chapter of the documentation.

### Optional Requirements

If you want to support languages other than English download and enable [String Translation](https://drupal.org/project/i18n), and follow our [guide](https://github.com/Islandora/islandora/wiki/Multilingual-Support) for setting up additional languges.

## Installation

Before installing Islandora the XACML policies located [here](https://github.com/Islandora/islandora-xacml-policies) should be copied into the Fedora global XACML policies folder. This will allow "authenticated users" in Drupal to access Fedora API-M functions. It is to be noted that the `permit-upload-to-anonymous-user.xml` and `permit-apim-to-anonymous-user.xml` files do not need to be present unless requirements for anonymous ingesting are present.

You will also have to remove some default policies if you want full functionality as well.

Remove `deny-purge-datastream-if-active-or-inactive.xml` to allow for purging of datastream versions.

More detailed information can be found in the 'Set XACML Policies' in the [Installing Fedora](https://wiki.duraspace.org/display/ISLANDORA/milestone+1+-+Installing+Fedora) chapter of the documentation.

## Configuration

The `islandora_drupal_filter` passes the username of 'anonymous' through to Fedora for unauthenticated Drupal Users. A user with the name of 'anonymous' may have XACML policies applied to them that are meant to be applied to Drupal users that are not logged in or vice-versa. This is a potential security issue that can be plugged by creating a user named 'anonymous' and restricting access to the account.

Drupal's cron can be run to remove expired authentication tokens.

**Breadcrumb Generation** on the configuration page, allows you to choose the default breadcrumb generation
 or a custom method (if implemented). 

### Customization

[Customize ingest forms](http://github.com/Islandora/islandora/wiki/Multi-paged-Ingest-Forms)

## Documentation

Further documentation for this module is available at [our wiki](https://wiki.duraspace.org/display/ISLANDORA/Islandora+Core+Module).

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [Diego Pino](https://github.com/DiegoPino)

## Development

If you would like to contribute to this module, please check out [CONTRIBUTING.md](CONTRIBUTING.md). In addition, we have helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the [Islandora.ca](http://islandora.ca) site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)

