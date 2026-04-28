# Islandora Microservice Rewrite

[![Minimum PHP Version](https://img.shields.io/badge/php-%3E%3D%207.2-8892BF.svg?style=flat-square)](https://php.net/)
[![Contribution Guidelines](http://img.shields.io/badge/CONTRIBUTING-Guidelines-blue.svg)](./CONTRIBUTING.md)
[![LICENSE](https://img.shields.io/badge/license-GPLv2-blue.svg?style=flat-square)](./LICENSE)

## Introduction

`islandora_microservice_rewrite` is an optional Islandora submodule that rewrites selected URI fields in generated derivative messages before those messages are serialized and sent to downstream microservices.

This is intended for split-host or containerized deployments where the public Drupal URL is not the right URL for internal services such as Alpaca or derivative processors. In those environments, derivative messages may need to use an internal hostname, proxy address, or alternate service path that is reachable from the microservice network.

## Requirements

- `islandora`

## Installation

For a full digital repository solution, see our [installation documentation](https://islandora.github.io/documentation/installation/).

To download/enable just this module, use the following from the command line:

```bash
$ composer require islandora/islandora
$ drush en islandora_islandora_microservice_rewrite
```

## Documentation

### What It Rewrites

When enabled, the module listens to Islandora's generated-message event and applies simple string replacements to these attachment content fields when they are present:

- `source_uri`
- `destination_uri`
- `file_upload_uri`

### Configuration

Enable the module and configure rewrite rules at:

`/admin/config/islandora/microservice-rewrite`

Enter one rule per line in the format:

```text
find|replace
```

Each rule is applied with `str_replace()`, in order, to the supported URI fields in the generated derivative message.

Blank lines are ignored. Malformed lines without a `|` separator are ignored.

### Example Rules

Rewrite a public Drupal hostname to an internal service hostname:

```text
https://repository.example.edu|http://drupal.internal
```

Rewrite multiple hostnames in the same environment:

```text
https://repository.example.edu|http://drupal.internal
https://preserve.example.edu|http://preserve.internal
```

Rewrite a more specific path:

```text
https://repository.example.edu/_flysystem/fedora|http://nginx/_flysystem/fedora
```

### When To Use This Module

Use this module when:

- Drupal generates derivative message URIs using a public hostname
- downstream Islandora services cannot resolve or reach that hostname
- your microservice network needs internal-only hostnames or alternate paths

Do not enable it unless your deployment actually needs URI rewriting.

## Development

If you would like to contribute, please get involved by attending our weekly [Tech Call](https://github.com/Islandora/documentation/wiki). We love to hear from you!

If you would like to contribute code to the project, you need to be covered by an Islandora Foundation [Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_cla.pdf) or [Corporate Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_ccla.pdf). Please see the [Contributors](http://islandora.ca/resources/contributors) pages on Islandora.ca for more information.

We recommend using the [islandora-playbook](https://github.com/Islandora-Devops/islandora-playbook) to get started.

## License

[GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)
