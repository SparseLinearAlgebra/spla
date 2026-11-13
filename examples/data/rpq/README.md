# RPQ example data

Small regular path query test data ported from LAGraph
(`data/rpq_data/`), Copyright LAGraph Contributors, BSD-2-Clause.

- `graph/{a,b}.mtx` — per-label adjacency matrices of a directed graph (8 vertices).
- `query/{a,b}.mtx` — per-label NFA adjacency matrices for the regex `(a b)*`.
- `query/meta.txt` — NFA start/final states (`nqs QS.. nqf QF..`, 1-based).
- `query/sources.txt` — source vertices (1-based).

Expected reachable vertices (1-based): `2 6 8`.

Run:

```shell
./build/rpq \
  --graph examples/data/rpq/graph \
  --query examples/data/rpq/query \
  --query-meta examples/data/rpq/query/meta.txt \
  --query-sources examples/data/rpq/query/sources.txt
```
