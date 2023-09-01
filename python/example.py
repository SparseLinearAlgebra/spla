from pyspla import *


def bfs(s: int, A: Matrix):
    v = Vector(A.n_rows, INT)  # to store depths

    front = Vector.from_lists([s], [1], A.n_rows, INT)  # front of new vertices to study
    front_size = 1  # current front size
    depth = Scalar(INT, 0)  # depth of search
    count = 0  # num of reached vertices

    while front_size > 0:  # while have something to study
        depth += 1
        count += front_size
        v.assign(front, depth, op_assign=INT.SECOND, op_select=INT.NQZERO)  # assign depths
        front = front.vxm(v, A, op_mult=INT.LAND, op_add=INT.LOR, op_select=INT.EQZERO)  # do traversal
        front_size = front.reduce(op_reduce=INT.PLUS).get()  # update front count to end algorithm

    return v, count, depth.get()


I = [0, 1, 2, 2, 3]
J = [1, 2, 0, 3, 2]
V = [1, 1, 1, 1, 1]
A = Matrix.from_lists(I, J, V, shape=(4, 4), dtype=INT)
print(A)

v, c, d = bfs(0, A)
print(v)
print(c)
print(d)

M = Matrix.from_lists([0, 1, 2, 3], [0, 3, 3, 2], [1, 2, 3, 4], (4, 4), INT)
print(M)
print(M.reduce_by_column(INT.PLUS))
