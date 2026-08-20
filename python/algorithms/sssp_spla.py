import math
from pyspla import FLOAT, Matrix, Scalar, Vector

INF = math.inf
EPS = 1e-12

def clone_vector(v: Vector):
    idx, vals = v.to_lists()
    return Vector.from_lists(idx, vals, v.n_rows, v.dtype)

def is_converged(old, new, eps=EPS):
    diff = old.eadd(FLOAT.MINUS_POW2, new)
    error2 = diff.reduce(FLOAT.PLUS)
    error = math.sqrt(error2.get())
    return error < eps

def sssp(start: int, A: Matrix, eps=EPS):
    n = A.n_rows
    dist = Vector.dense(n, FLOAT, INF)
    dist.set(start, 0.0)
    mask = Vector.dense(n, FLOAT, 1.0)
    init_inf = Scalar(FLOAT, INF)
    while True:
        prev = clone_vector(dist)
        new = A.mxv(
            mask,
            dist,
            op_mult=FLOAT.PLUS,
            op_add=FLOAT.MIN,
            op_select=FLOAT.ALWAYS,
            init=init_inf
        )
        dist = dist.eadd(FLOAT.MIN, new)
        if is_converged(prev, dist, eps):
            break
    idx, vals = dist.to_lists()
    return dist