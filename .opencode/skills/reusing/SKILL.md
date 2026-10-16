---
name: reusing
description: Use before creating any new code, types, functions, or documentation. Run this reuse checklist to find existing material to reuse or generalize instead of duplicating. Enforces the "one source of truth / no duplicates" principle.
---

# Reusing

Before writing new code or docs, search for existing material and reuse or
generalize it. Duplicating an existing function, type, or doc section violates
the project's single-source-of-truth principle.

## Checklist

Run through these before creating new material, using the *project structure*
hook (see the project facts contract of `AGENTS.md`) to know where each kind
of material lives:

- [ ] **Existing code** — search the library source for an existing function,
      class, or type that already does (or nearly does) the job. Prefer
      generalizing the existing one over adding a near-copy. Start from the
      shared/common modules named in the structure map.
- [ ] **Third-party content** — check the vendored/third-party directories
      before adding a new dependency or re-implementing what one already
      provides.
- [ ] **Build/tooling scripts** — check the project's build and utility
      scripts for an existing helper before adding a new one.
- [ ] **Tests** — check the test directory for existing fixtures, helpers, or
      patterns to extend rather than duplicate.
- [ ] **Docs** — check the structure map and existing doc pages and docstrings
      for a section to update rather than add a new page; follow the
      `documentation` skill.

## Rules

- If a close match exists, **generalize it** (make it generic enough to cover
  both cases) rather than copy-paste.
- Record what is reused in each subtask's **Code** section of
  `tasks/detailed_plan.md`.
- When in doubt, prefer the existing abstraction; only create new material when
  no reasonable generalization is possible.
