# Islandora FITS

Provides actions to extract and store technical metadata using a FITS microservice (CrayFits).

## Requirements

- `islandora` and `islandora_core_feature`
- A CrayFits microservice
- A message broker (e.g. Activemq) for Islandora
- An [Alpaca](https://github.com/Islandora/Alpaca/tree/2.x/islandora-connector-derivative) `islandora-connector-derivative` configured for CrayFits. The default configuration in this module assumes the queue is named `islandora-connector-fits`.

## Installation

For a full digital repository solution (including CrayFits), see our [installation documentation](https://islandora.github.io/documentation/installation/).

To download/enable just this module, use the following from the command line:

```bash
$ composer require islandora/islandora
$ drush en islandora_core_feature
$ drush mim islandora_tags
$ drush en islandora_fits
$ drush mim islandora_fits_tags
```

### Configuration installed automatically

On installation this module will:

* Create a new media type, "FITS Technical metadata."
* Create a new Action, "FITS - Generate a Technical metadata derivative", based on the FITS
Action plugin, "Generate a Technical metadata derivative," provided by this module.
* Add a Context, "Technical Metadata on Ingest", which causes FITS derivatives to be created
from media tagged "Original File".

To create FITS derivatives, the Context requires the existence of an `islandora_media_use` term with
an external URI of `https://projects.iq.harvard.edu/fits`. The `islandora_fits_tags`
migration will create such a term.

### File Checksum pseudo field

A "File Checksum" pseudo field can be added to the display of nodes. It contains the MD5 checksum
stored in a "FITS Technical metadata"-type media that is attached to that node. In the default
configuration, this value is the MD5 checksum of the file tagged "Original File".

## Documentation

Further documentation for this module is available on the [Islandora documentation site](https://islandora.github.io/documentation/).

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Sponsors

* UPEI

## Development

If you would like to contribute, please get involved by attending our weekly [Tech Call](https://github.com/Islandora/islandora-community/wiki/Weekly-Open-Tech-Call). We love to hear from you!

If you would like to contribute code to the project, you need to be covered by an Islandora Foundation [Contributor License Agreement](https://github.com/Islandora/islandora-community/wiki/Onboarding-Checklist#contributor-license-agreements) or [Corporate Contributor License Agreement](https://github.com/Islandora/islandora-community/wiki/Onboarding-Checklist#contributor-license-agreements). Please see the [Contributor License Agreements](https://github.com/Islandora/islandora-community/wiki/Contributor-License-Agreements) page on the islandora-community wiki for more information.

We recommend using the [islandora-playbook](https://github.com/Islandora-Devops/islandora-playbook) or [isle-dc](https://github.com/Islandora-Devops/isle-dc) to get started.

## License

[GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)
