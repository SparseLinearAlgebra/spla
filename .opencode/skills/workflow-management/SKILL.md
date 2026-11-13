---
name: workflow-management
description: Load first at the start of every session, before doing anything else. Drives the overall working loop for a task or set of tasks on an integration branch (main or a personal branch): global plan, one task at a time, feature branch, detailed plan, subtask execution, code review, quality gate, integration, final pull request.
---

# Workflow Management

The top-level loop that orchestrates all other workflow skills. For a
single task, load this skill first; it points to the skills that handle each
step. Project-specific facts (integration branch config key, commit format,
doc locations) are resolved through the hooks declared in the project facts
contract of `AGENTS.md`.

## Core Rules

- Log all tasks as GitHub issues with the `task` label (`gh issue create
  --label task`). Use the description exactly as the user provided it — minimal
  changes, only splitting and numbering. The task ID is the issue number.
- A batch of **related** tasks is tracked by a single **hub** issue (label
  `hub`). Hubs carry the global plan and are never chosen as tasks; task issues
  keep the `task` label and are attached to the hub as GitHub **sub-issues**
  through the REST API (`POST repos/$REPO/issues/<HUB>/sub_issues`). Listing the
  task numbers inside the hub body is **not** a substitute for the sub-issue
  relationship.
- The global plan for the active batch lives in `tasks/global_plan.md` (single
  source of truth) and is mirrored to the hub issue as a comment whose first
  line is `<!-- global-plan -->`. Task progress is marked in the file and the
  mirror is updated after each task integrates, exactly as the detailed plan is
  mirrored to a task issue (see the `subtask-loop` skill).
- Resolve the **integration branch** once per session using the git config key
  defined in the *contribution guidelines* hook. It is `main` in direct mode
  and a personal, long-lived branch in stacked mode. Never commit directly to
  the integration branch.
- Do tasks strictly **one at a time**. Each task gets its own feature branch
  (created from the integration branch), its own detailed plan, and its own
  integration step. Never combine multiple tasks in a single feature branch.
- In direct mode a task is integrated into `main` through its own pull
  request. In stacked mode a task is rebased and fast-forward merged into the
  integration branch and **no pull request is opened**; a single pull request
  into `main` is opened only on the user's explicit request.
- Each decision must be documented before implementation. Documentation must
  be detailed enough to reproduce the project from scratch and understand why
  each decision was made.
- Commit messages must follow the format defined in the *contribution
  guidelines* hook and be detailed enough to understand the reasons for
  changes.
- Documentation-only tasks (no source code files changed) skip code-specific
  gates (tests, lint, format) but still run the docs build — for documentation
  changes it is the relevant gate — and follow all other workflow rules:
  one task per branch, one commit per subtask, code review.

## Working Loop

0. If the user requests multiple related tasks at once, set up the batch before
   any implementation:
   a. Create the **hub** issue (`gh issue create --label hub`); it tracks the
      batch and is not itself a task.
   b. Create one issue per task (`gh issue create --label task`), then attach
      each task issue to the hub as a GitHub **sub-issue** (mandatory, not a
      body link):

      ```bash
      REPO=$(gh repo view --json nameWithOwner -q .nameWithOwner)
      CHILD_ID=$(gh api "repos/$REPO/issues/<N>" --jq .id)
      gh api -X POST "repos/$REPO/issues/<HUB>/sub_issues" -F sub_issue_id="$CHILD_ID"
      ```

      Verify the parent: `gh api "repos/$REPO/issues/<N>/parent" --jq .number`
      must print the hub number.
   c. Write the global plan in `tasks/global_plan.md` (see the `planning`
      skill), naming the hub and listing each task with its issue number.
   d. Post the global plan as a comment on the hub whose first line is
      `<!-- global-plan -->` (a pure mirror of the file). A related task added
      later is attached to the same hub and added to the plan.
1. Ensure user-defined tasks, the global plan, and project architecture are
   aligned.
