import pyspla

a = pyspla.Array.from_list(dtype=pyspla.INT, values=[-1, 0, 1])
print(a)

b = pyspla.Array.generate(dtype=pyspla.INT, shape=10, dist=(-10, 10))
print(b)

s = pyspla.Scalar(value=10, dtype=pyspla.INT)
print(s)

v = pyspla.Vector.from_lists([10, 20, 30], [-10, 100, 20], shape=100, dtype=pyspla.FLOAT)
print(v.to_list())

u = pyspla.Vector.generate(shape=100, dtype=pyspla.FLOAT, density=0.3)
print(u.to_list())

r = v.eadd(pyspla.FLOAT.MULT, u)
print(r.to_list())

m = pyspla.Vector.from_lists([0, 2, 5], [-1, 1, 1], shape=10, dtype=pyspla.INT)
t = pyspla.Vector(shape=10, dtype=pyspla.INT)
t.assign(m, pyspla.Scalar(pyspla.INT, 10), pyspla.INT.SECOND, pyspla.INT.GEZERO)
print(t.to_list())

G = pyspla.Matrix.generate((10, 10), pyspla.INT, density=0.1, dist=[0, 10])
print(G.to_lists())
print(G.reduce(pyspla.INT.PLUS))

M = pyspla.Matrix.from_lists([1, 2, 3], [1, 2, 3], [-1, 5, 10], (10, 10), pyspla.INT)
M.set_format(pyspla.FormatMatrix.ACC_CSR)
print(M.get(1, 0))
print(M.get(1, 1))
print(M.to_list())
