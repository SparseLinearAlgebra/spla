from pyspla import *

a = Array.from_list(dtype=INT, values=[-1, 0, 1])
print(a)

b = Array.generate(dtype=INT, shape=10, dist=(-10, 10))
print(b)

s = Scalar(value=10, dtype=INT)
print(s)

m = Vector.from_lists([0, 2, 5], [-1, 1, 1], shape=10, dtype=INT)
t = Vector(shape=10, dtype=INT)
t.assign(m, Scalar(INT, 10), INT.SECOND, INT.GEZERO)
print(t.to_list())

M = Matrix.generate((4, 4), INT, density=0.3, dist=[0, 10])
print(M)

M = Matrix.from_lists([1, 2, 3], [1, 2, 3], [-1, 5, 10], (4, 4), INT)
print(M)

v = Vector.generate(shape=4, dtype=INT, density=0.5, dist=[1, 10])
print(v)

v = Vector.from_lists([0, 1, 3], [-1, 7, 5], 4, INT)
print(v)
