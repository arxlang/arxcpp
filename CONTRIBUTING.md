# Contributing

Contributions are welcome, and they are greatly appreciated! Every
little bit helps, and credit will always be given.

You can contribute in many ways:

## Types of Contributions

### Report Bugs

Report bugs at https://github.com/arxlang/arx/issues.

If you are reporting a bug, please include:

  - Your operating system name and version.
  - Any details about your local setup that might be helpful in
    troubleshooting.
  - Detailed steps to reproduce the bug.

### Fix Bugs

Look through the GitHub issues for bugs. Anything tagged with “bug” and
“help wanted” is open to whoever wants to implement it.

### Implement Features

Look through the GitHub issues for features. Anything tagged with
“enhancement” and “help wanted” is open to whoever wants to implement
it.

### Write Documentation

arx could always use more documentation,
whether as part of the official arx docs,
in docstrings, or even on the web in blog posts, articles, and such.

### Submit Feedback

The best way to send feedback is to file an issue at
https://github.com/arxlang/arx/issues.

If you are proposing a feature:

  - Explain in detail how it would work.
  - Keep the scope as narrow as possible, to make it easier to
    implement.
  - Remember that this is a volunteer-driven project, and that
    contributions are welcome :)

## Get Started!

Ready to contribute? Here’s how to set up `arx` for local development.

1.  Fork the `arx` repo on GitHub.

2.  Clone your fork locally:
```bash
$ git clone git@github.com:your_name_here/arx.git
```

3.  Prepare your local development environment:
```bash
$ mamba env create --file /conda/dev.yaml
$ conda activate arx
$ pre-commit install
```

4.  Create a branch for local development:
```bash
$ git checkout -b name-of-your-bugfix-or-feature
```
Now you can make your changes locally.

5.  When you’re done making changes, check the compilation and the tests:

```bash
$ makim build.dev
$ makim tests.all
$ pre-commit run --all-files
```

Note: if you want to remove all the build folder before starting to build
you can run `makim build.dev --clean`.

6.  Commit your changes and push your branch to GitHub:
```bash
$ git add .
$ git commit -m “Your detailed description of your changes.”
$ git push origin name-of-your-bugfix-or-feature
```

7.  Submit a pull request through the GitHub website.

## Pull Request Guidelines

Before you submit a pull request, check that it meets these guidelines:

1.  The pull request should include tests.
2.  If the pull request adds functionality, the docs should be updated.
    Put your new functionality into a function with a docstring, and add
    the feature to the list in README.md.


## Containers

If you want to play with Arx inside a container, there is
`docker-compose` file and a `Dockerfile` prepared for that.

First, create the conda environment using the following command:

```bash
$ mamba env create --file conda/containers.yaml
```

And activate the new environment:

```bash
$ conda activate arx-containers
```

Now, you can build and run the new container for playing with arx:
commands:

```bash
containers-sugar build
containers-sugar run
```

Inside the container you can run the same makim targets, for example:

```bash
makim build.dev --clean
```

**Note:** For development, remember to install pre-commit hooks:

```bash
$ pre-commit install
```

## Conventional naming

In order to keep a similar conventional name used by arxlang, for c++ code
we are using snake case naming conventing for variables and functions
and camel case for classes.

## Release

This project uses semantic-release in order to cut a new release
based on the commit-message.

### Commit message format

**semantic-release** uses the commit messages to determine the consumer
impact of changes in the codebase. Following formalized conventions for
commit messages, **semantic-release** automatically determines the next
[semantic version](https://semver.org) number, generates a changelog and
publishes the release.

The convention used for the PR title check follows
[Conventional Commits](https://www.conventionalcommits.org).

So, the PR title should use the following prefixes:
  * `build`: Changes that affect the build system or external dependencies (example scopes: cmake, meson, etc)
  * `ci`: Changes to our CI configuration files and scripts (examples: CircleCi, SauceLabs)
  * `docs`: Documentation only changes
  * `perf`: A code change that improves performance
  * `refactor`: A code change that neither fixes a bug nor adds a feature
  * `test`: Adding missing tests or correcting existing tests
  * `chore`: Can be used as a generic task for tasks such as CI, test, support tasks or any other task that is not user facing task.
  * `feat`: is used for a new feature or when a existent one is improved.
  * `fix`: is used when a bug is fixed.
  * `fix!` or `feat!`: is used when there is a compatibility break.

The table below shows which commit message gets you which release type
when `semantic-release` runs (using the default configuration):

| Commit message                                                 | Release type     |
|----------------------------------------------------------------|------------------|
| `fix(pencil): stop graphite breaking when pressure is applied` | Fix Release      |
| `feat(pencil): add 'graphiteWidth' option`                     | Feature Release  |
| `perf(pencil): remove graphiteWidth option`                    | Chore            |
| `fix(pencil)!: The graphiteWidth option has been removed`      | Breaking Release |

source:
<https://github.com/semantic-release/semantic-release/blob/master/README.md#commit-message-format>

As this project uses the `squash and merge` strategy, ensure to apply
the commit message format to the PR's title.
