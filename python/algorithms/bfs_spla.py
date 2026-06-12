import sys
import argparse
from pathlib import Path
from pyspla import *

def bfs(s: int, A: Matrix):
    v = Vector(A.n_rows, INT)
    front = Vector.from_lists([s], [1], A.n_rows, INT)
    front_size = 1
    depth = Scalar(INT, 0)
    count = 0

    while front_size > 0:
        depth += 1
        count += front_size
        v.assign(front, depth, op_assign=INT.SECOND, op_select=INT.NQZERO)
        front = front.vxm(v, A, op_mult=INT.LAND, op_add=INT.LOR, op_select=INT.EQZERO)
        front_size = front.reduce(op_reduce=INT.PLUS).get()

    return v, count, depth.get()

def read_vectors(filename):
    with open(filename) as f:
        I = list(map(int, f.readline().replace(',', ' ').split()))
        J = list(map(int, f.readline().replace(',', ' ').split()))
        V = list(map(int, f.readline().replace(',', ' ').split()))
    rows = int(max(I) + 1)
    cols = int(max(J) + 1)
    return Matrix.from_lists(I, J, V, shape=(rows, cols), dtype=INT)

def read_mtx(filename):
    I, J, V = [], [], []
    n_vertices = 0
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('%'):
                continue
            parts = line.split()
            if n_vertices == 0:
                n_vertices = int(parts[0])
                continue
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            I.append(i); J.append(j); V.append(1)
            if i != j:
                I.append(j); J.append(i); V.append(1)
    return Matrix.from_lists(I, J, V, shape=(n_vertices, n_vertices), dtype=INT)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--matrix", type=Path)
    parser.add_argument("-v", "--vectors", type=Path)
    parser.add_argument("-s", "--start", type=int, default=0)
    parser.add_argument("-o", "--output", type=Path, default="bfs_spla_result.txt")
    args = parser.parse_args()

    if args.matrix and args.matrix.exists():
        A = read_mtx(str(args.matrix))
    elif args.vectors and args.vectors.exists():
        A = read_vectors(str(args.vectors))
    else:
        sys.exit(1)

    v, count, depth = bfs(args.start, A)

    with open(args.output, 'w') as f_out:
        f_out.write(f"Reached vertices: {count}\n")
        f_out.write(f"Max depth: {depth - 1}\n")
        indices, values = v.to_lists()
        for i, dist in zip(indices, values):
            f_out.write(f"{i} {dist - 1}\n")

    print(f"Result written to {args.output}")

if __name__ == "__main__":
    import os
    main()