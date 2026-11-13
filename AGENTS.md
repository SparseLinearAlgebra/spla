# AGENTS.md

This file is a short TOC of the agent workflow. The project itself is
described in `README.md`, which is landing for humans and the entrypoint for navigating the
repository for all.

## Start here

1. Load the `workflow-management` skill
   (`.opencode/skills/workflow-management/SKILL.md`) first, before doing
   anything else.
2. Read `README.md` for project navigation.
3. Before exploring code, load the `project-navigation` skill
   (`.opencode/skills/project-navigation/SKILL.md`): navigate via the README
   and the project structure map, not by scanning the whole repository.

## Main Principles

* This file is a short entry point for fast cold errors-free start.
* Only one source of truth. No duplicates. Each thing (anywhere in code, issues, pull request descriptions, and documentation) described exactly once. Use generalization (especially for code), cross-references, links, other similar techniques to avoid duplicates and reuse stuff. Find what you can to reuse before creating a new one.
* Source-of-truth hierarchy: code, scripts, CI configs > docs. If a fact can
  be extracted from code or scripts (e.g., CI configs), it is not duplicated
  in docs. Docs hold only what cannot be unambiguously reconstructed from
  code: design decisions, non-trivial constraints. Skills stay thin pointers
  to docs, code, or CI — they never re-describe them.
* Tools, not instructions. If you can do something with existing tool --- do it. No thinking, no long instructions, no manual analysis. You want to analyze code coverage? Just run coverage tool and analyze report. No workaround for regular tasks. If there is a tool for regular task it must be installed and configured appropriately.
* Never stay silent about problems. Any problem discovered or detected must be addressed. If it is in current scope or the fix is easy (e.g. a misprint), fix it — even if pre-existing. If it is significantly out of scope, report it to the user and propose creating an issue.
* Always learn, never forget — encode patterns before session ends.
* Issues, pull requests, and their comments are a knowledge base of technical
  details. Log task-related findings and inspirations in the descriptions or
  comments of the respective issues/PRs. When faced with a problem, check
  whether a similar problem was solved or described previously — look for the
  solution in existing issues, PRs, and their comments first.

## Project facts contract

Skills encode process only; every project-specific fact lives in exactly one
place. Skills reference the hooks below by name; this table is where they
resolve:

| Hook | Location |
|---|---|
| contribution guidelines — branching model, integration branch config key, commit format, pull requests | `CONTRIBUTING.md`, "Contribution guidelines" section |
| quality checks — build/test/style/docs commands, CI mapping, tool requirements | `DEVELOPMENT.md`, "Quality checks" section |
| project structure — directory map | `README.md`, "Project structure" section |
| documentation conventions — doc systems, change-to-doc-action mapping | `README.md`, "Documentation" section |

The CI workflows under `.github/workflows/` are the source of truth for what
CI executes.

## Skills

### Domain

| Skill | When to use |
|---|---|
| `.opencode/skills/project-navigation` | Starting navigation before exploring code |
| `.opencode/skills/run-tests` | Running the test suite |
| `.opencode/skills/code-style` | Formatting / linting before commit |

### Workflow / process

| Skill | When to use |
|---|---|
| `.opencode/skills/workflow-management` | Driving the overall task loop |
| `.opencode/skills/planning` | Global plans, atomic subtasks, task authoring |
| `.opencode/skills/subtask-loop` | Executing one atomic subtask |
| `.opencode/skills/git-workflow` | Branching, commits, merging |
| `.opencode/skills/reusing` | Finding existing code/docs to reuse, not duplicate |
| `.opencode/skills/user-guidance-transfer` | Recording user guidance verbatim |
| `.opencode/skills/documentation` | Mapping code changes to doc updates |
| `.opencode/skills/quality-gates` | The pre-merge gate that must pass |
| `.opencode/skills/code-review` | Whole-repo review before merge |