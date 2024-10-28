# ![Islandora Image](https://cloud.githubusercontent.com/assets/2371345/24199472/6f7bfb7a-0ee8-11e7-9c94-754762fd5566.png) Islandora Image

[![Minimum PHP Version](https://img.shields.io/badge/php-%3E%3D%207.2-8892BF.svg?style=flat-square)](https://php.net/)
[![Contribution Guidelines](http://img.shields.io/badge/CONTRIBUTING-Guidelines-blue.svg)](./CONTRIBUTING.md)
[![LICENSE](https://img.shields.io/badge/license-GPLv2-blue.svg?style=flat-square)](./LICENSE)

## Introduction

Provides an action to convert images with a [Houdini](https://github.com/Islandora/Crayfish/tree/2.x/Houdini) (`imagemagick`) server.

## Requirements

- `islandora` and `islandora_core_feature`
- A Houdini microservice
- A message broker (e.g. Activemq) for Islandora 8
- `islandora-connector-derivative` (from [Alpaca](https://github.com/Islandora/Alpaca/tree/1.x/islandora-connector-derivative)) configured for Houdini 

## Installation

For a full digital repository solution (including Houdini), see our [installation documentation](https://islandora.github.io/documentation/installation/).

To download/enable just this module, use the following from the command line:

```bash
$ composer require islandora/islandora
$ drush en islandora_core_feature
$ drush mim islandora_tags
$ drush en islandora_image
```

## Documentation

Further documentation for this module is available on the [Islandora documentation site](https://islandora.github.io/documentation/).

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Development

If you would like to contribute, please get involved by attending our weekly [Tech Call](https://github.com/Islandora/documentation/wiki). We love to hear from you!

If you would like to contribute code to the project, you need to be covered by an Islandora Foundation [Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_cla.pdf) or [Corporate Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_ccla.pdf). Please see the [Contributors](http://islandora.ca/resources/contributors) pages on Islandora.ca for more information.

We recommend using the [islandora-playbook](https://github.com/Islandora-Devops/islandora-playbook) to get started.

## License

[GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)
