# ![Islandora](https://cloud.githubusercontent.com/assets/2371345/25624809/f95b0972-2f30-11e7-8992-a8f135402cdc.png) Islandora

[![Minimum PHP Version](https://img.shields.io/badge/php-%3E%3D%207.4-8892BF.svg?style=flat-square)](https://php.net/)
[![Build Status](https://github.com/islandora/islandora/actions/workflows/build-2.x.yml/badge.svg)](https://github.com/Islandora/islandora/actions)
[![Contribution Guidelines](http://img.shields.io/badge/CONTRIBUTING-Guidelines-blue.svg)](./CONTRIBUTING.md)
[![LICENSE](https://img.shields.io/badge/license-GPLv2-blue.svg?style=flat-square)](./LICENSE)
[![codecov](https://codecov.io/gh/Islandora/islandora/branch/2.x/graph/badge.svg)](https://codecov.io/gh/Islandora/islandora)

## Introduction

This is the core module of Islandora's digital repository ecosystem. The `islandora` module provides:
- Fedora 5 integration via the [flysystem](https://drupal.org/project/flysystem) module
- Integration with the [context](https://drupal.org/project/context) module to control your digital repository's behaviour
- Publishing messages to a queue so they can be processed in the background

`islandora` contains several submodules and features:
- `islandora_core_feature` (**required**)
  - Configuration required by the `islandora` module
- `islandora_image`
  - Integrates with a [Houdini](https://github.com/Islandora/Crayfish/tree/dev/Houdini) (Imagemagick) server for image processing
- `islandora_audio` and `islandora_video`
  - Integrate with a [Homarus](https://github.com/Islandora/Crayfish/tree/dev/Homarus) (`ffmpeg`) server for audio/video processing 
- `islandora_text_extraction` and `islandora_text_extraction_defaults`
  - Integrate with a [Hypercube](https://github.com/Islandora/Crayfish/tree/dev/Hypercube) (`tessseract` and `pdftotext`) server for text extraction
- `islandora_breadcrumbs`
  - Provides breadcrumbs following collection structure
- `islandora_iiif`
  - Provides IIIF manifests for repository content 

## Requirements

Installing via composer will download all required libraries and modules.  However, for reference, `islandora` requires the following drupal modules:

- [context](http://drupal.org/project/context)
- [search_api](http://drupal.org/project/search_api)
- [jsonld](http://drupal.org/project/jsonld)
- [jwt](http://drupal.org/project/jwt)
- [filehash](http://drupal.org/project/filehash)
- [prepopulate](http://drupal.org/project/prepopulate)
- [eva](http://drupal.org/project/eva)
- [features](http://drupal.org/project/features)
- [migrate_plus](http://drupal.org/project/migrate_plus)
- [migrate_source_csv](http://drupal.org/project/migrate_source_csv)
- [flysystem](http://drupal.org/project/flysystem)

It also requires the following PHP libraries:

- [Crayfish Commons](https://packagist.org/packages/islandora/crayfish-commons)
- [Stomp PHP](http://drupal.org/project/)

If you are using a Drush version less than 10.4 you will also need to install and enable [migrate_tools](http://drupal.org/project/migrate_tools) separately.

## Installation

For a full digital repository solution, see our [installation documentation](https://islandora.github.io/documentation/installation/component_overview/).

To download/enable just this module, use the following from the command line:

```bash
$ composer require islandora/islandora
$ drush en islandora_core_feature
$ drush mim islandora_tags
```

## Configuration

![image](https://user-images.githubusercontent.com/20773151/67234502-ac171900-f41b-11e9-964e-c7d4cfadbd67.png)

You can set the following configuration at `admin/config/islandora/core`:
- Broker URL
  - The URL to your message broker (i.e. Activemq)
- JWT Expiry
  - Set to increase the amount of time that authorization tokens remain valid.  If you have a long running derivative processes or a migration, you may need to set this to be a very long time, e.g. `500d`.  Otherwise, it's best to leave it alone.
- Gemini URL
  - The URL to your Gemini server, which keeps track of where Islandora content is in Fedora.
- Fedora URL Display
  - Selected bundles can display the Fedora URL for repository content.

## Documentation

Further documentation for this module is available on the [Islandora 8 documentation site](https://islandora.github.io/documentation/).

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers

Current maintainers:

* [Danny Lamb](https://github.com/dannylamb)

## Sponsors

* UPEI
* discoverygarden inc.
* LYRASIS
* McMaster University
* University of Limerick
* York University
* University of Manitoba
* Simon Fraser University
* PALS
* American Philosophical Society
* Common Media Inc.

## Development

If you would like to contribute, please get involved by attending our weekly [Tech Call](https://github.com/Islandora/islandora-community/wiki/Weekly-Open-Tech-Call). We love to hear from you!

If you would like to contribute code to the project, you need to be covered by an Islandora Foundation [Contributor License Agreement](https://github.com/Islandora/islandora-community/wiki/Onboarding-Checklist#contributor-license-agreements) or [Corporate Contributor License Agreement](https://github.com/Islandora/islandora-community/wiki/Onboarding-Checklist#contributor-license-agreements). Please see the [Contributor License Agreements](https://github.com/Islandora/islandora-community/wiki/Contributor-License-Agreements) page on the islandora-community wiki for more information.

We recommend using the [islandora-playbook](https://github.com/Islandora-Devops/islandora-playbook) to get started.

## License

[GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)

