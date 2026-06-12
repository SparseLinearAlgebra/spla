from pyspla import Matrix, Vector, FLOAT, Scalar

INF = 1e9

def sssp_broken(start: int, A: Matrix):
    N = A.n_rows
    v = Vector(N, FLOAT)
    v.set(start, 0.0)
    
    frontier = Vector(N, FLOAT)
    frontier.set(start, 0.0)
    
    dummy_mask = Vector.dense(N, FLOAT, 1.0)
    inf_scalar = Scalar(FLOAT, INF)
    
    step = 0
    front_size = 1 
    while front_size > 0 and step < N:
        frontier = frontier.vxm(dummy_mask, A, op_mult=FLOAT.PLUS, op_add=FLOAT.MIN, op_select=FLOAT.ALWAYS, init=inf_scalar)
        v = v.eadd(FLOAT.MIN, frontier)
        idx, vals = v.to_lists()
        print(f"Iteration {step} V: {sorted(list(zip(idx, vals)))}")
        idx2, vals2 = frontier.to_lists()
        print(f"Iteration {step} Frontier: {sorted(list(zip(idx2, vals2)))}")
        front_size = frontier.reduce(FLOAT.PLUS).get()
        print(f"Iteration {step} Frontier sum: {front_size}")
        print('-' * 24)
        step += 1
    
    return v