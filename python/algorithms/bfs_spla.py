from pyspla import INT, Matrix, Scalar, Vector, Descriptor

def bfs_spla(s: int, A: Matrix,
             push_only: bool = False,
             pull_only: bool = False,
             push_pull: bool = True,
             front_factor: float = 0.05):
    n = A.n_rows
    if not (push_only or pull_only or push_pull):
        push_only = True

    v = Vector(n, INT)
    front_prev = Vector(n, INT)
    front_new = Vector(n, INT)
    front_prev.set(s, 1)

    depth = Scalar(INT, 0)
    count = 0
    front_size = 1

    desc = Descriptor()
    desc.set_early_exit(True)
    desc.set_struct_only(True)

    while front_size > 0:
        depth += 1
        count += front_size
        v.assign(front_prev, depth, op_assign=INT.SECOND, op_select=INT.NQZERO)
        front_density = front_size / n
        is_push_better = front_density <= front_factor

        if push_only or (push_pull and is_push_better):
            front_prev.vxm(v, A,op_mult=INT.LAND, op_add=INT.LOR, op_select=INT.EQZERO, out=front_new, desc=desc)
        else:
            A.mxv(v, front_prev, op_mult=INT.LAND, op_add=INT.LOR, op_select=INT.EQZERO, out=front_new, desc=desc)

        front_size = front_new.count_mf().get()

        front_prev, front_new = front_new, front_prev

    return v, count, depth.get()