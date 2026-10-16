---
name: subtask-loop
description: Use when executing an atomic subtask from the detailed plan: implement -> test -> document -> pre-commit checks -> quality checks -> commit -> mark done. Covers the full execution cycle, code quality checks, commit rules, blocked work protocol, and completion tracking.
---

# Subtask Execution Loop

Each atomic subtask from `tasks/detailed_plan.md` is executed as a
self-contained cycle. Project-specific facts (commit format, integration
branch) are resolved through the hooks declared in the project facts contract
of `AGENTS.md`.

## Cycle Steps

Execute these steps in order. **Do not skip steps. Do not proceed past a step
until it is verified complete.**

## Documentation-Only Subtasks

When a subtask modifies only documentation (no source code files), the
following cycle steps are adapted:

| Step | Action |
|------|--------|
| 1. Implement | Write documentation |
| 2. Write Tests | **Skip** — no code to test |
| 3. Update Docs | The implementation IS the documentation; verify navigation links are updated |
| 4. Pre-Commit Check | **Skip** — no source files to format or lint |
| 5. Code Quality Checks | **Skip** — no code to check |
| 6. Commit | **Follow exactly** — one commit per subtask, single SN identifier |
| 7. Mark Done | **Follow exactly** |

The absence of code changes **never** justifies batching multiple subtasks
into a single commit.

### 1. Implement

Write the code specified in the subtask's **Code** section.

### 2. Write Tests

Write the tests specified in the subtask's **Tests** section. See the
`run-tests` skill for how to run them.

### 3. Update Documentation

**Load the `documentation` skill before updating any docs.** Update all
task-related documentation per that skill.

**Hard gate — this step is not complete until:**

- [ ] At least one documentation file was created or updated for this subtask
- [ ] Documentation completeness is verified per the `documentation` skill's
      procedure

### 4. Pre-Commit Check

Run formatting and linting per the `code-style` skill. Do not commit until it
passes.

### 5. Code Quality Checks

- **Duplication check**: scan for accidental code duplication (same logic under
  different names, copy-pasted blocks). Consolidate if found.
- **Genericity check**: verify new code reuses existing abstractions and uses
  appropriate typing instead of bespoke re-implementations.
- **Equivalence test check**: if the subtask is a variant of an existing
  algorithm, ensure an equivalence test exists comparing it to the reference
  implementation.
- **Separation check**: verify library modules do not contain ad-hoc I/O or
  artifact generation that belongs in a dedicated module.

### 6. Commit

**Load the `git-workflow` skill before committing.** The commit message format
lives in the *contribution guidelines* hook; the operational procedure
(validation, merge) lives in the skill.

### 7. Mark Completed

Mark the subtask as completed in `tasks/detailed_plan.md` (`[done]` suffix +
commit hash on the subtask heading), then mirror the file onto the plan
comment of the task issue: find the comment by its `<!-- detailed-plan -->`
first line and PATCH its body to the current file content:

```bash
REPO=$(gh repo view --json nameWithOwner -q .nameWithOwner)
CID=$(gh api "repos/$REPO/issues/<N>/comments" \
  --jq '[.[] | select(.body | startswith("<!-- detailed-plan -->"))] | last.id')
{ echo '<!-- detailed-plan -->'; cat tasks/detailed_plan.md; } > /tmp/plan_comment.md
gh api -X PATCH "repos/$REPO/issues/comments/$CID" \
  --input <(jq -Rs '{body: .}' /tmp/plan_comment.md)
```

The local file is the single source of truth; the comment is a pure mirror.

If at any point in steps 1–7 you hit an unresolvable problem that prevents 100%
completion, **STOP the cycle immediately** and follow the Blocked Work Protocol
below. Do NOT attempt to "complete" the subtask with partial results, reverted
work, or known limitations. Do NOT proceed to the next subtask.

## Subtask Outcome

A subtask has exactly two valid outcomes:

- **Resolved**: implemented, tested, docs updated, committed. Record the commit
  hash in `tasks/detailed_plan.md`.
- **Blocked**: an algorithmic or design problem prevents 100% completion. Do NOT
  commit partial work. Do NOT proceed to the next subtask. Follow the Blocked
  Work Protocol.

There is no third state. "Reverted and left as a known limitation" is not a
valid outcome — it means the subtask is blocked. Report it.

Never silently skip a subtask. If a subtask was attempted, reverted, and its
planned changes were not committed, the subtask is incomplete. Do not mark it
done. Do not proceed. Report it as blocked.

