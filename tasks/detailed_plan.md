# Detailed plan: continuous performance tracking on self-hosted (task #248)

Integration branch: `gsv/agentify` (stacked mode). Feature branch:
`gsv/benchmark-tracking`.

Track SSSP, triangle count, BFS and PageRank on both self-hosted GPUs
(NVIDIA GeForce GT 1030 — OpenCL platform 2; AMD Radeon RX 6750 XT — platform 1)
with the Continuous Benchmark action (`benchmark-action/github-action-benchmark@v1`,
marketplace name "Continuous Benchmark"). One figure per algorithm with both GPUs
in a single chart, benchmarked on real-world SuiteSparse matrices cached locally
on the runner.

User-confirmed decisions:

- Datasets: coAuthorsCiteseer (1.6M edges), roadNet-CA (5.5M), cit-Patents (33M) —
  capped by the GT 1030's 2 GB VRAM.
- Figure value per commit: sum of mean GPU times across the three datasets;
  per-dataset breakdown kept in the `extra` field (chart tooltip).
- Alerts: commit comment on alert AND workflow failure, threshold 150%.
- Data directory: `~/spla-bench-data`, overridable via `SPLA_BENCH_DATA` env var.

Reuse (per the reuse checklist): `spla::Timer::get_laps_ms()` for timing values;
the shared example option/CLI infrastructure in `examples/options.hpp` and
`examples/common.hpp`; the Python script style of `run_tests.py` (argparse,
stdlib only); the job structure, GPU matrix entries and platform/device IDs of
`.github/workflows/self-hosted.yml`; existing doc sections (DEVELOPMENT.md CI
table, README Performance section) instead of new pages. No new dependencies:
C++ JSON is hand-written for a fixed small schema; Python scripts use the
standard library only.

### S1: bench mode in the four algorithm examples [done f44de140]

**Code:** `examples/options.hpp` — new `OPT_BENCH "bench"` option (string, path
to a JSON file, default empty = disabled). `examples/common.hpp` — new helper
`write_bench_entry(const std::string& path, const char* algo, const
std::string& dataset, const spla::Timer& gpu)` computing mean and stdev of
`gpu.get_laps_ms()` (reuses the existing `Timer`) and writing one JSON object
`{"name": "<algo>", "dataset": "<dataset>", "unit": "ms", "value": <mean>,
"range": "<stdev>"}`. The `dataset` field (stem of `--mtxpath`) was added in
S3 so the aggregator can build per-dataset breakdowns without coupling to
file naming.
`examples/bfs.cpp`, `examples/sssp.cpp`, `examples/pr.cpp`, `examples/tc.cpp` —
after the GPU loop, call the helper with display names `BFS`, `SSSP`, `PR`, `TC`
when the option is set.

**Tests:** build locally; run all four examples on a small `.mtx` on the local
NVIDIA GPU with `--bench /tmp/b.json --run-cpu=false --run-ref=false`; validate
each JSON with `python3 -m json.tool`, check `value > 0` and `range >= 0`.
Verify the boolean-disable syntax works (used by the workflow in S5).

**Docs:** none — example CLIs are not documented (`EXAMPLES.md` covers C++ API
usage only); workflow-level docs land in S6.

**Spec:**
- The helper writes a single JSON object (no array) to the given path, creating
  or truncating the file; one call per process run.
- `value` is the arithmetic mean of the GPU laps in ms; `range` is the
  population stdev formatted as a string (the action treats `range` as text).
- The first lap is skipped as warmup when more than one lap exists: it carries
  one-time OpenCL costs (kernel compile, context init) that would otherwise
  dominate the metric on small graphs.
- When `--bench` is empty the examples behave exactly as before.
- The vendored cxxopts has no `--no-<bool>` negation; booleans are disabled
  with the explicit `--<opt>=false` form (verified in S1 tests).

### S2: benchmark data tooling with local cache [done f5e2e044]

**Code:** new `bench/` directory. `bench/matrices.json` — manifest array of
`{name, urls}` for coAuthorsCiteseer, roadNet-CA and cit-Patents; each entry
lists the canonical `sparse.tamu.edu` URL first and the
`suitesparse-collection-website.herokuapp.com` mirror as fallback. Named
`matrices.json` (not `datasets.json`) because the repo `.gitignore` rule
`dataset*` for graph data files would swallow a `datasets.json` name.
`bench/download_data.py` (stdlib only, argparse like `run_tests.py`) — args
`--data-dir` (default `$SPLA_BENCH_DATA` or `~/spla-bench-data`) and
`--manifest` (default `bench/matrices.json` next to the script).

**Tests:** the dev machine's DNS cannot reach any SuiteSparse file host
(`sparse.tamu.edu` and the S3 file host fail name resolution; herokuapp
resolves but 301-redirects to the unreachable S3 host), so the download/extract
pipeline was verified with `file://` URLs: fresh dir downloads and extracts,
re-run reports `cached`, a missing local copy plus unreachable URL exits 1
with per-URL errors. The three canonical URL paths were verified against the
herokuapp 301 `Location` headers.

**Docs:** none — the data location and offline behavior are documented in S6.

**Spec:**
- For each manifest entry the target is `<data-dir>/<name>.mtx`. If it exists
  and is non-empty: print `cached <name>` and skip (this makes runs fully
  offline after the first download).
