from pyspla import *

v = Vector.from_lists([0, 1, 3], [5, -1, 3], 4, INT)
print(v.map(INT.AINV))
