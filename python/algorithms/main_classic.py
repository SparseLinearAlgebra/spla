import argparse
import sys
from pathlib import Path

from bfs_classic import INF as BFS_INF
from bfs_classic import bfs
from graph_classic import (
    read_mtx_pr_classic,
    read_mtx_unweighted,
    read_mtx_weighted,
    read_vectors_pr_classic,
    read_vectors_unweighted,
    read_vectors_weighted,
)
from pr_classic import pagerank_classic
from sssp_classic import INF as SSSP_INF
from sssp_classic import sssp
from tc_classic import tc_simple


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--algo", choices=["bfs", "sssp", "pr", "tc"], required=True)
    parser.add_argument("-m", "--matrix", type=Path)
    parser.add_argument("-v", "--vectors", type=Path)
    parser.add_argument("-o", "--output", type=Path, default="result_classic.txt")
    parser.add_argument("-s", "--start", type=int, default=0)
    parser.add_argument("-a", "--alpha", type=float, default=0.85)
    parser.add_argument("-e", "--eps", type=float, default=1e-4)
    args = parser.parse_args()

    if not args.matrix and not args.vectors:
        sys.exit("Error: no graph file provided")

    if args.algo == "bfs":
        if args.matrix:
            graph, n = read_mtx_unweighted(str(args.matrix))
        else:
            graph, n = read_vectors_unweighted(str(args.vectors))
        distances = bfs(args.start, graph, n)
        with open(args.output, "w") as f_out:
            for i in range(len(distances)):
                if distances[i] != BFS_INF:
                    f_out.write(f"{i} {distances[i]}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "sssp":
        if args.matrix:
            graph, n = read_mtx_weighted(str(args.matrix))
        else:
            graph, n = read_vectors_weighted(str(args.vectors))
        distances = sssp(args.start, graph, n)
        with open(args.output, "w") as f_out:
            for i in range(len(distances)):
                if distances[i] != SSSP_INF:
                    f_out.write(f"{i} {distances[i]}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "pr":
        if args.matrix:
            adj_in, out_degree, n = read_mtx_pr_classic(str(args.matrix))
        else:
            adj_in, out_degree, n = read_vectors_pr_classic(str(args.vectors))
        p, iters = pagerank_classic(adj_in, out_degree, n, args.alpha, args.eps)
        with open(args.output, "w") as f_out:
            f_out.write(f"Iterations: {iters}\n")
            for i in range(len(p)):
                f_out.write(f"{i} {p[i]:.6f}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "tc":
        if args.matrix:
            graph, n = read_mtx_unweighted(str(args.matrix))
        else:
            graph, n = read_vectors_unweighted(str(args.vectors))
        for i in range(n):
            graph[i].sort()
        triangles = tc_simple(graph, n)
        with open(args.output, "w") as f_out:
            f_out.write(f"Triangles: {triangles}\n")
        print(f"Result saved to {args.output}")


if __name__ == "__main__":
    main()
