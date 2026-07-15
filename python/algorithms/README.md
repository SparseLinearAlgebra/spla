# Graph Algorithms with SPLA

This project provides two implementations of graph algorithms:

- **Classic (CPU)**: naive implementation in Python using adjacency lists.
- **SPLA (GPU)**: implementation using sparse linear algebra primitives from the SPLA library, targeting GPU execution.

Tests (`test_compare.py`) verify that both versions produce identical results.

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
 Create and activate a virtual environment:

```bash
cd python
python -m venv venv
source venv/bin/activate 
cd algorithms
```
## Usage

Two entry points are provided:

- `main_spla.py` – GPU-accelerated version (SPLA)
- `main_classic.py` – CPU reference version

Both scripts accept the same command-line arguments.

### Command Syntax

```bash
python main_spla.py --algo <bfs|sssp|pr|tc> (-m <file.mtx> | -v <file.txt>) [options]
```

### Required Arguments

| Argument | Description |
|----------|-------------|
| `--algo` | Algorithm to run: `bfs`, `sssp`, `pr`, `tc` |
| `-m` or `-v` | Input graph file. Use `-m` for Matrix Market (`.mtx`) or `-v` for vectors format (`.txt`). |

### Optional Arguments

| Option | Default | Description |
|--------|---------|-------------|
| `-o` | `result.txt` | Output file path |
| `-s` | `0` | Start vertex (for BFS and SSSP) |
| `-a` | `0.85` | Damping factor (for PageRank) |
| `-e` | `1e-4` | Convergence tolerance (for PageRank) |
| `-h`, `--help` | – | Show help message and exit |

---

## Examples

```bash
# BFS on a Matrix Market file
python main_spla.py --algo bfs -m graph.mtx -s 0 -o bfs_result.txt

# SSSP on a vectors file (weighted graph)
python main_spla.py --algo sssp -v weighted.txt -s 0 -o sssp_result.txt

# PageRank with custom parameters
python main_spla.py --algo pr -v graph.txt -a 0.85 -e 1e-6 -o pr_result.txt

# Triangle Counting
python main_spla.py --algo tc -m graph.mtx -o tc_result.txt

# Classic reference version
python main_classic.py --algo bfs -v graph.txt -s 0 -o bfs_classic.txt
```

