from pyspla import *

M = Matrix.from_lists([0, 1, 2], [3, 2, 0], [-5, 3, 9], (3, 4), INT)
print(M)

print(M.transpose())

print(M.transpose(op_apply=INT.UONE))

print(M.transpose(op_apply=INT.AINV))
