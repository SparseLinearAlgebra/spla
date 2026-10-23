import math
from pyspla import FLOAT, Matrix, Scalar, Vector


def pr_spla(A: Matrix, alpha: float, eps: float):
    N = A.n_rows
    mask = Vector(N, FLOAT)
    addition = Vector(N, FLOAT).fill_with(Scalar(FLOAT, (1.0 - alpha) / N))
    p_prev   = Vector(N, FLOAT).fill_with(Scalar(FLOAT, 1.0 / N))
    p_tmp  = Vector(N, FLOAT)
    p      = Vector(N, FLOAT)
    errors = Vector(N, FLOAT)
    zero   = Scalar(FLOAT, 0.0)
    error2 = Scalar(FLOAT)
    error = eps + 0.1
    while error > eps:
        A.mxv(mask, p_prev,
              op_mult=FLOAT.MULT, op_add=FLOAT.PLUS, op_select=FLOAT.ALWAYS,
              init=zero, out=p_tmp)
        p_tmp.eadd(FLOAT.PLUS, addition, out=p)
        p.eadd(FLOAT.MINUS_POW2, p_prev, out=errors)
        errors.reduce(FLOAT.PLUS, out=error2, init=zero)
        error = math.sqrt(error2.get())
        p, p_prev = p_prev, p

    return p_prev