import math
from pyspla import FLOAT, Matrix, Scalar, Vector
INF = math.inf


def make_vector(n):
    v = Vector(n, FLOAT)
    v.set_fill_value(Scalar(FLOAT, INF))
    return v


def sssp_spla(start, A, push_only=False, pull_only=False, push_pull=True, front_factor=0.05):
    n = A.n_rows
    if not (push_only or pull_only or push_pull):
        push_only = True

    mask = Vector(n, FLOAT)
    init_inf = Scalar(FLOAT, INF)
    dist = make_vector(n)
    dist.set(start, 0.0)
    feedback = make_vector(n)
    feedback.set(start, 0.0)
    frontier = make_vector(n)
    fs = feedback.count_mf().get()

    while fs > 0:
        front_density = fs / n
        is_push_better = front_density <= front_factor

        if push_only or (push_pull and is_push_better):
            feedback.vxm(mask, A,
                         op_mult=FLOAT.PLUS, op_add=FLOAT.MIN,
                         op_select=FLOAT.ALWAYS, init=init_inf, out=frontier)
        else:
            A.mxv(mask, feedback,
                  op_mult=FLOAT.PLUS, op_add=FLOAT.MIN,
                  op_select=FLOAT.ALWAYS, init=init_inf, out=frontier)

        dist.eadd_fdb(frontier, FLOAT.MIN, feedback)
        fs = feedback.count_mf().get()

    return dist