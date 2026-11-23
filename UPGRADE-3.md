# Upgrading to Islandora 3.x

This development branch requires Drupal 11 and PHP 8.3+. Upgrade Drupal before
moving an existing Islandora 2.x installation to 3.x. Back up the database,
configuration, and file storage and validate the upgrade on a copy of the site.

## Sites without Fedora

Core Islandora no longer requires Flysystem, Chullo, or the Fedora entity mapper.
It uses Drupal's registered writable storage schemes. New core-feature file
fields and OCR actions default to `public://`. Existing field settings, actions,
and file URIs are not rewritten.

Update Islandora and run `drush updb -y` and `drush cr`. The post-update hook checks
`file_managed` for `fcrepo://`, the historical `fedora://`, and configured Fedora
scheme aliases. If none are used, it returns without enabling `islandora_fcrepo`.

Composer does not automatically remove requirements you added to your site's
root composer.json. If you no longer use Flysystem or Fedora, review
`composer why drupal/flysystem` and remove obsolete root requirements as part of
the upgrade. Do not disable advisory blocking to retain Flysystem 1.x. Do not
uninstall an enabled storage module until you have confirmed that no file fields,
files, or other modules rely on it.

## Sites using Fedora

Fedora integration now lives in the optional `islandora_fcrepo` module bundled
with Islandora. Its nested composer.json documents its dependencies; Composer
does not automatically install nested submodule requirements. Add them to the
site root alongside the Islandora upgrade:

```sh
composer require 'drupal/flysystem:^3.0@beta' 'league/flysystem:^3.35.3' \
  'islandora/chullo:^2.0' 'islandora/fedora-entity-mapper:^1.0' -W
```

Resolve these requirements together with the Islandora 3.x requirement; the
old Islandora 2.x constraint is incompatible with Flysystem 3. Flysystem's Drupal
module is currently beta software. Test your existing storage and download
workflows before deploying.

Keep the existing `$settings['flysystem']` scheme names, Fedora root URLs, and JWT
configuration. The driver remains `fedora`; there is no binary migration or URI
rename. Run `drush updb -y` and `drush cr`. Existing Fedora file URIs cause the
post-update hook to enable `islandora_fcrepo`. If its dependencies are missing, the
hook fails with installation instructions rather than marking the upgrade done.

Sites using Fedora only for indexing, or configured for future Fedora uploads
but without existing Fedora file URIs, must explicitly run:

```sh
drush en islandora_fcrepo -y
drush cr
```

Fedora URI display settings move to `/admin/config/islandora/fcrepo`; existing
bundle selections are copied when the module is installed. Existing
`/_flysystem/{scheme}/...` URLs remain supported. Fedora indexing actions are now
provided by the optional module; existing action identifiers are retained.

Review custom modules using `Drupal\islandora\Flysystem` classes or the
`islandora.entity_mapper` service. The new adapter/plugin namespace is
`Drupal\islandora_fcrepo`, and the mapper service is
`islandora_fcrepo.entity_mapper`. The adapter implements Flysystem 3's exception
and metadata contracts. There is no compatibility shim for the Flysystem 1 API.

If Flysystem 2 is currently enabled even though you have no Fedora files, do not
let Composer remove its code while Drupal still lists it as installed. Upgrade
it to Flysystem 3 with Islandora first, then review other storage consumers,
uninstall the unused Flysystem module, and remove its root Composer requirement.
The automatic post-update hook does not uninstall storage providers.
