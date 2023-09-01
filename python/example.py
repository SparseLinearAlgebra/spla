from pyspla import *

M = Matrix.from_lists([0, 1, 2, 2], [1, 2, 0, 4], [1, 2, 3, 4], (3, 5), INT)
print(M)

N = Matrix.from_lists([0, 1, 2, 3], [2, 0, 1, 3], [2, 3, 4, 5], (5, 4), INT)
print(N)

R = M.mxm(N, INT.MULT, INT.PLUS)
print(R)
