# Islandora IIIF

[![Minimum PHP Version](https://img.shields.io/badge/php-%3E%3D%207.2-8892BF.svg?style=flat-square)](https://php.net/)
[![Contribution Guidelines](http://img.shields.io/badge/CONTRIBUTING-Guidelines-blue.svg)](./CONTRIBUTING.md)
[![LICENSE](https://img.shields.io/badge/license-GPLv2-blue.svg?style=flat-square)](./LICENSE)

## Introduction

Provides [IIIF manifests](https://iiif.io) using views.

## Requirements

- `islandora` and `islandora_core_feature`
- A IIIF image server (such as Cantaloupe)

## Installation

For a full digital repository solution, see our [installation documentation](https://islandora.github.io/documentation/installation/).

To download/enable just this module, use the following from the command line:

```bash
$ composer require drupal/islandora
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
3. Structured Text Term: If your Islandora Object has a separate media with hOCR, point to it with this setting.
4. File Width and Height fields: If you want to use TIFF or JP2 files directly, set the fields where the files' width and height can be found with these settings.

### Action to add image dimensions from the IIIF server

The module also provides an action that lets a site owner populate a TIFF or JP2 image's width and
height attributes into fields on the media so the IIIF server is not bogged down trying to generate a manifest if
it doesn't have them.

It is an advanced action that must be configured. Go to
Admin -> Actions UI -> Actions, choose
"Add image dimensions retrieved from the IIIF server" from the Create Action drop-down
And on  the next screen, choose the Media Use term which this should be applied to (as
it is a node action), as well as the width
and height fields that the action should
populate.

To use it, either:
- Add it as a derivative reaction (in Contexts) to a node, or
- Use it as a batch action in a View, such as on a Paged Content object's list of child pages.

### Setting up the action as a Contexts Derivative Reaction

These instructions assume a standard Islandora Starter Site, so if you have different field names or Contexts, adjust accordingly.

1. Go to admin/config/system/actions
2. Choose "Add image dimensions retrieved from the IIIF server" from the Create advanced Action drop-down and then click Create.
3. Enter Original File for the Source Media Use term.
4. Choose media -- file -- Width and Media -- File -- Height for the corresponding configuration values and click Save.
5. Go to admin/structure/context, and click Duplicate on the Page Derivatives row.
6. Name the new context something like "Retrieve Page Dimensions" and edit it.
7. This is the tricky bit, delete 'Original File' from the 'Media has term with URI' field and replace it with Service File. The explanation for this is that to retrieve a file from the IIIF server, it must be part of an Islandora Media that has been fully created and saved and given a URL. This hasn't happened yet when Original File derivatives are being created, so we need to hang our action onto a derivative that is created after the original one.
8. Under Reactions, deselect the existing actions and select  "Add image dimensions from IIIF server" and click Save and continue.
9. Go back to your Paged Content object and add another child with a File media, to which you should upload another TIFF or JP2 file.
10. Without going  to the Original File Book Manifestt, make sure that a service file has been generated, then click Edit on the Original File media.
11. Ensure that the Width and Height fields are populated with the correct values based on the file.


## Documentation

Official documentation is available on the [Islandora 2 documentation site](https://islandora.github.io/documentation/).

## Development

If you would like to contribute, please get involved by attending our weekly [Tech Call](https://github.com/Islandora/documentation/wiki). We love to hear from you!

If you would like to contribute code to the project, you need to be covered by an Islandora Foundation [Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_cla.pdf) or [Corporate Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_ccla.pdf). Please see the [Contributors](http://islandora.ca/resources/contributors) pages on Islandora.ca for more information.

## License

[GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)
