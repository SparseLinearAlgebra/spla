import argparse
import sys
import math
from pathlib import Path
from bfs_spla import bfs_spla 
from sssp_spla import sssp_spla 
from pr_spla import pr_spla 
from tc_spla import tc_spla 
from graph_spla import (
    read_spla,
    read_vectors_spla,
    read_mtx_pr_spla,
    read_vectors_pr_spla,
)
from pyspla import INT, FLOAT

INF = math.inf

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--algo",
        choices=["bfs", "sssp", "pr", "tc"],
        required=True,
        help="Algorithm to run: "
             "bfs - Breadth-First Search, "
             "sssp - Single-Source Shortest Paths, "
             "pr - PageRank, "
             "tc - Triangle Counting",
    )
    parser.add_argument(
        "-m", "--matrix",
        type=Path,
        help="Path to graph in Matrix Market format (.mtx)",
    )
    parser.add_argument(
        "-v", "--vectors",
        type=Path,
        help="Path to graph in vectors format (.txt)",
    )
    parser.add_argument(
        "-o", "--output",
        type=Path,
        default="result.txt",
        help="Output file path",
    )
    parser.add_argument(
        "-s", "--start",
        type=int,
        default=0,
        help="Start vertex(default: 0)",
    )
    parser.add_argument(
        "-a", "--alpha",
        type=float,
        default=0.85,
        help="Damping factor(default: 0.85)",
    )
    parser.add_argument(
        "-e", "--eps",
        type=float,
        default=1e-4,
        help="Convergence tolerance(default: 1e-4)",
    )
    args = parser.parse_args()

    if not args.matrix and not args.vectors:
        sys.exit("Error: no graph file provided")

    if args.algo == "bfs":
        if args.matrix:
            A = read_spla(str(args.matrix), dtype=INT)
            v, count, depth = bfs_spla(args.start, A)
        else:
            A = read_vectors_spla(str(args.vectors), dtype=INT)
            v, count, depth = bfs_spla(args.start, A, push_only=True, pull_only=False, push_pull=False)
        with open(args.output, "w") as f_out:
            f_out.write(f"Reached vertices: {count}\n")
            f_out.write(f"Max depth: {depth - 1}\n")
            idx, vals = v.to_lists()
            for k in range(len(idx)):
                f_out.write(f"{idx[k]} {vals[k] - 1}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "sssp":
        if args.matrix:
            A = read_spla(str(args.matrix), dtype=FLOAT)
            v = sssp_spla(args.start, A)
        else:
            A = read_vectors_spla(str(args.vectors), dtype=FLOAT)
             v = sssp_spla(args.start, A, push_only=True, pull_only=False, push_pull=False)
        with open(args.output, "w") as f_out:
            idx, vals = v.to_lists()
            for k in range(len(idx)):
                if vals[k] < INF:
                    f_out.write(f"{idx[k]} {vals[k]}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "pr":
        if args.matrix:
            A = read_mtx_pr_spla(str(args.matrix), alpha=args.alpha)
        else:
            A = read_vectors_pr_spla(str(args.vectors), alpha=args.alpha)
        p = pr_spla(A, args.alpha, args.eps)
        with open(args.output, "w") as f_out:
            idx, vals = p.to_lists()
            for k in range(len(idx)):
                f_out.write(f"{idx[k]} {vals[k]:.6f}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "tc":
        if args.matrix:
            A = read_spla(str(args.matrix), dtype=INT)
        else:
            A = read_vectors_spla(str(args.vectors), dtype=INT)
        triangles = tc_spla(A)
        with open(args.output, "w") as f_out:
            f_out.write(f"Triangles: {triangles}\n")
        print(f"Result saved to {args.output}")

if __name__ == "__main__":
    main()