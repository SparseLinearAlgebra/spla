from pyspla import *

M = Matrix.from_lists([0, 1, 2, 2], [1, 2, 0, 4], [1, 2, 3, 4], (3, 5), INT)
print(M)

N = Matrix.from_lists([0, 1, 2, 3], [1, 2, 0, 3], [2, 3, 4, 5], (4, 5), INT)
print(N)

mask = Matrix.dense((3, 4), INT, fill_value=1)
print(mask)

R = M.mxmT(mask, N, INT.MULT, INT.PLUS, INT.GTZERO)
print(R)

R = M.mxmT(mask, N, INT.MULT, INT.PLUS, INT.EQZERO)
print(R)