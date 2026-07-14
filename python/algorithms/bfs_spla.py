from pyspla import *

def bfs(s: int, A: Matrix):
    v = Vector(A.n_rows, INT)
    front = Vector.from_lists([s], [1], A.n_rows, INT)
    front_size = 1
    depth = Scalar(INT, 0)
    count = 0

    while front_size > 0:
        depth += 1
        count += front_size
        v.assign(front, depth, op_assign=INT.SECOND, op_select=INT.NQZERO)
        front = front.vxm(v, A, op_mult=INT.LAND, op_add=INT.LOR, op_select=INT.EQZERO)
        front_size = front.reduce(op_reduce=INT.PLUS).get()

    return v, count, depth.get()