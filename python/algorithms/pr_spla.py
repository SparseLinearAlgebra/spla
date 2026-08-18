import math
from pyspla import FLOAT, Matrix, Scalar, Vector

def pr(A: Matrix, alpha: float, eps: float):
    N = A.n_rows
    dummy_mask = Vector(N, FLOAT)
    addition = Vector.dense(N, FLOAT, (1.0 - alpha) / N)
    p_prev = Vector.dense(N, FLOAT, 1.0 / N)
    init_zero = Scalar(FLOAT, 0.0)
    error = eps + 0.1
    while error > eps:
        p_tmp = A.mxv(
            dummy_mask,
            p_prev,
            op_mult=FLOAT.MULT,
            op_add=FLOAT.PLUS,
            op_select=FLOAT.ALWAYS,
            init=init_zero
        )
        p = p_tmp.eadd(FLOAT.PLUS, addition)
        errors = p.eadd(FLOAT.MINUS_POW2, p_prev)
        error2 = errors.reduce(FLOAT.PLUS)
        error = math.sqrt(error2.get())
        p, p_prev = p_prev, p
        
    return p_prev