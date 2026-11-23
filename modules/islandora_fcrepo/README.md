# Islandora Fedora Repository

Optional Fedora integration for Islandora 3.x on Drupal 11. This module owns the
Flysystem adapter and driver, JWT-authenticated Fedora requests, file versioning
and tombstones, Fedora targets and file URIs in event messages, Fedora URI
display, download URL compatibility, and Fedora indexing action configuration.

Install the dependencies listed in this directory's composer.json into the site
root, then enable `islandora_fcrepo`. See [the upgrade guide](../../UPGRADE-3.md)
for commands and migration behavior. Installing core Islandora alone does not
install these dependencies. Keep Composer security blocking enabled.

Existing settings remain valid, for example:

```php
$settings['flysystem']['fcrepo'] = [
  'driver' => 'fedora',
  'config' => [
    'root' => 'http://fedora:8080/fcrepo/rest/',
    'public' => TRUE,
  ],
];
```

Use your existing scheme (`fedora`, `fcrepo`, or a custom alias); do not change
existing file URIs. `config.public` retains the previous Drupal file access
behavior. Fedora's JWT authorization and Drupal's file download hooks still
apply. The module does not make Fedora's HTTP endpoint publicly accessible.

Fedora downloads keep their `/_flysystem/{scheme}/{path}` URLs. GET requests use streaming responses, and the adapter returns the native stream
for the complete object, allowing Drupal's BinaryFileResponse and the Flysystem stream
wrapper to process byte ranges without truncating copies or ordinary reads.

Core-feature defaults use public storage. Sites choosing Fedora for new fields
or derivative actions must select their Fedora scheme explicitly. Existing
field and action configurations are preserved.

The module's tests use mocked Fedora API responses and native non-seekable
streams. They cover the Flysystem 3 adapter contract, download access denials,
HTTP byte ranges, malformed UTF-8 paths, plugin discovery, URL compatibility,
and migration of existing settings. They do not require a running Fedora server.
