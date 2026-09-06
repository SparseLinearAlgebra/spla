# Graph Algorithms with SPLA

This project contains two implementations of graph algorithms:

- **Classic (CPU):** Python reference implementation
- **SPLA (GPU):** implementation using sparse linear algebra primitives from SPLA

Tests (`test_compare.py`) checks that both implementations produce identical results.

Run tests:
```bash 
python -m unittest test_compare.py -v
```
---

## Supported Algorithms

| Algorithm | Flag |
|-----------|------|
| BFS (Breadth-First Search) | `--algo bfs` |
| SSSP (Single-Source Shortest Paths) | `--algo sssp` |
| PageRank | `--algo pr` |
| Triangle Counting | `--algo tc` |

---

## Installation

```bash
cd python
python -m venv venv
source venv/bin/activate      
pip install -e .
cd algorithms
```

---

## Usage

Two entry points are available:

- `main_spla.py` — GPU version (SPLA)
- `main_classic.py` — CPU reference version

Both scripts use the same CLI arguments.

```text
usage: main_spla.py [-h] --algo {bfs,sssp,pr,tc} [-m MATRIX] [-v VECTORS] [-o OUTPUT] [-s START] [-a ALPHA] [-e EPS]

options:
  -h, --help            show this help message and exit
  --algo {bfs,sssp,pr,tc}
                        Algorithm to run:
                        bfs  - Breadth-First Search
                        sssp - Single-Source Shortest Paths
                        pr   - PageRank
                        tc   - Triangle Counting
  -m MATRIX, --matrix MATRIX
                        Path to graph in Matrix Market format (.mtx)
  -v VECTORS, --vectors VECTORS
                        Path to graph in vectors format (.txt)
  -o OUTPUT, --output OUTPUT
                        Output file path (default: result.txt)
  -s START, --start START
                        Start vertex (used in bfs, sssp; default: 0)
  -a ALPHA, --alpha ALPHA
                        Damping factor (used in pr; default: 0.85)
  -e EPS, --eps EPS
                        Convergence tolerance (used in pr; default: 1e-4)
```
---

## Examples

```bash
# BFS
python main_spla.py --algo bfs -m graph.mtx -s 0 -o bfs_result.txt

# SSSP
python main_spla.py --algo sssp -v graph.txt -s 0 -o sssp_result.txt

# PageRank
python main_spla.py --algo pr -v graph.txt -a 0.85 -e 1e-6 -o pr_result.txt

# Triangle Counting
python main_spla.py --algo tc -m graph.mtx -o tc_result.txt

# Classic reference version
python main_classic.py --algo bfs -v graph.txt -s 0 -o bfs_classic.txt
```

---

## Input format

Two options:

- `-v graph.txt` — Vectors format
- `-m graph.mtx` — Matrix Market format (from https://sparse.tamu.edu/)


### graph.txt

Example 
```text
3          # number of vertices n 
0 1 2      # I: source vertex indices
1 2 0      # J: target vertex indices
5 3 2      # V: edge weights 
```

### graph.mtx (Matrix Market)
Example 
```text
%%MatrixMarket matrix coordinate real general
% rows cols nnz
4 4 4      #rows, cols, non-zero values
1 2 1.0    #source vertex, target vertex, weight
2 3 2.0
3 4 3.0
4 1 4.0
```