## Per-Subtask Execution Tracking

For each subtask, use the `todowrite` tool to track cycle steps as separate
items. **No subtask may be committed with any step still `pending`.**

Example for subtask S1:

```
- "S1: Implement" -> in_progress -> completed
- "S1: Write tests" -> in_progress -> completed
- "S1: Update documentation" -> in_progress -> completed
- "S1: Pre-Commit Check (format + lint)" -> in_progress -> completed
- "S1: Quality checks" -> in_progress -> completed
- "S1: Commit" -> in_progress -> completed
```

## Multi-Subtask Discipline

When a task has multiple subtasks (S1, S2, S3, ...), execute them **strictly
sequentially**:

1. Complete all cycle steps for S1 (Implement → Tests → Docs → Pre-Commit Check
   → Quality → Commit → Mark Done).
2. Only after S1 is committed, start S2.
3. Never mark multiple subtasks `completed` in `todowrite` before committing
   each individually.

A `todowrite` listing "S1: Implement [completed], S2: Implement [completed], S1:
Write tests [completed]" indicates skipped commits — each subtask must be fully
committed before the next begins.

## Blocked Work Protocol

If you encounter an algorithmic problem that you cannot resolve to 100%
correctness, **STOP**. Do not commit. Do not merge. Do not comment out or weaken
failing tests to make the suite green. Instead:

1. Stay on the feature branch.
2. Report the problem concretely to the user:
   - Which tests fail and why.
   - What algorithmic gap exists (e.g., "the output is missing entries for
     nested inputs").
   - What you've tried and what remains unresolved.
3. Ask the user for guidance: additional subtasks, algorithmic hints, descoping,
   or splitting the task.
4. **Transfer user guidance to the task** per the `user-guidance-transfer`
   skill — post it verbatim as a comment on the task issue.
5. Append a `## Design Notes` section to `tasks/detailed_plan.md`. See the
   `planning` skill for the full template. Minimum required content:

   - **Correct Design**: algorithmic design as confirmed by the user —
     coordinate spaces, invariants, decomposition schema. Quote the user's
     design guidance verbatim where available.
   - **Blocked Subtasks**: which subtasks are blocked and why.
   - **Root Causes**: why each failure occurs, with concrete examples. Every
     limitation MUST be traceable to a concrete input, a concrete location in
     the data structure, and a concrete execution path in the code. Never write
     vague descriptions.
   - **Approaches Tried**: what was attempted and why it didn't fully work.
   - **Remaining Work**: concrete, actionable items (e.g., "Track the origin
     state through the queue by adding a field to the queue item") — not vague
     goals.
   - **Skipped Tests**: list any tests skipped and the reason.

   Commit this summary so the plan serves as a persistent design record for
   future task refinement.
6. Post the block report (the `## Design Notes` content) as a comment on the
   task issue, so the block is visible in the task record.

## Task Completion Verification

This section is the **single source of truth** for what "done" means. Before
a task is considered done — its last subtask's commit carries `Closes #<N>`
(the task's own issue) and is merged to its integration branch (see the
*contribution guidelines* hook) — verify:

- [ ] Every clause in the task description is traceable to implemented and
      committed code.
- [ ] No subtask was reverted without resolution.
- [ ] No requirement was silently skipped or deferred.
- [ ] All tests pass (0 failures, 0 skipped).
- [ ] All quality gates pass (see the `quality-gates` skill).
- [ ] If any of the above fails, the task is NOT done — it is blocked. Follow
      the Blocked Work Protocol. Do NOT close it with known unresolved
      limitations.

Partial completion is not completion. "All passing tests are for the parts I
did" does not mean the task is done if other parts were reverted.

## Marking Complete

A task is complete when its last subtask's commit carries `Closes #<N>` (the
task's own issue number) as a standalone line and that commit is on the
integration branch. The closing keyword takes effect when the commit reaches
`main`: in direct mode the task's own pull request carries it; in stacked mode
the issue stays open until the final pull request to `main` (opened on the
user's explicit request) lands. Never edit the issue body — it is
user-authored and immutable.

Commit hashes recorded in `tasks/detailed_plan.md` may change after an
integration-branch rebase; the `(<issue>-S<n>)` commit subject is the stable
key.

Complete means COMPLETE: every requirement met, every test passing, every
edge case handled. Never close a task with known failures or unresolved
limitations.
