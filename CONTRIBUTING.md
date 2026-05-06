# Welcome!

If you are reading this document then you are interested in contributing to Islandora 8. All contributions are welcome: use-cases, documentation, code, patches, bug reports, feature requests, etc. You do not need to be a programmer to speak up!

We also have an IRC channel -- #islandora -- on freenode.net. Feel free to hang out there, ask questions, and help others out if you can.

Please note that this project operates under the [Islandora Community Code of Conduct](http://islandora.ca/codeofconduct). By participating in this project you agree to abide by its terms.

## Workflows

The Islandora 8 Tech Call meets each Wednesday at 1:00 PM Eastern, and the Islandora 8 User Call meets every second Thursday at 1:00 PM Eastern. Meeting notes and announcements are posted to the [Islandora community list](https://groups.google.com/forum/#!forum/islandora) and the [Islandora developers list](https://groups.google.com/forum/#!forum/islandora-dev). You can view meeting agendas, notes, and call-in information [here](https://github.com/Islandora/documentation/wiki#islandora-8-tech-calls). Anybody is welcome to join the calls, and add items to the agenda.

### Use cases

If you would like to submit a use case to the Islandora 8 project, please submit an issue [here](https://github.com/Islandora/documentation/issues/new) using the [Use Case template](https://github.com/Islandora/documentation/wiki/Use-Case-template), prepending "Use Case:" to the title of the issue.

### Documentation

You can contribute documentation in two different ways. One way is to create an issue [here](https://github.com/Islandora/documentation/issues/new), prepending "Documentation:" to the title of the issue. Another way is by pull request, which is the same process as [Contribute Code](https://github.com/Islandora/documentation/blob/master/CONTRIBUTING.md#contribute-code). All documentation resides in [`docs`](https://github.com/Islandora/documentation/tree/master/docs).

### Request a new feature

To request a new feature you should [open an issue in the Islandora 8 repository](https://github.com/Islandora/documentation/issues/new) or create a use case (see the _Use cases_ section above), and summarize the desired functionality. Prepend "Enhancement:" if creating an issue on the project repo, and "Use Case:" if creating a use case.

### Report a bug

To report a bug you should [open an issue in the Islandora 8 repository](https://github.com/Islandora/documentation/issues/new) that summarizes the bug. Prepend the label "Bug:" to the title of the issue.

In order to help us understand and fix the bug it would be great if you could provide us with:

1. The steps to reproduce the bug. This includes information about e.g. the Islandora version you were using along with the versions of stack components.
2. The expected behavior.
3. The actual, incorrect behavior.

Feel free to search the issue queue for existing issues (aka tickets) that already describe the problem; if there is such a ticket please add your information as a comment.

**If you want to provide a pull along with your bug report:**

That is great! In this case please send us a pull request as described in the section _Create a pull request_  below.

### Contribute code

Before you set out to contribute code you will need to have completed a [Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_cla.pdf) or be covered by a [Corporate Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_ccla.pdf). The signed copy of the license agreement should be sent to <mailto:community@islandora.ca>

_If you are interested in contributing code to Islandora but do not know where to begin:_

In this case you should [browse open issues](https://github.com/Islandora/documentation/issues) and check out [use cases](https://github.com/Islandora/documentation/labels/use%20case).

If you are contributing Drupal code, it must adhere to [Drupal Coding Standards](https://www.drupal.org/coding-standards); Travis CI will check for this on pull requests.

Contributions to the Islandora codebase should be sent as GitHub pull requests. See section _Create a pull request_ below for details. If there is any problem with the pull request we can work through it using the commenting features of GitHub.

* For _small patches_, feel free to submit pull requests directly for those patches.
* For _larger code contributions_, please use the following process. The idea behind this process is to prevent any wasted work and catch design issues early on.

    1. [Open an issue](https://github.com/Islandora/documentation/issues), prepending "Enhancement:" in the title if a similar issue does not exist already. If a similar issue does exist, then you may consider participating in the work on the existing issue.
    2. Comment on the issue with your plan for implementing the issue. Explain what pieces of the codebase you are going to touch and how everything is going to fit together.
    3. Islandora committers will work with you on the design to make sure you are on the right track.
    4. Implement your issue, create a pull request (see below), and iterate from there.

### Create a pull request

Take a look at [Creating a pull request](https://help.github.com/articles/creating-a-pull-request). In a nutshell you need to:

1. [Fork](https://help.github.com/articles/fork-a-repo) this repository to your personal or institutional GitHub account (depending on the CLA you are working under). Be cautious of which branches you work from though (you'll want to base your work off the default branch). See [Fork a repo](https://help.github.com/articles/fork-a-repo) for detailed instructions.
2. Commit any changes to your fork.
3. Send a [pull request](https://help.github.com/articles/creating-a-pull-request) using the [pull request template](https://github.com/Islandora/documentation/blob/master/.github/PULL_REQUEST_TEMPLATE.md) to the Islandora GitHub repository that you forked in step 1.  If your pull request is related to an existing issue -- for instance, because you reported a [bug/issue](https://github.com/Islandora/documentation/issues) earlier -- prefix the title of your pull request with the corresponding issue number (e.g. `issue-123: ...`). Please also include a reference to the issue in the description of the pull. This can be done by using '#' plus the issue number like so '#123', also try to pick an appropriate name for the branch in which you're issuing the pull request from.

You may want to read [Syncing a fork](https://help.github.com/articles/syncing-a-fork) for instructions on how to keep your fork up to date with the latest changes of the upstream (official) repository.

## Testing locally

The CI tests that run on pull requests in GitHub leverage the docker images built in [islandora/islandora_ci](https://github.com/Islandora/islandora_ci). The CI tests a wide range of PHP and Drupal version combinations. You can run these same tests locally if you have docker installed.

Set which Drupal and PHP version you want to test and setup the docker network

```bash
DRUPAL_VERSION=11.3
PHP_VERSION=8.3
docker network create ci-default
```

Optionally, if you want to run a specific test suite (e.g. `kernel`), provide it via the `TEST_SUITE` environment variable (e.g. `TEST_SUITE=kernel`). Leave the value blank if you want to run all tests.

Also optional, if you're running functional javascript tests, start a chromedriver docker container

```bash
docker run -d \
    --rm \
    --name chromedriver \
    --network ci-default \
    drupalci/webdriver-chromedriver:production \
    "chromedriver --log-path=/dev/null --verbose --allowed-ips= --allowed-origins=*"
```

If you're running functional tests, start an activemq docker container

```bash
docker run -d --name activemq --network ci-default webcenter/activemq:5.14.3
```

Run the tests

```bash
docker run \
    --name drupal-ci-$DRUPAL_VERSION-$PHP_VERSION \
    --hostname drupal \
    --rm \
    --volume $(pwd):/var/www/drupal/web/modules/contrib/islandora:ro \
    --env ENABLE_MODULES=islandora \
    --env TEST_SUITE="${TEST_SUITE:-}" \
    --network ci-default \
    ghcr.io/islandora/ci:$DRUPAL_VERSION-php$PHP_VERSION
```

## License Agreements

The Islandora Foundation requires that contributors complete a [Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_cla.pdf) or be covered by a [Corporate Contributor License Agreement](http://islandora.ca/sites/default/files/islandora_ccla.pdf). The signed copy of the license agreement should be sent to <a href="mailto:community@islandora.ca?Subject=Contributor%20License%20Agreement" target="_top">community@islandora.ca</a>. This license is for your protection as a contributor as well as the protection of the Foundation and its users; it does not change your rights to use your own contributions for any other purpose. A list of current CLAs is kept [here](https://github.com/Islandora/islandora/wiki/Contributor-License-Agreements).
