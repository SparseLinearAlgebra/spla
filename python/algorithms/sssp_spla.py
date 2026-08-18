import math
from pyspla import FLOAT, Matrix, Scalar, Vector

INF = math.inf
def clone_vector(v: Vector):
    idx, vals = v.to_lists()
    return Vector.from_lists(idx, vals, v.n_rows, v.dtype)

def has_changed(prev: Vector, new: Vector):
    prev_idx, prev_vals = prev.to_lists()
    new_idx, new_vals = new.to_lists()
    if prev_idx != new_idx or prev_vals != new_vals:
        return True
    return False


def sssp(start: int, A: Matrix):
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
        if not has_changed(prev, dist):
            break

    return dist