# Global plan: OpenCL mxm + PathIndex + graph algorithms

Hub issue: [#251](https://github.com/SparseLinearAlgebra/spla/issues/251)

Integration branch: `gsv/agentify` (stacked mode). Feature branch: `mxm-opencl`.

Specification sources of truth: `Algorithm Semantics.md` (mxm algorithm), `plan_final.md`
(approved implementation plan), `GALATIC/` (mxm reference).

## Tasks

| ID | Issue | Description | Status |
|----|-------|-------------|--------|
| T1 | #252 | OpenCL `mxm` (SpGEMM) kernel (Part 1, S1–S8) | [ ] |
| T2 | #253 | `PathIndex` type (Part 2) | [ ] |
| T3 | #254 | Graph algorithms: reach-rpq, single-path-cfpq, msp-bfs, msl-bfs (Part 3) | [ ] |

## Dependencies

- T2 depends on T1 (`path_index.zero_drop` exercises `exec_mxm`).
- T3 depends on T1 and T2 (single-path-cfpq uses `PathIndex`).

## Conflicts / overlapping changes

- T2 and T3 both modify `src/algorithm.cpp` / `include/spla/algorithm.hpp`.
- T1 and T2 both modify `src/opencl/cl_mxm.hpp` (T2 adds `USE_PATH_INDEX_SEMANTICS`)
  and type registration files.
- Resolved by sequential execution order (below).

## Shared infrastructure

- `cl_merge_union` (merge-path) — already on `mxm-opencl` via `pr-245`; used by T1.
- `PathIndex` type (T2) — reused by T3 (single-path-cfpq).

## Reuse

- T1: `cl_mxmT_masked.hpp` (skeleton), `cl_merge_union`, `cl_sort_by_key`/`cl_reduce_by_key`/
  `cl_exclusive_scan`/`cl_csr_resize`, `CLProgramBuilder`, `cl_alloc`, `Algo_mxm_cpu`.
- T2: `Pair` (PR #241) as reference, `common_def.cl` macro machinery, `cl_reduce_by_key`/`cl_merge_union`.
- T3: existing semiring ops (`LOR_INT`/`LAND_INT`, `MIN_INT`/`FIRST_INT`), `exec_mxm`/`exec_m_eadd`,
  `PathIndex`, `bench/` infra.

## Execution order

T1 → T2 → T3.
