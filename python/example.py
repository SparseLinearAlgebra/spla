import pyspla

a = pyspla.Array.from_list(dtype=pyspla.INT, values=[-1, 0, 1])
b = pyspla.Array.generate(dtype=pyspla.INT, shape=10, dist=(-10, 10))

print(a)
print(b)
