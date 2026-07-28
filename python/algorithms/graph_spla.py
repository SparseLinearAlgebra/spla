from pyspla import FLOAT, INT, Matrix


def read_mtx_int(filename):
    row_indices, col_indices, values = [], [], []
    n = 0
    with open(filename, "r") as f:
        for line in f:
            if line.startswith("%"):
                continue
            parts = line.split()
            if n == 0:
                n = int(parts[0])
                continue
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            row_indices.append(i)
            col_indices.append(j)
            values.append(1)
            if i != j:
                row_indices.append(j)
                col_indices.append(i)
                values.append(1)
    return Matrix.from_lists(row_indices, col_indices, values, shape=(n, n), dtype=INT)


def read_vectors_int(filename):
    with open(filename, "r") as f:
        n_line = f.readline().strip()
        if not n_line:
            return Matrix.from_lists([], [], [], shape=(0, 0), dtype=INT)
        n = int(n_line)
        rows = list(map(int, f.readline().split()))
        cols = list(map(int, f.readline().split()))
    row_indices, col_indices, values = [], [], []
    for k in range(len(rows)):
        i = rows[k]
        j = cols[k]
        row_indices.append(i)
        col_indices.append(j)
        values.append(1)
        if i != j:
            row_indices.append(j)
            col_indices.append(i)
            values.append(1)
    return Matrix.from_lists(row_indices, col_indices, values, shape=(n, n), dtype=INT)


def read_mtx_float(filename):
    row_indices, col_indices, values = [], [], []
    n = 0
    with open(filename, "r") as f:
        for line in f:
            if line.startswith("%"):
                continue
            parts = line.split()
            if n == 0:
                n = int(parts[0])
                continue
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            w = float(parts[2]) if len(parts) > 2 else 1.0
            row_indices.append(i)
            col_indices.append(j)
            values.append(w)
            if i != j:
                row_indices.append(j)
                col_indices.append(i)
                values.append(w)
    return Matrix.from_lists(row_indices, col_indices, values, shape=(n, n), dtype=FLOAT)


def read_vectors_float(filename):
    with open(filename, "r") as f:
        n_line = f.readline().strip()
        if not n_line:
            return Matrix.from_lists([], [], [], shape=(0, 0), dtype=FLOAT)
        n = int(n_line)
        rows = list(map(int, f.readline().split()))
        cols = list(map(int, f.readline().split()))
        weights = list(map(float, f.readline().split()))
    row_indices, col_indices, values = [], [], []
    for k in range(len(rows)):
        i = rows[k]
        j = cols[k]
        w = weights[k]
        row_indices.append(i)
        col_indices.append(j)
        values.append(w)
        if i != j:
            row_indices.append(j)
            col_indices.append(i)
            values.append(w)
    return Matrix.from_lists(row_indices, col_indices, values, shape=(n, n), dtype=FLOAT)


def read_mtx_pr(filename, alpha):
    row_indices, col_indices = [], []
    n = 0
    with open(filename, "r") as f:
        for line in f:
            if line.startswith("%"):
                continue
            parts = line.split()
            if n == 0:
                n = int(parts[0])
                out_degree = [0] * n
                continue
            i = int(parts[0]) - 1
            j = int(parts[1]) - 1
            row_indices.append(i)
            col_indices.append(j)
            out_degree[i] += 1
            if i != j:
                row_indices.append(j)
                col_indices.append(i)
                out_degree[j] += 1
    values = [alpha / out_degree[u] for u in row_indices]
    return Matrix.from_lists(row_indices, col_indices, values, shape=(n, n), dtype=FLOAT)


def read_vectors_pr(filename, alpha):
    with open(filename, "r") as f:
        n_line = f.readline().strip()
        if not n_line:
            return Matrix.from_lists([], [], [], shape=(0, 0), dtype=FLOAT)
        n = int(n_line)
        rows = list(map(int, f.readline().split()))
        cols = list(map(int, f.readline().split()))
    out_degree = [0] * n
    row_indices, col_indices = [], []
    for k in range(len(rows)):
        i = rows[k]
        j = cols[k]
        row_indices.append(i)
        col_indices.append(j)
        out_degree[i] += 1
        if i != j:
            row_indices.append(j)
            col_indices.append(i)
            out_degree[j] += 1
    values = [alpha / out_degree[u] for u in row_indices]
    return Matrix.from_lists(row_indices, col_indices, values, shape=(n, n), dtype=FLOAT)
