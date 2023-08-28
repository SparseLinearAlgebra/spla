import pyspla

a = pyspla.Array.from_list(dtype=pyspla.INT, values=[-1, 0, 1])
print(a)

b = pyspla.Array.generate(dtype=pyspla.INT, shape=10, dist=(-10, 10))
print(b)

s = pyspla.Scalar(value=10, dtype=pyspla.INT)
print(s)
