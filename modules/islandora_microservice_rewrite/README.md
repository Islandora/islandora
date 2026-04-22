# Islandora Microservice Rewrite

`islandora_microservice_rewrite` is an optional Islandora submodule that
rewrites selected URI fields in generated derivative messages before those
messages are serialized and sent to downstream microservices.

This is intended for split-host or containerized deployments where the public
Drupal URL is not the right URL for internal services such as Alpaca or
derivative processors. In those environments, derivative messages may need to
use an internal hostname, proxy address, or alternate service path that is
reachable from the microservice network.

## What It Rewrites

When enabled, the module listens to Islandora's generated-message event and
applies simple string replacements to these attachment content fields when they
are present:

- `source_uri`
- `destination_uri`
- `file_upload_uri`

## Configuration

Enable the module and configure rewrite rules at:

`/admin/config/islandora/microservice-rewrite`

Enter one rule per line in the format:

```text
find|replace
```

Each rule is applied with `str_replace()`, in order, to the supported URI
fields in the generated derivative message.

Blank lines are ignored. Malformed lines without a `|` separator are ignored.

## Example Rules

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

## When To Use This Module

Use this module when:

- Drupal generates derivative message URIs using a public hostname
- downstream Islandora services cannot resolve or reach that hostname
- your microservice network needs internal-only hostnames or alternate paths

Do not enable it unless your deployment actually needs URI rewriting.

## Manual Verification

1. Enable `islandora_microservice_rewrite`.
2. Go to `/admin/config/islandora/microservice-rewrite`.
3. Save one or more rewrite rules.
4. Trigger a derivative-generating action from the Drupal UI.
5. Confirm the derivative flow now works with the rewritten internal URI path.

The main implementation branch intentionally does not include diagnostic logging.
For teammate QA, create a separate follow-up commit on top of the feature branch
that injects `logger.channel.islandora` into the subscriber and writes rewritten
values to `/admin/reports/dblog`. Keep that logging commit out of the upstream
PR.
