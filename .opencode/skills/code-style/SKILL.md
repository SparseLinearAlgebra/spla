---
name: code-style
description: Use before committing to format and lint. Thin pointer to the quality checks hook and the CI workflow it references, which hold the exact commands and tool versions.
---

# Code style

What the style checks enforce and the exact commands (including tool versions)
are in the *quality checks* hook (see the project facts contract of
`AGENTS.md`); CI runs the same checks. Read that section before running
anything.

Do not commit until the check passes clean. If a required tool is missing on
this machine, install it at the version the doc prescribes — a different
version may format differently and produce spurious diffs.
