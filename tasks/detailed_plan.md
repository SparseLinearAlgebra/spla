# Detailed plan: adapt migrated opencode skills to spla

Exception task (environment setup): no GitHub issue is created for this task, so
tracking is local-only in this file and commits carry plain spla-style messages
instead of the `(<N>-S<n>):` key. Integration branch: `gsv/agentify` (stacked
mode). All subtasks are documentation-only: code gates (build, tests, lint) do
not apply; the relevant gate is the cross-reference verification listed at the
end.

### S1: CONTRIBUTING.md — Contribution guidelines section [done 528325f9]

Branching model (integration branch, `spla.integrationBranch` config key,
stacked/direct modes), per-subtask commit format with closing keywords, pull
request rules (template, rebase-and-merge).

### S2: DEVELOPMENT.md — fill stub sections + Quality checks [done ef59f3fb]

Building from sources (`build.py` options), unit-tests execution
(`run_tests.py`, CPU/GPU), release management (`bump_version.py`, deploy
workflows), new Quality checks section (local gate commands, CI workflow
mapping, tool requirements).

### S3: README.md — fix deps row, extend structure table, add Documentation section [done 148b451e]

`deps/` is vendored source, not submodules; structure table gains `tasks/` and
`AGENTS.md`; new Documentation section (Doxygen/pdoc/root-`.md` systems,
publish locations, source-change to doc-action mapping).

### S4: AGENTS.md — declare hook contract, update skills table [done 472137d3]

Hook-to-location contract (contribution guidelines, quality checks, project
structure, documentation conventions); skills table descriptions aligned with
the new skill frontmatter; PySymGym residue removed.

### S5: workflow-management skill to hooks [done 143c4b98]

Strip project facts (config key, doc paths) to hook references; keep the
working loop, batch/hub issue machinery, plan files, integration modes.

### S6: planning skill to hooks [done 49ebce4b]

Neutral examples; hook references for commit format and docs mapping; keep
global/detailed plan formats, subtask format, design notes, task authoring.

### S7: subtask-loop skill to hooks [done c3407243]

Generalize the docs-only rule (no source files); hook reference for the
integration branch; keep cycle, outcomes, tracking, blocked protocol,
completion verification.

### S8: git-workflow skill to hooks [done 3e70c99d]

Hook references for config key, commit format, PR flow; third-party content
rule generalized with paths resolved via the project structure hook; keep
operational procedures.

### S9: quality-gates skill to hooks [done e31b10a9]

Keep PASS/BLOCKED semantics and no-exceptions rule; gate contents referenced
via the quality checks hook.

### S10: run-tests skill to hooks [done d3c1c969]

Thin pointer to the quality checks hook + CI workflows; process rules (read
the doc first, 0 failures, never weaken tests).

### S11: code-style skill to hooks [in progress]

Thin pointer to the quality checks hook; process rule (no commit before clean).

### S12: documentation skill to hooks [pending]

Keep principles and completeness checklist; mapping table moves to the
documentation conventions hook.

### S13: project-navigation skill to hooks [pending]

Map-first and improve-as-you-go rules; map location via README + project
structure hook.

### S14: reusing skill to hooks [pending]

Reuse-over-duplicate principle and record-reuse rule; directory checklist via
the project structure hook.

### S15: code-review skill to hooks [pending]

Hook reference for the integration branch; doc conventions via the
documentation skill; keep review loop and checklist.

### S16: user-guidance-transfer skill to hooks [pending]

Already process-only; neutral example.

## Out-of-scope findings (reported, not fixed)

- `Doxyfile` INPUT references `include/spla_c/`, which does not exist in the repository; Doxygen warns about the
  missing input. Fixing the Doxyfile is out of scope for this task.

## Verification (docs-only task)

- [ ] Zero project facts in `.opencode/skills/` (no config keys, commands,
      paths, workflow filenames, PySymGym residue)
- [ ] Every hook used by a skill exists in the AGENTS.md contract and resolves
      to an existing doc section with that exact heading
- [ ] Every command/path in CONTRIBUTING.md, DEVELOPMENT.md, README.md verified
      against reality
- [ ] No fact duplicated across docs; skill frontmatter valid (name = folder,
      neutral description)
