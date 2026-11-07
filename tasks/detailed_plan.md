# Detailed plan: OpenCL mxm (SpGEMM) kernel (task #252)

Integration branch: `gsv/agentify` (stacked mode). Feature branch: `mxm-opencl`.

Specification: `Algorithm Semantics.md` (algorithm, 10 steps + quality criteria),
`plan_final.md` Part 1 (this plan's source), `GALATIC/` (reference).

Mapping of subtasks to Algorithm Semantics steps and GALATIC files:

| Si | Semantics step | GALATIC (how) |
|----|----------------|---------------|
| S1 | — (spla infra) | — |
| S2 | 1: estimate symbols/chunks, pool | `source/device/Multiply.cuh` |
| S3 | 2–4: work split, mult in scratch, sort+combine, chunk to pool | `acSpGEMM_DetermineBlockStarts.cuh`, `acSpGEMM_SpGEMM.cuh`, `SortAndCombine.cuh`, `Chunk.cuh` |
| S4 | 4–6: DirectChunk, group by row, SIMPLE/MAX_CHUNKS/ITERATIVE | `Chunk.cuh`, `MergeCaseOffsets.h`, `MultiplyKernels.h` |
| S5 | 7: merge by category | `acSpGEMM_Merge*.cuh` → spla `cl_merge_union` |
| S6 | 8–9: identity removal, offsets + CSR | merge files, `Chunk.cuh`, `acSpGEMM_ChunksToCSR.cuh` |
| S7 | 10: pool growth, OOM | `source/device/Multiply.cuh` |
| S8 | quality criteria 1–3 | — |

Test notation: a matrix = a list of rows `{row:[(col,value),…]}`; ∅ empty. Large rows
(thresholds FLUSH_THRESHOLD/DIRECT_ROW_THRESHOLD/HEAVY_DIRECT_THRESHOLD) generated
programmatically. Reference for comparison: `Algo_mxm_cpu`.

### S1: Skeleton and registration [done 0b2d0c45]

**Code:** `src/opencl/cl_mxm.hpp` with `Algo_mxm_cl<T>` modeled on `cl_mxmT_masked.hpp`;
reuse `CLProgramBuilder`, `CLCsr<T>`, `cl_csr_resize<T>`, `TIME_PROFILE_SCOPE`,
`CL_DISPATCH_PROFILED`. Register mxm for INT/UINT/FLOAT in `src/opencl/cl_algo_registry.cpp`;
add `cl_mxm.hpp` to SRC_OPENCL in `CMakeLists.txt`.

**Tests:** `tests/test_opencl_mxm.cpp` + `spla_test_target(test_opencl_mxm)` in `tests/CMakeLists.txt`:
- `mxm.empty`: A(3×3)=∅, B(3×3)={0:[(0,1)]} → R=∅ (nnz=0, Ap=[0,0,0,0]); vice versa; both empty. No crash.
- `mxm.incompatible`: A(3×4), B(5×2) → size error before kernel launch.
- `mxm.types`: product of `mxm.naive` (S8) on INT/UINT/FLOAT gives same R.

**Docs:** Doxygen for `Algo_mxm_cl`; `CHANGELOG.md` entry about OpenCL mxm support.

**Spec:**
- Name and key match the dispatcher; kernels may be stubs, but the mxm path does not crash.

### S2: Pool size estimation [done 802cd356]

**Code:** host-side in `cl_mxm.hpp`: estimate symbols and chunks per row and the final pool size
statistically via the GALATIC formula (`Multiply.cuh`: average row lengths of A/B, column overlap),
with `POOL_OVERALLOC` margin; allocate the pool via `cl_alloc`. No separate counting kernels.

**Tests:** `mxm.pool_estimate` — compare with a CPU reference of the same formula on
A(3×3)={0:[(0,1),(1,1),(2,1)],1:[(0,1),(2,1)],2:[(1,1)]}, B(3×3)={0:[(0,1),(2,1)],1:[(1,1)],2:[(0,1),(1,1),(2,1)]}:
compute a_avg_row=nnz(A)/rows(A), b_avg_row=nnz(B)/rows(B), overlap=b_avg_row/cols(B); pool = max(…)·(sizeof(T)+sizeof(uint))·POOL_OVERALLOC, clamped to [minExpectedNNZ, maxExpectedNNZ]. Check equality and bounds.
Underestimation/pool overflow covered in S7.

**Docs:** none.

**Spec:**
- Estimate is statistical and may be inexact (miss handled in S7); no preliminary chunk count — chunks created dynamically in S3/S4, classification in S4.

### S3: NormalChunk [done 42e53410]

**Code:** work distribution — each work-group gets a slice of A (`acSpGEMM_DetermineBlockStarts.cuh`);
`mxm_chunk_product` accumulates (col, A[i,k]·B[k,j]) in scratch (`acSpGEMM_SpGEMM.cuh`); sort+combine via
`cl_sort_by_key<T>`/`cl_reduce_by_key<T>` (`SortAndCombine.cuh`); copy to pool.

**Tests:**
- `mxm.chunk_sorted`: A(1×2)={0:[(0,1),(1,2)]}, B(2×2)={0:[(0,1)],1:[(0,1),(1,3)]} → symbols (0,1),(0,2),(1,6) → after sort+combine {(0,3),(1,6)} (col=0 collapsed 1+2=3, sorted).
- `mxm.chunk_over_flush`: A row with two nonzeros, each B row length 600 → 1200 symbols (>FLUSH_THRESHOLD=1024, ≤ACCUMULATION_CAPACITY=2048) → one chunk of 1200, not truncated.

**Docs:** none (internal kernel).

**Spec:**
- Flush at ≥ FLUSH_THRESHOLD, on ACCUMULATION_CAPACITY fill, or group end; chunk sorted, without duplicates, size ≤ ACCUMULATION_CAPACITY.

### S4: DirectChunk and row classification [done 42e53410]

**Code:** `mxm_materialize_direct` (DirectChunk = B-row reference + multiplier, `Chunk.cuh`);
register chunk in linked list of its row (head per row + next in chunk, chunk counter per row — like
`outputRowListHead`/`outputRowChunkCounter` in `Multiply.cuh`, `writeNextFront`/`readNextFront` in `Chunk.cuh`),
mark rows for merge (step 5); `mxm_classify_rows` (SIMPLE/MAX_CHUNKS/ITERATIVE, `MergeCaseOffsets.h`, `MultiplyKernels.h`);
buffer D in `cl_mxm.hpp`.

**Tests:**
- `mxm.direct_below`/`mxm.direct_above`: A row with one nonzero (k, v=2), B[k] length 2000 (< HEAVY_DIRECT_THRESHOLD=4096) and 5000 (>) → R row = 2·B[k] in both.
- `mxm.direct_only`: A row single nonzero (k=0), B[0] length 1500 → one light DirectChunk through MAX_CHUNKS path (one pass, identity removal), R = materialized B[0].
- `mxm.direct_mixed`: A row with (k1 short) + (k2 long) → NormalChunk + DirectChunk merged into one R row.
- `mxm.direct_heavy_portions`: heavy DirectChunk (5000) materialized in portions; R = 2·B[k] in full.

**Docs:** none (internal kernels).

**Spec:**
- DirectChunk stores b_row + multiplier; materialization multiplies B values by multiplier; B row already sorted → DirectChunk not deduplicated, always merged.
- Classification: SIMPLE = one ordinary chunk; MAX_CHUNKS = one light DirectChunk or ≤ MAX_CHUNKS_TO_MERGE chunks; ITERATIVE = > MAX_CHUNKS_TO_MERGE or heavy DirectChunk.

### S5: Merging via cl_merge_union [done 42e53410]

**Code:** CPU loop in `cl_mxm.hpp`; pairwise merge; T0/T1/D; identity removal after each merge; final row to pool.
Contract from `algorithms/merge-path/merge_union_usage.md`. GALATIC uses its own merge-path kernels
(`acSpGEMM_MergeSimple/MergeMaxChunks/MergeGeneralized.cuh`); spla uses the ready `cl_merge_union` (pr-245).

**Tests:**
- `mxm.path_simple`: A row one nonzero k, B[k] length 100 → one NormalChunk (SIMPLE), R = chunk without merging.
- `mxm.path_max_chunks`: A row 8 nonzeros, each B row length 500 → 4000 symbols → ~2 chunks (≤16) → one pass (MAX_CHUNKS).
- `mxm.path_iterative`: (a) A row 40 nonzeros × 1000 symbols → ~20 chunks (>16); (b) heavy DirectChunk (B row 5000). Both → ITERATIVE.
- `mxm.tmp_overflow`: intermediate merge larger than initial T0/T1/D → buffer expanded, merge repeated, no partial write to pool.

**Docs:** none (internal CPU loop).

**Spec:**
- `cl_merge_union` inputs sorted, no duplicates; output buffer holds a_size + b_size; no partial write to pool; temp allocator lives until queue completes.

### S6: Identity and CSR assembly [done 42e53410]

**Code:** `mxm_compact_identity`, `mxm_row_sizes`, `cl_exclusive_scan<uint>`, `mxm_write_rows_to_csr`;
`cl_csr_resize<T>` or manual R->Ap/Aj/Ax. CSR assembly as `acSpGEMM_ChunksToCSR.cuh`; identity removal as merge files/`Chunk.cuh`.

**Tests:**
- `mxm.path_zero_drop`: A(1×2)={0:[(0,1),(1,-1)]}, B(2×2)={0:[(0,1)],1:[(0,1)]} → col 0 = 1+(-1)=0 → dropped at sort+combine; R=∅.
- `mxm.identity_drop`: identity after merging two chunks (NormalChunk + DirectChunk multiplier −1) → col 0 removed, other cols kept.
- `mxm.csr_assembly`: empty R → Ap=[0,…,0], Aj=Ax=∅; one row R(1×2)={(0,3),(1,6)} → Ap=[0,2], Aj=[0,1], Ax=[3,6]; all rows vs `Algo_mxm_cpu`.

**Docs:** Doxygen for new public functions, if any.

**Spec:**
- Identity removed after each merge, and for SIMPLE after chunk creation; Ap from final row lengths; empty result: values=0, empty Aj/Ax, zero Ap.

### S7: Pool recovery and out-of-memory [done]

**Code:** pool growth, overflow flag, restart point, repeat unfinished rows, growth of pointer array and T0/T1/D;
`Status::OutOfMemory = 9` in `config.hpp`; `SPLA_STATUS_OUT_OF_MEMORY = 9` in `spla.h`. As `source/device/Multiply.cuh`
(step 10: up to POOL_RETRIES, then OOM).

**Tests:** (test hook: small initial pool / small POOL_RETRIES)
- `mxm.memory_underestimate`: initial pool < actual chunk volume → pool expanded, R correct (vs `Algo_mxm_cpu`).
- `mxm.pool_chunk_over`: one chunk > initial pool → pool grows to chunk size, R correct.
- `mxm.ptr_array_overflow`: many rows/chunks overflow pointer array → expanded, work continues.
- `mxm.memory_oom`: no expansion after POOL_RETRIES → `Status::OutOfMemory` (=9), propagated through C API (`SPLA_STATUS_OUT_OF_MEMORY`).

**Docs:** Doxygen for `Status::OutOfMemory` and `SPLA_STATUS_OUT_OF_MEMORY`; `CHANGELOG.md`.

**Spec:**
- Completed chunks not recomputed; pool offsets valid; new buffer not smaller than the chunk that did not fit.

### S8: End-to-end tests and semirings [done]

**Code:** tests in `tests/test_opencl_mxm.cpp`: (+,×) and (min,+), INT/UINT/FLOAT, rectangular, all classification
paths, chunk > FLUSH_THRESHOLD and ≤ ACCUMULATION_CAPACITY, chunk > initial pool. Reference: `Algo_mxm_cpu`.

**Tests:** vs `Algo_mxm_cpu` (exact INT/UINT, tol 1e-4 float):
- `mxm.naive`: A(3×4)={0:[(0,1),(2,2)],1:[(1,3),(3,4)],2:[(0,5),(2,6)]}, B(4×2)={0:[(0,7),(1,1)],1:[(1,1)],2:[(1,1)],3:[(1,1)]} → R(3×2)={0:[(0,7),(1,3)],1:[(1,7)],2:[(0,35),(1,11)]}.
- `mxm.semiring_minplus`: (min,+), init=+∞: A(2×2)={0:[(0,1),(1,3)],1:[(0,2),(1,1)]}, B(2×2)={0:[(0,1),(1,2)],1:[(0,4),(1,1)]} → R(2×2)={0:[(0,2),(1,3)],1:[(0,3),(1,2)]}.
- `mxm.semiring_bool`: (LOR, LAND), INT, init=0: A(2×2)={0:[(1,1)]}, B(2×2)={1:[(0,1)]} → R(2×2)={0:[(0,1)]}.
- `mxm.rectangular`: A(2×3)={0:[(0,1),(2,2)]}, B(3×4)={0:[(0,1),(3,2)],2:[(1,3)]} → R(2×4)={0:[(0,1),(1,6),(3,2)]}.
- `mxm.scale`: matrices from `bench/matrices.json` without memory failure; vs `Algo_mxm_cpu`.
- Full run of project tests.

**Docs:** none (`CHANGELOG.md` entries added in S1 and S7).

**Spec:**
- Result matches `Algo_mxm_cpu` on all covered configurations; all classification and pool-recovery branches covered.
