from pyspla import INT, Matrix, Scalar


def _strict_lower(A: Matrix) -> Matrix:
    """
    Strict lower triangle of an undirected adjacency matrix with unit weights.

    Robust to the storage orientation of A (upper, lower or symmetric) and
    to duplicated edges: each undirected edge {i, j} is represented once,
    as (max(i, j), min(i, j)).
    """
    i, j, _ = A.to_lists()
    edges = sorted({(a, b) if a > b else (b, a) for a, b in zip(i, j) if a != b})
    li = [a for a, b in edges]
    lj = [b for a, b in edges]
    return Matrix.from_lists(li, lj, [1] * len(li), shape=A.shape, dtype=INT)


def tc_sandia(A: Matrix) -> int:
    """
    Triangle count, Sandia algorithm (M.M. Wolf et al., HPEC 2017):

        L = tril(A)           strict lower triangle, unit weights
        C = L * L  (mask L)   C[i, j] = number of k with i > k > j and i~k, k~j
        t = sigma(C)          every triangle is counted exactly once

    A: undirected graph adjacency matrix, any edge orientation.
    Edge weights of A are ignored (triangles are counted, not weighted).
    """
    L = _strict_lower(A)
    zero = Scalar(INT, 0)
    C = L.mxmT(
        L,
        L.transpose(),
        op_mult=INT.MULT,
        op_add=INT.PLUS,
        op_select=INT.GTZERO,
        init=zero,
    )
    return C.reduce(op_reduce=INT.PLUS, init=zero).get()


def tc_spla(A: Matrix) -> int:
    """
    Triangle count (default implementation: Sandia algorithm, see tc_sandia).
    """
    return tc_sandia(A)


def tc_spla_mxmT(A: Matrix) -> int:
    """
    Triangle count via masked A * A^T product (mirrors C++ spla::tc):

        C = A * A^T  (mask A)   C[i, j] = number of common neighbors of i, j
                                that are greater than both
        t = sigma(C)            every triangle is counted exactly once

    A: upper triangle of the undirected graph (entries with i < j only),
    unit weights.
    """
    zero = Scalar(INT, 0)
    B = A.mxmT(A, A, op_mult=INT.MULT, op_add=INT.PLUS, op_select=INT.GTZERO, init=zero)
    return B.reduce(op_reduce=INT.PLUS, init=zero).get()
