---
name: run-tests
description: Use when running the test suite. Thin pointer to the quality checks hook and the CI workflows it references, which hold the exact commands, plus process rules for running and interpreting results.
---

# Run tests

What the test pipeline is and where the exact commands live are in the
*quality checks* hook (see the project facts contract of `AGENTS.md`) and the
CI workflows it references. Read them before running anything.

## Rules

- Run exactly what the *quality checks* doc prescribes for the areas you
  changed; when in doubt, run the full suite.
- Success is 0 failures and 0 skipped. A suite that "passes" with skips or
  weakened assertions does not count.
- Never weaken, skip, or comment out failing tests to make the suite green
  (see the Blocked Work Protocol in the `subtask-loop` skill).
- If a required tool is missing on this machine, install it at the version the
  *quality checks* doc prescribes — do not substitute a different runner.
