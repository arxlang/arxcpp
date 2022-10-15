<!-- Describe what this PR aims to resolve. E.g. This PR aims to ...  -->

<!-- Which issue this PR aims to resolve or fix. E.g. This PR resolves #... -->

# PR Checklist

Ensure that:

- it includes tests.
- the tests for your implementation are executed on CI.
- it includes documentation
- the PR's title is according to the [semantic-release pattern][semantic-release-message].
  - `build`: Changes that affect the build system or external dependencies (example scopes: cmake, meson, etc)
  - `ci`: Changes to our CI configuration files and scripts (examples: CircleCi, SauceLabs)
  - `docs`: Documentation only changes
  - `perf`: A code change that improves performance
  - `refactor`: A code change that neither fixes a bug nor adds a feature
  - `test`: Adding missing tests or correcting existing tests
  - `chore:` Can be used as a generic task for tasks such as CI, test, support tasks or any other task that is not user facing task.
  - `feat:` is used for a new feature or when a existent one is improved.
  - `fix:` is used when a bug is fixed.
  - `BREAKING CHANGE:` is used when there is a compatibility break.
- pre-commit hooks were executed locally


<!-- Add any other extra information that would help to understand the changes proposed by the PR -->


[semantic-release-message]: https://github.com/semantic-release/semantic-release/blob/master/README.md#commit-message-format "Semantic Release Commit Message Format"
