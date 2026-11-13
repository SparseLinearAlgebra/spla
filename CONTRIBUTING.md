# Spla project contributing guide

Thank you very much if you have decided to contribute to our project. We follow very simple and clear open-source
research community accepted guidelines for contributing. The guideline instructions divided into sections depending on
the part of the project you want to contribute.

## Contribution guidelines

The contribution model below applies to contributors working directly on this repository (without forking). External
contributors follow the fork-based guides further down.

### Branching model

- `main` is the default **integration branch**. All work happens on a feature branch created from the integration
  branch; never commit directly to the integration branch.
- A developer may redirect the integration branch to a personal, long-lived branch (stacked mode) with:

  ```shell
  $ git config spla.integrationBranch <branch>
  ```

  In stacked mode, completed tasks are rebased and fast-forward merged into the personal branch; a single pull request
  into `main` is opened only on explicit request. Without the config, direct mode applies: each task is integrated
  into `main` through its own pull request.

### Commit message format

- One commit per completed atomic subtask of a task.
- Subject: `(<N>-S<n>): <summary>`, where `<N>` is the task issue number and `<n>` the subtask number
  (e.g., `(236-S1): add kron product unit test`). Ranges, lists, or multiple subtask identifiers in one commit are
  forbidden.
- The last subtask's commit carries the closing keywords as standalone lines: `Closes #<N>` for the task's own issue
  plus `Fixes #M` (defects) or `Closes #M` otherwise for each linked issue fully resolved. An issue is closed when the
  carrying commit reaches `main`.

### Pull requests

- Use the [pull request template](./.github/PULL_REQUEST_TEMPLATE.md).
- Merge with **rebase and merge** (fast-forward), not a merge commit and not squash — this keeps `main` linear while
  preserving the per-subtask commits the message format is built around.

## Source code developers guide (1)

1. Fork this repository using your GitHub account.
2. Install `git` and clone your forked copy of the `repo`.
3. Build project following build instructions in [README.md](./README.md) file, make sure everything is ok.
4. Run unit-tests following instructions in [README.md](./README.md) file, make sure all tests passing.
5. Implement new feature or fix existing one in the C/C++ source code.
6. Commit your changes.
7. Open a pull-request following repository template rules.
8. Wait for review from developers of the project.
9. Fix major and minor issues if presented.
10. Get your work merged into `main`!

## Documentation improvements guide (2)

1. Fork this repository using your GitHub account.
2. Install `git` and clone your forked copy of the `repo`.
3. Edit code documentation in source files and/or in [docs](./docs) folder files.
4. Commit your changes.
5. Open a pull-request following repository template rules.
6. Wait for review from developers of the project.
7. Fix major and minor issues if presented.
8. Get your work merged into `main`!

## Useful links

If you want to fix something, ask a question, add new functionality or feature, have a look at our templates.

- **Issue templates**:
  [https://github.com/SparseLinearAlgebra/spla/tree/main/.github/ISSUE_TEMPLATE](https://github.com/SparseLinearAlgebra/spla/tree/main/.github/ISSUE_TEMPLATE)
- **Pull-request templates**:
  [https://github.com/SparseLinearAlgebra/spla/tree/main/.github/PULL_REQUEST_TEMPLATE](https://github.com/SparseLinearAlgebra/spla/tree/main/.github/PULL_REQUEST_TEMPLATE)
