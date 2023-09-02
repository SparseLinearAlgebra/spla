from pyspla import *

M = Matrix.from_lists([0, 0, 1], [0, 1, 1], [1, 2, 3], (2, 2), INT)
print(M.eadd(INT.MULT, M.transpose()))

M = Matrix.from_lists([0, 0, 1], [0, 1, 1], [1, 2, 3], (2, 2), INT)
print(M.emult(INT.MULT, M.transpose()))
