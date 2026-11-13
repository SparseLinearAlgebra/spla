---
name: planning
description: Use when planning tasks: creating global plans across multiple tasks, decomposing a task into atomic subtasks, or authoring new task descriptions. Covers multi-task planning, detailed plan format, atomic subtask requirements, and task authoring guidelines.
---

# Planning

Project-specific facts (integration branch, commit format, doc locations) are
resolved through the hooks declared in the project facts contract of
`AGENTS.md`.

## Multi-task Planning

When the user asks to work on a **set of related tasks**, do NOT jump directly
into implementation. First set up the batch, then plan it:

1. Create a **hub** issue (label `hub`) for the batch and one issue per task
   (label `task`), and attach each task issue to the hub as a GitHub
   **sub-issue** via the REST API (`POST repos/$REPO/issues/<HUB>/sub_issues`
   with the task's database `id`); a link in the hub body is **not** a
   sub-issue. The exact commands live in the `workflow-management` skill.
2. Create the high-level global plan in `tasks/global_plan.md`, naming the hub
   and listing each task with its issue number, then mirror it to the hub as a
   comment whose first line is `<!-- global-plan -->`.

The global plan must:

- Name the **hub** issue it belongs to.
- List all tasks to be done with their IDs, issue numbers (`#N`), brief
  descriptions, and a **Status** (`[done #N]` once the task is integrated).
- Record the integration branch the batch targets (see the *contribution
  guidelines* hook; `main` by default).
- Identify dependencies between tasks (which must be done before which).
- Identify potential conflicts or overlapping changes (e.g., two tasks
  modifying the same file).
- Identify shared infrastructure (types, helpers, utilities) that multiple
  tasks need. Create shared modules to avoid duplication across tasks.
- Identify existing reusable abstractions (types, helper functions, generators)
  that new tasks should use rather than reinvent.
- **Reuse analysis**: load the `reusing` skill and run the reuse checklist
  against existing code and documentation before creating new material.
- For each task that produces artifacts (reports, plots, exported models),
  note the exact output format requirements up front.
- Propose an execution order that minimizes rework and avoids conflicts.
- Align tasks with the project architecture.

After the global plan is created and mirrored to the hub, proceed with the
normal working loop: one task at a time, a feature branch per task created from
the integration branch, a detailed plan in `tasks/detailed_plan.md` for each.
After each task integrates, mark it `[done #N]` in `tasks/global_plan.md` and
update the hub's `<!-- global-plan -->` comment. The local file is the single
source of truth; the hub comment is a pure mirror (see the `workflow-management`
skill).

## Detailed Plan (Atomic Subtasks)

`tasks/detailed_plan.md` MUST decompose the task into atomic subtasks. Each
subtask:

- Is small enough to complete in a single focused work session (typically
  30–90 minutes).
- Produces a compilable, testable increment — no partial implementations left
  uncommitted.
- Has a unique identifier (e.g., "S1", "S2") used in commit messages and plan
  tracking.

If a task is ambiguous or underspecified **during planning** (conflicting
requirements, unclear scope, missing constraints), do not guess. Ask the user
for clarification, then transfer the guidance to the task per the
`user-guidance-transfer` skill before proceeding with decomposition.

Before writing the detailed plan, load the `reusing` skill and run the reuse
checklist: search existing documentation and code for functions, types, and doc
sections that can be reused or generalized. Record what is reused in each
subtask's **Code** section.

### Subtask Format

Each subtask in `tasks/detailed_plan.md` MUST include these four sections. **A
subtask with missing Code, Tests, or Docs sections is incomplete and must not
be executed.**

```
### SN: <title>

**Code:** <files to create or modify, types and functions to add>
**Tests:** <test files to create or modify, specific test approaches>
**Docs:** <doc files to create or update. Use the `documentation` skill
          to determine which docs are affected.>

**Spec:**
- <detailed implementation specification>
```

Example:

```
### S1: Add a batched variant of the existing reduction operation

**Code:** New `reduce_batched` function in the reduction module (reuse the
          existing kernel dispatch from the same module)
**Tests:** New unit tests covering empty, single-element and multi-block
           inputs, plus an equivalence test against the unbatched reference
**Docs:** Update the API reference entry for the new function per the
          documentation conventions

**Spec:**
- Dispatch to the existing reduction kernel with a per-batch accumulator ...
```

### Documentation Mapping

For the mapping of source changes to required doc actions, load the
`documentation` skill — it is the single source of truth. Use it when writing
the **Docs** section of each subtask.

### Granularity

If a subtask cannot be committed as a self-contained increment, it is too large
— split it further. The commit message format lives in the *contribution
guidelines* hook; the operational commit procedure lives in the `git-workflow`
skill.

### Bounded uncommitted work

Uncommitted work on a feature branch must never exceed one atomic subtask. If a
session is interrupted, the loss is bounded to that single subtask.

## Post-Implementation Design Notes

When a task hits algorithmic limitations — partially completed, with skipped
tests or remaining work — append a `## Design Notes` section to
`tasks/detailed_plan.md`. This serves as persistent design knowledge for future
task refinement. The section must be structured as follows:

```
## Design Notes (discovered during implementation)

### <Topic Title>

Design rationale, coordinate spaces, invariants — as confirmed by the user.

### <Failure Topic Title>

- Root causes with concrete examples (e.g., "for input X, the output is
  missing entry Y because...")
- What was attempted and why it didn't fully work
- Remaining work: concrete, actionable items
- Skipped tests: list them and the reason
```

Requirements:

- Every algorithmic limitation MUST be traceable to a concrete input, a
  concrete location in the data structure, and a concrete execution path in
  the code.
- Never write vague descriptions like "some entries are missing" — specify
  which input, which state/range, and which path should produce them.
- Remaining work items must be actionable (e.g., "Track the origin state
  through the queue by adding a field to the queue item") — not vague goals.
- If the user provided design guidance (e.g., decomposition schema, coordinate
  system), record it verbatim in the `### <Topic>` section as the
  authoritative reference.

## Task Authoring Guidelines

When creating a new task issue (`gh issue create`, label `task`), follow
these rules (they apply to the issue body):

- **Specify output format upfront**. If the task produces report artifacts
  (plots, tables, exported models), include the exact layout, units, and
  formatting rules in the task description.
- **Keep tasks single-responsibility**. A task should do one thing. If it
  requires more than 5 sub-items or spans multiple unrelated concerns, split it
  into multiple tasks.
- **Specify equivalence requirements**. For any new algorithm variant,
  explicitly state "must produce results identical to X" so equivalence tests
  are built from the start.
- **Specify type genericity**. If a module must handle arbitrary types, state
  it explicitly (e.g., "generic over the input and value types").
- **Specify reuse expectations**. If the task builds on existing infrastructure
  (e.g., "reuse the dataset abstractions from the common module"), name the
  dependencies. This prevents reinvention.

## Task Completeness Verification

See the `subtask-loop` skill — it is the single source of truth for verifying
task completion before a task is considered done.
