# Islandora IIIF

[![Minimum PHP Version](https://img.shields.io/badge/php-%3E%3D%207.2-8892BF.svg?style=flat-square)](https://php.net/)
[![Contribution Guidelines](http://img.shields.io/badge/CONTRIBUTING-Guidelines-blue.svg)](./CONTRIBUTING.md)
[![LICENSE](https://img.shields.io/badge/license-GPLv2-blue.svg?style=flat-square)](./LICENSE)

## Introduction

Provides IIIF manifests using views.

## Requirements

- `islandora` and `islandora_core_feature`
- A IIIF image server (such as Cantaloupe)

## Installation

For a full digital repository solution, see our [installation documentation](https://islandora.github.io/documentation/installation/).

To download/enable just this module, use the following from the command line:

```bash
$ composer require islandora/islandora
$ drush en islandora_core_feature
$ drush mim islandora_tags
$ drush en islandora_iiif
```

## Configuration

You can set the following configuration at `admin/config/islandora/iiif`:
- IIIF Image server location
  - The URL to your IIIF image server (without trailing slash).

### Views Style Plugin

This module implements a Views Style plugin. It provides the following settings:

1. Tile Source: A field that was added to the views list of fields with the image to be served. This should be a File or Image type field on a Media.
2. Structured Text field: This lets you specify a file field   where OCR text with positional data, e.g., hOCR can be found.
## Documentation

Official documentation is available on the [Islandora 8 documentation site](https://islandora.github.io/documentation/).

## Development

If you would like to contribute, please get involved by attending our weekly [Tech Call](https://github.com/Islandora/documentation/wiki). We love to hear from you!

If you would like to contribute code to the project, you need to be covered by an Islandora Foundation [Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_cla.pdf) or [Corporate Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_ccla.pdf). Please see the [Contributors](http://islandora.ca/resources/contributors) pages on Islandora.ca for more information.

We recommend using the [islandora-playbook](https://github.com/Islandora-Devops/islandora-playbook) to get started.

## License

[GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)
