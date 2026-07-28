from pyspla import FLOAT, Matrix, Scalar, Vector

INF = float(1e9)


def sssp_spla(start: int, A: Matrix):
    """
    Relaxes distances via min-plus multiplication of the current vector by the adjacency matrix.
    Addition acts as multiplication, minimum acts as addition.
    """
    n = A.n_rows
    initial_indices = list(range(n))
    initial_values = [INF] * n
    initial_values[start] = 0.0
    dist = Vector.from_lists(initial_indices, initial_values, n, FLOAT)

    mask = Vector.dense(n, FLOAT, 1.0)
    inf_scalar = Scalar(FLOAT, INF)

    for _ in range(n - 1):
        new_dist = dist.vxm(
            mask, A, op_mult=FLOAT.PLUS, op_add=FLOAT.MIN, op_select=FLOAT.ALWAYS, init=inf_scalar
        )
        relaxed = dist.eadd(FLOAT.MIN, new_dist)
        indices, values = relaxed.to_lists()
        if start not in indices:
            indices.append(start)
            values.append(0.0)
        dist = Vector.from_lists(indices, values, n, FLOAT)

    final_indices, final_values = dist.to_lists()
    final_map = {i: INF for i in range(n)}
    for k in range(len(final_indices)):
        val = final_values[k]
        if val == 0.0 and final_indices[k] != start:
            val = INF
        final_map[final_indices[k]] = val

    all_indices = list(range(n))
    all_values = [final_map[i] for i in all_indices]
    return Vector.from_lists(all_indices, all_values, n, FLOAT)