- Otherwise try the entry's URLs in order: download to a temp file, extract
  the single `.mtx` member from the tarball into the data dir, rename it to
  the target name, verify the result is non-empty. The first URL that works
  wins; all errors are collected and reported together.
- No local copy and every URL failed: print an error naming the dataset and
  exit 1 (the workflow must not silently benchmark on missing data).

### S3: per-GPU aggregation script

**Code:** `bench/aggregate.py` (stdlib only) — args `--gpu <label>`,
`--out <path>` and one or more per-run JSON file paths (S1 output, one per
algorithm x dataset). Also extends the S1 per-run schema with the `dataset`
field (helper signature gains a `dataset` parameter; the four examples pass
the stem of `--mtxpath`).

**Tests:** real runs of `bfs` on two differently-named copies of a small
`.mtx` (verifies the dataset field end-to-end) plus synthetic per-run files
for all four algorithms x three datasets; validate the output JSON: 4
entries, correct names, values equal the sums of the per-dataset means,
breakdown in `extra`; unreadable input exits 1.

**Docs:** none — covered by S6.

**Spec:**
- Group per-run entries by their `name` (algorithm), preserving first-seen
  order. Output is a JSON array with one entry per algorithm:
  `{"name": "<ALGO> - <GPU>", "unit": "ms", "value": <sum of values>,
  "extra": "<dataset>: <value> ms; ..."}` where the breakdown lists each
  dataset in input order.
- No `range` on aggregated entries (the action treats it as optional).

### S4: custom dashboard page on gh-pages

**Code:** `dev/bench/index.html` — static Chart.js page (CDN script, same
approach as the action's default template) that loads `data.js`
(`window.BENCHMARK_DATA`) and renders one canvas per algorithm: series are
grouped by the algorithm prefix of each benchmark name (text before the first
` - `), one dataset line per GPU suffix. X axis is commit date, tooltip shows
value, the `extra` breakdown and a link to the commit.

**Tests:** serve the directory locally with a hand-written `data.js` containing
two commits x 4 algorithms x 2 GPUs; verify in a browser that exactly 4 charts
render, each with 2 lines; check JS syntax with `node --check` if available.

**Docs:** none — the dashboard URL is documented in S6.

**Spec:**
- Committed directly to the existing `gh-pages` branch under `dev/bench/`
  (separate from the feature branch; the action keeps an existing
  `index.html` and only updates `data.js`). No conflict with the `docs-cpp/`
  and `docs-python/` dirs published by the docs workflows.
- The page must tolerate a missing or single-commit `data.js` (first run)
  without JS errors.

### S5: benchmark workflow

**Code:** `.github/workflows/benchmark.yml`. Trigger: push to
`main`/`release`/`pre-release` with a code paths filter (`.github/**`,
`examples/**`, `include/**`, `src/**`, `bench/**`, `CMakeLists.txt`); no
pull_request trigger (action security caveat). Top-level permissions:
`contents: write`, `deployments: write`.

**Tests:** YAML parse check (`python3 -c "import yaml,sys; yaml.safe_load(...)"`).
End-to-end verification happens on the first CI run after integration (seeds
`data.js` and the dashboard); the plan tracks that as a post-integration check.

**Docs:** the CI mapping table row is added in S6 (single source of truth).

**Spec:**
- Job `build` (self-hosted): checkout with submodules, CMake configure + build
  exactly like the build job of `self-hosted.yml` (examples are ON by default).
- Job `bench` (needs build, self-hosted, matrix over the two GPUs reusing the
  platform/device IDs from `self-hosted.yml`): run
  `python3 bench/download_data.py`; for each algorithm in bfs sssp pr tc and
  each dataset in the manifest run
  `./build/<algo> --mtxpath <data-dir>/<ds>.mtx --platform <p> --device 0
  --run-cpu=false --run-ref=false --bench <out.json>` (binaries land at the
  build dir root, not in a subdirectory); then
  `python3 bench/aggregate.py --gpu <label> ...` and upload the per-GPU JSON as
  an artifact.
- Job `store` (needs bench, ubuntu-latest to keep the GPU box free): download
  both artifacts, merge the two JSON arrays into one file, run
  `benchmark-action/github-action-benchmark@v1` with
  `tool: customSmallerIsBetter`, `name: spla`, `github-token:
  ${{ secrets.GITHUB_TOKEN }}`, `auto-push: true`, `gh-pages-branch: gh-pages`,
  `benchmark-data-dir-path: dev/bench`, `comment-on-alert: true`,
  `fail-on-alert: true`, `alert-threshold: 150%`.

### S6: documentation

**Code:** none (documentation-only subtask; code gates skipped, docs build is
the relevant gate).

**Tests:** docs build per the quality checks hook (Doxygen + pdoc); verify the
new links resolve.

**Docs:** `DEVELOPMENT.md` — add the `benchmark.yml` row to the CI mapping
table and a new "Performance tracking" section: dashboard URL, workflow
overview, data directory and offline behavior, how to add a dataset to the
manifest, alert semantics. `README.md` — link the dashboard in the Performance
section.

**Spec:**
- The DEVELOPMENT.md section states facts that cannot be read from code (why
  sparse.tamu.edu, why local cache, VRAM constraint) and points to the manifest
  and workflow for everything else — no duplication of their content.