2. Choose exactly ONE open `task`-labeled issue that is not yet done: a task
   is done when its subtask commits are on the integration branch
   (`git log "$INTEGRATION" --format=%s | grep -cE '\(<N>-S[0-9]+\):'` > 0,
   where N is the issue number). List candidates with
   `gh issue list --label task --state open`. Hubs carry `hub`, not `task`, so
   this list never returns them.
3. Rebase the integration branch onto `origin/main` (task-boundary refresh, see
   `git-workflow`), then create a feature branch from the integration branch for
   this single task (branching model: *contribution guidelines*; procedure:
   `git-workflow`).
4. Generate a detailed plan in `tasks/detailed_plan.md`, decomposing the task
   into atomic subtasks (see `planning`), then post it as a comment on the
   task issue; the first line of the comment is the marker
   `<!-- detailed-plan -->`.
5. Load the `subtask-loop` skill, then execute each subtask using its cycle.
5a. Verify all subtasks are complete and unblocked. Check
    `tasks/detailed_plan.md`:
    - If any subtask is marked `[blocked]` or `[deferred]`, STOP immediately.
      The task is NOT complete. Do NOT proceed to code review or the PR.
      Report blocking subtasks to the user and await guidance.
    - If a subtask was attempted, not committed, and its work reverted, the
      subtask is NOT complete. Do not silently skip it.
5b. When continuing a partially-done task, analyze current state before any
    code changes:
    - Verify `git branch --show-current` is the correct feature branch.
    - Review committed subtasks: `git log --oneline` on the feature branch.
    - Read `tasks/detailed_plan.md` and `tasks/global_plan.md`.
    - Cross-reference committed file changes with planned subtasks:
      `git diff --stat HEAD.."$INTEGRATION"`.
    - Report status to the user: "S1-S3 committed, S4 pending, ..."
6. After all subtasks are done, perform code review on the entire repo (see
   the `code-review` skill). Iteratively detect and fix problems until zero
   findings.
7. Load the `quality-gates` skill and run the gate. It must show PASS. If
   BLOCKED, do not assess whether failures are pre-existing or unrelated to
   your changes; fix every failure and re-run until PASS.
8. Integrate the task (see `git-workflow`):
   - **Direct mode** (`$INTEGRATION` is `main`): push the feature branch and
     open a pull request into `main`, merged with rebase and merge
     (fast-forward) so the per-subtask commits are preserved and `main` stays
     linear. The user reviews and merges the PR manually; proceed only after
     they confirm the merge.
   - **Stacked mode** (`$INTEGRATION` is a personal branch): rebase and
     fast-forward merge the feature branch into `$INTEGRATION`, then delete the
     feature branch. Do **not** push for a PR and do **not** open a PR.
9. For a batch, update its progress: mark the integrated task `[done #<N>]` in
   `tasks/global_plan.md`, then mirror the file to the hub's global-plan
   comment using the same procedure as the detailed plan (step 7 of the
   `subtask-loop` skill), substituting the hub number for the task number and
   the `<!-- global-plan -->` marker for `<!-- detailed-plan -->`.
10. Verify the last subtask's commit carries `Closes #<N>` (the task's own
    issue) as a standalone line — the issue closes when the commit reaches
    `main`. For the last task of a batch, the same commit also carries
    `Closes #<hub>` once every sub-issue is resolved, closing the hub when the
    commit reaches `main`. See the Task Completeness Verification in the
    `subtask-loop` skill (the single source of truth for what "done" means).
11. Return to step 2 for the next task. In stacked mode, keep accumulating
    completed tasks on `$INTEGRATION`; open the request to `main` only via the
    finalize step below, on the user's explicit request.

## Finalize to main (stacked mode, explicit request only)

When — and only when — the user explicitly asks to open the request to `main`:

1. Rebase `$INTEGRATION` onto `origin/main` (see `git-workflow`) and re-run the
   aggregated code review and quality gate over the whole `main...integration`
   diff (see `code-review` and `quality-gates`).
2. Push `$INTEGRATION` and open one pull request with base `main` and head
   `$INTEGRATION` (see `git-workflow`).
3. The user reviews and merges the PR with rebase and merge. After the merge,
   sync `main`; never delete the long-lived integration branch.
