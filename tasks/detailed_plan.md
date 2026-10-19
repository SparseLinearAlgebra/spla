# Detailed plan: ignore docs build outputs in .gitignore (task #249)

Integration branch: `gsv/agentify` (stacked mode). Feature branch:
`gsv/gitignore-docs-build`.

The local quality gate's docs build leaves untracked output directories in the
project root (`html/` and `latex/` from `doxygen Doxyfile`, `pydocs/` from
`SPLA_DOCS=true pdoc -o pydocs python/pyspla`). `.gitignore` does not cover
them, so they pollute `git status` after every local docs build and risk
accidental commits.

### S1: add root-anchored ignore patterns for the docs build outputs [done aac38c34]

**Code:** `.gitignore` — new "Docs build output" section with `/html/`,
`/latex/` and `/pydocs/` (root-anchored so directories with these names in
subdirectories are not affected).

**Tests:** `git check-ignore -v html latex pydocs` reports the new patterns;
create the three directories locally, confirm `git status --short` is clean,
remove them again.

**Docs:** none — `.gitignore` is tooling configuration, covered by the quality
checks hook's commands (no doc section describes it).

**Spec:**
- Patterns are root-anchored (`/html/` etc.), matching only the build outputs
  created at the project root by the documented docs build commands.
