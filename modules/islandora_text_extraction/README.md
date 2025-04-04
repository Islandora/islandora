# Islandora Text Extraction

[![Minimum PHP Version](https://img.shields.io/badge/php-%3E%3D%207.2-8892BF.svg?style=flat-square)](https://php.net/)
[![Contribution Guidelines](http://img.shields.io/badge/CONTRIBUTING-Guidelines-blue.svg)](./CONTRIBUTING.md)
[![LICENSE](https://img.shields.io/badge/license-GPLv2-blue.svg?style=flat-square)](./LICENSE)

## Introduction

Provides an action to extract text with a [Hypercube](https://github.com/Islandora/Crayfish/tree/2.x/Hypercube) (`tesseract` and `pdftotext`) server, as well as a Media type to hold the extracted text.

## Requirements

- `islandora` and `islandora_core_feature`
- A Hypercube microservice
- A message broker (e.g. Activemq) for Islandora 8
- An instance of `islandora-connector-derivative` (from [Alpaca](https://github.com/Islandora/Alpaca/tree/1.x/islandora-connector-derivative)) configured for Hypercube

## Installation

For a full digital repository solution (including a Hypercube microservice), see our [installation documentation](https://islandora.github.io/documentation/installation/).

To download/enable just this module, use the following from the command line:

```bash
$ composer require islandora/islandora
$ drush en islandora_core_feature
$ drush mim islandora_tags
$ drush en islandora_text_extraction
```

## Documentation

Official documentation is available on the [Islandora documentation site](https://islandora.github.io/documentation/).

## Sponsors

Original work for this module was done by @ajstanley for @roblib at University of Prince Edward Island.

## Development

If you would like to contribute, please get involved by attending our weekly [Tech Call](https://github.com/Islandora/documentation/wiki). We love to hear from you!

If you would like to contribute code to the project, you need to be covered by an Islandora Foundation [Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_cla.pdf) or [Corporate Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_ccla.pdf). Please see the [Contributors](http://islandora.ca/resources/contributors) pages on Islandora.ca for more information.

We recommend using the [islandora-playbook](https://github.com/Islandora-Devops/islandora-playbook) to get started.

## License

[GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)
