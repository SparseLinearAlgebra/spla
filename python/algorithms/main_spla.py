import sys
import argparse
from pathlib import Path

from bfs_spla import bfs
from sssp_spla import sssp, INF
from pr_spla import pagerank
from tc_spla import cohen

from graph_spla import read_mtx_int, read_vectors_int
from graph_spla import read_mtx_float, read_vectors_float
from graph_spla import read_mtx_pr, read_vectors_pr

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--algo", choices=["bfs", "sssp", "pr", "tc"], required=True)
    parser.add_argument("-m", "--matrix", type=Path)
    parser.add_argument("-v", "--vectors", type=Path)
    parser.add_argument("-o", "--output", type=Path, default="result.txt")
    parser.add_argument("-s", "--start", type=int, default=0)
    parser.add_argument("-a", "--alpha", type=float, default=0.85)
    parser.add_argument("-e", "--eps", type=float, default=1e-4)
    args = parser.parse_args()

    if not args.matrix and not args.vectors:
        sys.exit("Error: no graph file provided")

    if args.algo == "bfs":
        if args.matrix:
            A = read_mtx_int(str(args.matrix))
        else:
            A = read_vectors_int(str(args.vectors))
        v, count, depth = bfs(args.start, A)
        with open(args.output, 'w') as f_out:
            f_out.write(f"Reached vertices: {count}\n")
            f_out.write(f"Max depth: {depth - 1}\n")
            idx, vals = v.to_lists()
            for k in range(len(idx)):
                f_out.write(f"{idx[k]} {vals[k] - 1}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "sssp":
        if args.matrix:
            A = read_mtx_float(str(args.matrix))
        else:
            A = read_vectors_float(str(args.vectors))
        v = sssp(args.start, A)
        with open(args.output, 'w') as f_out:
            idx, vals = v.to_lists()
            for k in range(len(idx)):
                if vals[k] < INF:
                    f_out.write(f"{idx[k]} {vals[k]}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "pr":
        if args.matrix:
            A = read_mtx_pr(str(args.matrix), args.alpha)
        else:
            A = read_vectors_pr(str(args.vectors), args.alpha)
        p, iters = pagerank(A, args.alpha, args.eps)
        with open(args.output, 'w') as f_out:
            f_out.write(f"Iterations: {iters}\n")
            idx, vals = p.to_lists()
            for k in range(len(idx)):
                f_out.write(f"{idx[k]} {vals[k]:.6f}\n")
        print(f"Result saved to {args.output}")

    elif args.algo == "tc":
        if args.matrix:
            A = read_mtx_int(str(args.matrix))
        else:
            A = read_vectors_int(str(args.vectors))
        triangles = cohen(A)
        with open(args.output, 'w') as f_out:
            f_out.write(f"Triangles: {triangles}\n")
        print(f"Result saved to {args.output}")

if __name__ == "__main__":
    main()