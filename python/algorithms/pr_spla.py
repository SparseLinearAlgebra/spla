import math

from pyspla import FLOAT, Matrix, Scalar, Vector


def pagerank(A: Matrix, alpha: float, eps: float):
    """
    The function iteratively multiplies the rank vector by the transition matrix,
    adds a constant `(1-alpha)/N`, stops on change < eps. Returns the rank vector and iterations.
    """
    N = A.n_rows
    p = Vector.dense(N, FLOAT, 1.0 / N)
    addition = Vector.dense(N, FLOAT, (1.0 - alpha) / N)
    dummy_mask = Vector.dense(N, FLOAT, 1.0)
    zero = Scalar(FLOAT, 0.0)
    error = eps + 1.0
    iterations = 0

    while error > eps:
        p_prev = p
        p_tmp = p_prev.vxm(
            dummy_mask, A, op_mult=FLOAT.MULT, op_add=FLOAT.PLUS, op_select=FLOAT.ALWAYS, init=zero
        )
        p = p_tmp.eadd(FLOAT.PLUS, addition)
        diff = p.eadd(FLOAT.MINUS_POW2, p_prev)
        error2 = diff.reduce(FLOAT.PLUS, init=zero)
        error = math.sqrt(error2.get())
        iterations += 1

    return p, iterations
