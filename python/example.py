from pyspla import *

u = Vector.from_lists([0, 1], [10, 20], 4, INT)
v = Vector.from_lists([1, 3], [-5, 12], 4, INT)
print(u.emult(INT.PLUS, v))
