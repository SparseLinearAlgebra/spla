from pyspla import INT, Matrix, Scalar

def tc_spla(A: Matrix):
    zero = Scalar(INT, 0)
    B = A.mxmT(A, A, op_mult=INT.MULT, op_add=INT.PLUS, op_select=INT.GTZERO, init=zero)
    return B.reduce(op_reduce=INT.PLUS, init=zero).get()