from pyspla import *

def split_into_lower_upper(A: Matrix):
    I, J, V = A.to_lists()
    L_I, L_J, L_V = [], [], []
    U_I, U_J, U_V = [], [], []

    for k in range(len(I)):
        i = I[k]
        j = J[k]
        val = V[k]
        if i < j:
            U_I.append(i)
            U_J.append(j)
            U_V.append(val)
        elif i > j:
            L_I.append(i)
            L_J.append(j)
            L_V.append(val)

    rows, cols = A.shape
    lower = Matrix.from_lists(L_I, L_J, L_V, shape=(rows, cols), dtype=INT)
    upper = Matrix.from_lists(U_I, U_J, U_V, shape=(rows, cols), dtype=INT)
    return lower, upper

def cohen(A: Matrix):
    L, U = split_into_lower_upper(A)
    B = L.mxm(M=U, op_mult=INT.MULT, op_add=INT.PLUS)
    C = A.emult(op_mult=INT.MULT, M=B)
    total_sum = C.reduce(op_reduce=INT.PLUS).get()
    return total_sum // 2