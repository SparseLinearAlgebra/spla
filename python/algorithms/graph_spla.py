from scipy.io import mmread
from pyspla import Matrix, INT, FLOAT

def read_spla(filepath, dtype=FLOAT):
    coo = mmread(filepath).tocoo()
    n = int(max(coo.shape))
    rows = coo.row.astype(int).tolist()
    cols = coo.col.astype(int).tolist()
    vals = coo.data.astype(float).tolist()
    if dtype == INT:
        vals = [int(v) for v in vals]
    return Matrix.from_lists(rows, cols, vals, shape=(n, n), dtype=dtype)

def read_vectors_spla(filepath, dtype=FLOAT):
    with open(filepath) as f:
        I = list(map(int, f.readline().split()))
        J = list(map(int, f.readline().split()))
        V = list(map(float, f.readline().split()))
    if dtype == INT:
        V = [int(v) for v in V]
    n = max(max(I), max(J)) + 1
    return Matrix.from_lists(I, J, V, shape=(n, n), dtype=dtype)

def read_mtx_pr_spla(file_path, alpha=0.85):
    coo = mmread(file_path).tocoo()
    n = int(max(coo.shape))
    rows = coo.row.astype(int).tolist()
    cols = coo.col.astype(int).tolist()
    out_deg = [0] * n
    for i in rows:
        out_deg[i] += 1
    weights = []
    for i in rows:
        if out_deg[i] > 0:
            weights.append(alpha / out_deg[i])
        else:
            weights.append(0.0)
    return Matrix.from_lists(rows, cols, weights, shape=(n, n), dtype=FLOAT)

def read_vectors_pr_spla(file_path, alpha=0.85):
    with open(file_path) as f:
        I = list(map(int, f.readline().split()))
        J = list(map(int, f.readline().split()))
    n = max(max(I), max(J)) + 1
    out_deg = [0] * n
    for i in I:
        out_deg[i] += 1
    V = []
    for i in I:
        if out_deg[i] > 0:
            V.append(alpha / out_deg[i])
        else:
            V.append(0.0)
    return Matrix.from_lists(I, J, V, shape=(n, n), dtype=FLOAT)