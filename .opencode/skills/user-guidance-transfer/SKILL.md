---
name: user-guidance-transfer
description: Use when the user provides design guidance, descoping, algorithmic hints, or clarification during a task. Record it verbatim as a comment on the task issue so it is never lost or paraphrased.
---

# User Guidance Transfer

When the user gives guidance mid-task (design decisions, algorithmic hints,
descoping, clarification of ambiguity), transfer it into the task record
verbatim.

## Procedure

1. Post the guidance as a comment on the task issue:
   `gh issue comment <N> --body '[USER GUIDANCE]: "<verbatim quote>"'`.
   Never edit the issue body — it is user-authored and immutable.
2. Quote the user's guidance **verbatim**. Do not paraphrase, summarize, or
   interpret.
3. If the guidance is algorithmic and belongs in the persistent design record,
   also record it verbatim in the `### <Topic>` section of the `## Design
   Notes` block in `tasks/detailed_plan.md` (see the `planning` skill).

Example comment:

```
[USER GUIDANCE]: "Use CSR representation; the matrix indices must be the same
node order as the DFA from task 2."
```

Only transfer guidance the user actually gave; do not invent or pad it.
