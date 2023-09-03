from pyspla import *

M = Matrix.from_lists([0, 0, 1, 2], [1, 2, 3, 0], [-1, 1, 2, 3], (3, 4), INT)
print(M)
print(M.extract_row(0))
print(M.extract_row(0, op_apply=INT.AINV))

M = Matrix.from_lists([0, 1, 1, 2], [1, 0, 3, 1], [-1, 1, 2, 3], (3, 4), INT)
print(M)
print(M.extract_column(1))
print(M.extract_column(1, op_apply=INT.AINV))
