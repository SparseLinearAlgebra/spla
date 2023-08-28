import pyspla

print(pyspla.Array.generate(dtype=pyspla.FLOAT, shape=3, dist=[100, 300]).to_list())
