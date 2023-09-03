"""
Wrapped native (spla C API) matrix primitive implementation.
"""

__copyright__ = "Copyright (c) 2021-2023 SparseLinearAlgebra"

__license__ = """
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import ctypes

from .bridge import backend, check
from .type import Type, INT, UINT, FLOAT
from .object import Object
from .memview import MemView
from .scalar import Scalar
from .descriptor import Descriptor
from .op import OpUnary, OpBinary, OpSelect
import random as rnd


class Matrix(Object):
    """
    Generalized statically-typed sparse storage-invariant matrix primitive.

    Notes
    -----

    Matrix typical usage:

    - Instantiate matrix primitive
    - Build incrementally from yours data source
    - Matrix usage in a sequence of math operations
    - Read-back matrix data to python to analyse results

    Steps (2) and (4) requires internal format transformations and possible transfer of data
    from acc (GPU) side if acceleration was employed in computations. These steps may be very
    intensive, so you have to avoid them in critical parts of computations. If you need faster
    data reads, prefer usage of batched reads, where all content of storage read at once.

    Details
    -------

    Matrix class support all spla C API matrix functions.
    It provides bind functionality as well as new functions/methods for better python user experience.

    Matrix internally manages optimal format of stored data. Use hints to
    force matrix state and format changes.

    Matrix optional uses dedicated/integrated GPU to speedup computations
    using one of built-in OpenCL or CUDA accelerators.
    """

    __slots__ = ["_dtype", "_shape"]

    def __init__(self, shape, dtype=INT, hnd=None, label=None):
        """
        Creates new matrix of specified type and shape.

        >>> M = Matrix((4, 5), INT)
        >>> print(M)
        '
            0 1 2 3 4
         0| . . . . .|  0
         1| . . . . .|  1
         2| . . . . .|  2
         3| . . . . .|  3
            0 1 2 3 4
        '

        :param dtype: Type.
            Type parametrization of a storage.

        :param shape: tuple.
            Matrix size (2-dim).

        :param hnd: optional: ctypes.c_void_p. default: None.
            Optional native handle to retain.

        :param label: optional: str. default: None.
            Optional label to assign.
        """

        super().__init__(None, None)

        assert dtype
        assert shape
        assert shape[0] > 0 and shape[1] > 0
        assert issubclass(dtype, Type)

        self._dtype = dtype
        self._shape = shape

        if not hnd:
            hnd = ctypes.c_void_p(0)
            check(backend().spla_Matrix_make(ctypes.byref(hnd),
                                             ctypes.c_uint(shape[0]),
                                             ctypes.c_uint(shape[1]),
                                             dtype._hnd))

        super().__init__(label, hnd)

    @property
    def dtype(self):
        """
        Type used for storage parametrization of this container.

        >>> M = Matrix((4, 5), INT)
        >>> print(M.dtype)
        '
            <class 'pyspla.type.INT'>
        '
        """
        return self._dtype

    @property
    def n_rows(self):
        """
        Number of rows in the matrix.

        >>> M = Matrix((4, 5), INT)
        >>> print(M.n_rows)
        '
            4
        '
        """
        return self._shape[0]

    @property
    def n_cols(self):
        """
        Number of cols in the matrix.

        >>> M = Matrix((4, 5), INT)
        >>> print(M.n_cols)
        '
            5
        '
        """
        return self._shape[1]

    @property
    def shape(self):
        """
        2-Tuple with shape of matrix.

        >>> M = Matrix((4, 5), INT)
        >>> print(M.shape)
        '
            (4, 5)
        '
        """

        return self._shape

    def set_format(self, fmt):
        """
        Instruct container to format internal data with desired storage format.
        Multiple different formats may be set at same time, data will be duplicated in different formats.
        If selected data already in a selected format, then nothing to do.

        See `FormatMatrix` enumeration for all supported formats.

        :param fmt: FormatMatrix.
            One of built-in storage formats to set.
        """

        check(backend().spla_Matrix_set_format(self.hnd, ctypes.c_int(fmt.value)))

    def set(self, i, j, v):
        """
        Set value at specified index

        >>> M = Matrix((4, 4), INT)
        >>> M.set(0, 0, -1)
        >>> M.set(1, 2, 4)
        >>> M.set(3, 1, 10)
        >>> print(M)
        '
            0 1 2 3
         0|-1 . . .|  0
         1| . . 4 .|  1
         2| . . . .|  2
         3| .10 . .|  3
            0 1 2 3
        '

        :param i: uint.
            Row index to set.

        :param j: uint.
            Column index to set.

        :param v: any.
            Value to set.
        """

        check(self._dtype._matrix_set(self.hnd, ctypes.c_uint(i), ctypes.c_uint(j), self._dtype._c_type(v)))

    def get(self, i, j):
        """
        Get value at specified index.

        >>> M = Matrix.from_lists([1, 2, 3, 3], [0, 1, 0, 3], [-1, -4, 4, 2], (4, 4), INT)
        >>> print(M.get(1, 0))
        '
            -1
        '

        >>> M = Matrix.from_lists([1, 2, 3, 3], [0, 1, 0, 3], [-1, -4, 4, 2], (4, 4), INT)
        >>> print(M.get(1, 3))
        '
            0
        '

        :param i: uint.
            Row index of value to get.

        :param j: uint.
            Column index of value to get.

        :return: Value.
        """

        c_value = self._dtype._c_type(0)
        check(self._dtype._matrix_get(self.hnd, ctypes.c_uint(i), ctypes.c_uint(j), ctypes.byref(c_value)))
        return self._dtype.cast_value(c_value)

    def build(self, view_I: MemView, view_J: MemView, view_V: MemView):
        """
        Builds matrix content from a raw memory view resources.

        :param view_I: MemView.
            View to keys of matrix to assign.

        :param view_J: MemView.
            View to keys of matrix to assign.

        :param view_V: MemView.
            View to actual values to store.
        """

        assert view_I
        assert view_J
        assert view_V

        check(backend().spla_Matrix_build(self.hnd, view_I.hnd, view_J.hnd, view_V.hnd))

    def read(self):
        """
        Read the content of the matrix as a MemView of I, J and V.

        :return: tuple (MemView, MemView, MemView) objects with view to the matrix keys and matrix values.
        """

        view_I_hnd = ctypes.c_void_p(0)
        view_J_hnd = ctypes.c_void_p(0)
        view_V_hnd = ctypes.c_void_p(0)
        check(backend().spla_Matrix_read(self.hnd,
                                         ctypes.byref(view_I_hnd),
                                         ctypes.byref(view_J_hnd),
                                         ctypes.byref(view_V_hnd)))
        return MemView(hnd=view_I_hnd, owner=self), \
               MemView(hnd=view_J_hnd, owner=self), \
               MemView(hnd=view_V_hnd, owner=self)

    def clear(self):
        """
        Clears matrix removing all elements, so it has no values.
        """

        check(backend().spla_Matrix_clear(self.hnd))

    def to_lists(self):
        """
        Read matrix data as a python lists of I, J and V.

        >>> M = Matrix.from_lists([1, 2, 3, 3], [0, 1, 0, 3], [-1, -4, 4, 2], (4, 4), INT)
        >>> print(M.to_lists())
        '
            ([1, 2, 3, 3], [0, 1, 0, 3], [-1, -4, 4, 2])
        '

        :return: Tuple (List, List, List) with the matrix keys and matrix values.
        """

        I, J, V = self.read()
        count = int(I.size / ctypes.sizeof(UINT._c_type))

        if count == 0:
            return [], [], []

        buffer_I = (UINT._c_type * count)()
        buffer_J = (UINT._c_type * count)()
        buffer_V = (self._dtype._c_type * count)()

        check(backend().spla_MemView_read(I.hnd, ctypes.c_size_t(0), ctypes.sizeof(buffer_I), buffer_I))
        check(backend().spla_MemView_read(J.hnd, ctypes.c_size_t(0), ctypes.sizeof(buffer_J), buffer_J))
        check(backend().spla_MemView_read(V.hnd, ctypes.c_size_t(0), ctypes.sizeof(buffer_V), buffer_V))

        return list(buffer_I), list(buffer_J), list(buffer_V)

    def to_list(self):
        """
        Read matrix data as a python lists of tuples where key and value stored together.

        >>> M = Matrix.from_lists([1, 2, 3, 3], [0, 1, 0, 3], [-1, -4, 4, 2], (4, 4), INT)
        >>> print(M.to_list())
        '
            [(1, 0, -1), (2, 1, -4), (3, 0, 4), (3, 3, 2)]
        '

        :return: List of matrix entries as (I, J, V).
        """

        I, J, V = self.to_lists()
        return list(zip(I, J, V))

    def to_string(self, format_string="{:>%s}", width=2, precision=2, skip_value=0, cell_sep=""):
        """
        Generate from a vector a pretty string for a display.

        >>> M = Matrix.from_lists([1, 2, 3], [1, 2, 3], [-1, 5, 10], (4, 4), INT)
        >>> print(M)
        '
            0 1 2 3
         0| . . . .|  0
         1| .-1 . .|  1
         2| . . 5 .|  2
         3| . . .10|  3
            0 1 2 3
        '

        :param format_string: str.
            How to format single value.

        :param width: int.
            Integral part length.

        :param precision: int.
            Fractional part length.

        :param skip_value: any.
            Value to skip and not display

        :param cell_sep: str.
            How to separate values in a row.

        :return: Pretty string with vector content.
        """

        format_string = format_string % width
        header = format_string.format("") + " " + "".join(format_string.format(i) for i in range(self.n_cols))

        result = header + "\n"
        for row in range(self.n_rows):
            result += format_string.format(row) + "|"
            for col in range(self.n_cols):
                value = self.get(row, col)
                value = value if value != skip_value else "."
                result += cell_sep + self.dtype.format_value(value, width, precision)
            result += "|  " + str(row) + "\n"
        result += header

        return result

    @classmethod
    def from_lists(cls, I: list, J: list, V: list, shape, dtype=INT):
        """
        Build matrix from a list of sorted keys and associated values to store in matrix.
        List with keys `I` and `J` must index entries from range [0, shape-1] and all keys must be sorted.

        >>> M = Matrix.from_lists([1, 2, 3], [1, 2, 3], [-1, 5, 10], (4, 4), INT)
        >>> print(M)
        '
            0 1 2 3
         0| . . . .|  0
         1| .-1 . .|  1
         2| . . 5 .|  2
         3| . . .10|  3
            0 1 2 3
        '

        :param I: list[UINT].
             List with integral keys of entries.

        :param J: list[UINT].
             List with integral keys of entries.

        :param V: list[Type].
             List with values to store in the matrix.

        :param shape: tuple.
             Matrix size.

        :param dtype:
             Type of storage parametrization for matrix.

        :return: Created matrix filled with values.
        """

        assert len(I) == len(V)
        assert len(J) == len(V)
        assert shape
        assert shape[0] > 0 and shape[1] > 0

        if not I:
            return Matrix(shape, dtype)

        count = len(I)

        c_I = (UINT._c_type * count)(*I)
        c_J = (UINT._c_type * count)(*J)
        c_V = (dtype._c_type * count)(*V)

        view_I = MemView(buffer=c_I, size=ctypes.sizeof(c_I), mutable=False)
        view_J = MemView(buffer=c_J, size=ctypes.sizeof(c_J), mutable=False)
        view_V = MemView(buffer=c_V, size=ctypes.sizeof(c_V), mutable=False)

        M = Matrix(shape=shape, dtype=dtype)
        M.build(view_I, view_J, view_V)

        return M

    @classmethod
    def rand(cls, shape, dtype=INT, density=0.1, seed=None, dist=(0, 1)):
        """
        Creates new matrix of desired type and shape and fills its content
        with random values, generated using specified distribution.

        >>> M = Matrix.rand((4, 4), INT, density=0.3, dist=[0, 10])
        >>> print(M)
        '
            0 1 2 3
         0| . 4 . 5|  0
         1| . 7 . .|  1
         2| . . . .|  2
         3| . . . 2|  3
            0 1 2 3
        '

        :param shape: tuple.
            Size of the matrix (number of values).

        :param dtype: optional: Type. default: INT.
            Type of values matrix will have.

        :param density: optional: float. default: 0.1.
            Density of matrix or how many entries to generate.

        :param seed: optional: int. default: None.
            Optional seed to randomize generator.

        :param dist: optional: tuple. default: [0,1].
            Optional distribution for uniform generation of values.

        :return: Created matrix filled with values.
        """

        if seed is not None:
            rnd.seed(seed)

        keys = sorted(list({(rnd.randint(0, shape[0] - 1), rnd.randint(0, shape[1] - 1))
                            for _ in range(int(shape[0] * shape[1] * density))}))

        I = [k[0] for k in keys]
        J = [k[1] for k in keys]
        count = len(keys)

        if dtype is INT:
            V = [rnd.randint(dist[0], dist[1]) for i in range(count)]
        elif dtype is UINT:
            V = [rnd.randint(dist[0], dist[1]) for i in range(count)]
        elif dtype is FLOAT:
            V = [rnd.uniform(dist[0], dist[1]) for i in range(count)]
        else:
            raise Exception("unknown type")

        return cls.from_lists(I, J, V, shape=shape, dtype=dtype)

    @classmethod
    def dense(cls, shape, dtype=INT, fill_value=0):
        """
        Creates new dense matrix of specified shape and fills with desired value.

        >>> M = Matrix.dense((3, 4), INT, 2)
        >>> print(M)
        '
            0 1 2 3
         0| 2 2 2 2|  0
         1| 2 2 2 2|  1
         2| 2 2 2 2|  2
            0 1 2 3
        '

        :param shape: 2-tuple.
            Size of the matrix.

        :param dtype: optional: Type. default: INT.
            Type of values matrix will have.

        :param fill_value: optional: any. default: 0.
            Optional value to fill with.

        :return: Matrix filled with value.
        """

        from .bridge import FormatMatrix

        M = Matrix(shape, dtype)
        M.set_format(FormatMatrix.CPU_LIL)

        for i in range(shape[0]):
            for j in range(shape[1]):
                M.set(i, j, fill_value)

        return M

    @classmethod
    def diag(cls, shape, dtype=INT, diag_value=1):
        """
        Diagonal matrix of desired shape and desired fill value on diagonal.

        >>> M = Matrix.diag((5, 5), INT, -1)
        >>> print(M)
        '
            0 1 2 3 4
         0|-1 . . . .|  0
         1| .-1 . . .|  1
         2| . .-1 . .|  2
         3| . . .-1 .|  3
         4| . . . .-1|  4
            0 1 2 3 4
        '

        :param shape: 2-tuple.
            Size of the matrix.

        :param dtype: optional: Type. default: INT.
            Type of values matrix will have.

        :param diag_value: optional: any. default: 1.
            Optional value to fill the diagonal with.

        :return: Matrix with main diagonal filled with value.
        """

        M = Matrix(shape, dtype)

        for i in range(min(shape[0], shape[1])):
            M.set(i, i, diag_value)

        return M

    def mxm(self, M, op_mult, op_add, out=None, init=None, desc=None):
        """
        General sparse-matrix by sparse-matrix product.

        Generate left operand matrix of shape 3x5 for product.
        >>> M = Matrix.from_lists([0, 1, 2, 2], [1, 2, 0, 4], [1, 2, 3, 4], (3, 5), INT)
        >>> print(M)
        '
            0 1 2 3 4
         0| . 1 . . .|  0
         1| . . 2 . .|  1
         2| 3 . . . 4|  2
            0 1 2 3 4
        '

        Generate right operand matrix of shape 5x4 for product, num of rows must match.
        >>> N = Matrix.from_lists([0, 1, 2, 3], [2, 0, 1, 3], [2, 3, 4, 5], (5, 4), INT)
        >>> print(N)
        '
            0 1 2 3
         0| . . 2 .|  0
         1| 3 . . .|  1
         2| . 4 . .|  2
         3| . . . 5|  3
         4| . . . .|  4
            0 1 2 3
        '

        Evaluate product using respective element-wise operations.
        >>> R = M.mxm(N, INT.MULT, INT.PLUS)
        >>> print(R)
        '
            0 1 2 3
         0| 3 . . .|  0
         1| . 8 . .|  1
         2| . . 6 .|  2
            0 1 2 3
        '

        :param M: Matrix.
            Matrix for a product.

        :param op_mult: OpBinary.
            Element-wise binary operator for matrix vector elements product.

        :param op_add: OpBinary.
            Element-wise binary operator for matrix vector products sum.

        :param out: optional: Matrix: default: None.
            Optional matrix to store result of product.

        :param init: optional: Scalar: default: 0.
            Optional neutral init value for reduction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Matrix with result.
        """

        if out is None:
            out = Matrix(shape=(self.n_rows, M.n_cols), dtype=self.dtype)
        if init is None:
            init = Scalar(dtype=self.dtype, value=0)

        assert M
        assert out
        assert init
        assert out.dtype == self.dtype
        assert M.dtype == self.dtype
        assert init.dtype == self.dtype
        assert out.n_rows == self.n_rows
        assert out.n_cols == M.n_cols
        assert self.n_cols == M.n_rows

        check(backend().spla_Exec_mxm(out.hnd, self.hnd, M.hnd,
                                      op_mult.hnd, op_add.hnd,
                                      init.hnd, self._get_desc(desc), self._get_task(None)))

        return out

    def mxmT(self, mask, M, op_mult, op_add, op_select, out=None, init=None, desc=None):
        """
        Masked sparse-matrix by sparse-matrix^T (transposed) product with sparse-mask.

        Generate left operand matrix of shape 3x5 for product.
        >>> M = Matrix.from_lists([0, 1, 2, 2], [1, 2, 0, 4], [1, 2, 3, 4], (3, 5), INT)
        >>> print(M)
        '
            0 1 2 3 4
         0| . 1 . . .|  0
         1| . . 2 . .|  1
         2| 3 . . . 4|  2
            0 1 2 3 4
        '

        Generate right operand matrix of shape 4x5 for product, since transposed only num of columns must match.
        >>> N = Matrix.from_lists([0, 1, 2, 3], [1, 2, 0, 3], [2, 3, 4, 5], (4, 5), INT)
        >>> print(N)
        '
            0 1 2 3 4
         0| . 2 . . .|  0
         1| . . 3 . .|  1
         2| 4 . . . .|  2
         3| . . . 5 .|  3
            0 1 2 3 4
        '

        Generate mask of interested us values of shape 3x4 where dim is num of rows from `M` and `N`.
        >>> mask = Matrix.dense((3, 4), INT, fill_value=1)
        >>> print(mask)
        '
            0 1 2 3
         0| 1 1 1 1|  0
         1| 1 1 1 1|  1
         2| 1 1 1 1|  2
            0 1 2 3
        '

        Evaluate product for all values using respective select operation.
        >>> R = M.mxmT(mask, N, INT.MULT, INT.PLUS, INT.GTZERO)
        >>> print(R)
        '
            0 1 2 3
         0| 2 . . .|  0
         1| . 6 . .|  1
         2| . .12 .|  2
            0 1 2 3
        '

        Evaluate the same product but disable by mask using falsified predicate.
        >>> R = M.mxmT(mask, N, INT.MULT, INT.PLUS, INT.EQZERO)
        >>> print(R)
        '
            0 1 2 3
         0| . . . .|  0
         1| . . . .|  1
         2| . . . .|  2
            0 1 2 3
        '

        :param mask: Matrix.
            Matrix to select for which values to compute product.

        :param M: Matrix.
            Matrix for a product.

        :param op_mult: OpBinary.
            Element-wise binary operator for matrix vector elements product.

        :param op_add: OpBinary.
            Element-wise binary operator for matrix vector products sum.

        :param op_select: OpSelect.
            Selection op to filter mask.

        :param out: optional: Matrix: default: None.
            Optional matrix to store result of product.

        :param init: optional: Scalar: default: 0.
            Optional neutral init value for reduction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Matrix with result.
        """

        if out is None:
            out = Matrix(shape=mask.shape, dtype=self.dtype)
        if init is None:
            init = Scalar(dtype=self.dtype, value=0)

        assert M
        assert out
        assert init
        assert mask
        assert out.dtype == self.dtype
        assert M.dtype == self.dtype
        assert mask.dtype == self.dtype
        assert init.dtype == self.dtype
        assert out.n_rows == self.n_rows
        assert out.n_cols == M.n_rows
        assert self.n_cols == M.n_cols
        assert mask.shape == out.shape

        check(backend().spla_Exec_mxmT_masked(out.hnd, mask.hnd, self.hnd, M.hnd,
                                              op_mult.hnd, op_add.hnd, op_select.hnd,
                                              init.hnd, self._get_desc(desc), self._get_task(None)))

        return out

    def kron(self, M, op_mult, out=None, desc=None):
        """
        Kronecker product of two sparse matrices.

        Generate two matrices, sparse with different values and dense with 1.
        >>> A = Matrix.from_lists([0, 1, 2], [1, 2, 0], [2, 3, 4], (3, 3), INT)
        >>> B = Matrix.dense((3, 3), INT, 1)

        Evaluate product with default `mutl` and show result.
        >>> print(A.kron(B, op_mult=INT.MULT))
        '
            0 1 2 3 4 5 6 7 8
         0| . . . 2 2 2 . . .|  0
         1| . . . 2 2 2 . . .|  1
         2| . . . 2 2 2 . . .|  2
         3| . . . . . . 3 3 3|  3
         4| . . . . . . 3 3 3|  4
         5| . . . . . . 3 3 3|  5
         6| 4 4 4 . . . . . .|  6
         7| 4 4 4 . . . . . .|  7
         8| 4 4 4 . . . . . .|  8
            0 1 2 3 4 5 6 7 8
        '

        The same matrices but order is changed gives a bit of different result.
        >>> print(B.kron(A, op_mult=INT.MULT))
        '
            0 1 2 3 4 5 6 7 8
         0| . 2 . . 2 . . 2 .|  0
         1| . . 3 . . 3 . . 3|  1
         2| 4 . . 4 . . 4 . .|  2
         3| . 2 . . 2 . . 2 .|  3
         4| . . 3 . . 3 . . 3|  4
         5| 4 . . 4 . . 4 . .|  5
         6| . 2 . . 2 . . 2 .|  6
         7| . . 3 . . 3 . . 3|  7
         8| 4 . . 4 . . 4 . .|  8
            0 1 2 3 4 5 6 7 8
        '

        Generate diagonal matrix and dense (not square).
        >>> A = Matrix.diag((3, 3), INT, 1)
        >>> B = Matrix.dense((2, 4), INT, 1)

        Eval product with `plus` operation instead.
        >>> print(A.kron(B, op_mult=INT.PLUS).to_string(width=3))
        '
              0  1  2  3  4  5  6  7  8  9 10 11
          0|  2  2  2  2  .  .  .  .  .  .  .  .|  0
          1|  2  2  2  2  .  .  .  .  .  .  .  .|  1
          2|  .  .  .  .  2  2  2  2  .  .  .  .|  2
          3|  .  .  .  .  2  2  2  2  .  .  .  .|  3
          4|  .  .  .  .  .  .  .  .  2  2  2  2|  4
          5|  .  .  .  .  .  .  .  .  2  2  2  2|  5
              0  1  2  3  4  5  6  7  8  9 10 11
        '

        :param M: Matrix.
            Right matrix for product.

        :param op_mult: OpBinary.
            Element-wise binary operator for matrix elements product.

        :param out: optional: Matrix: default: None.
            Optional matrix to store result of product.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Matrix with result.
        """

        if out is None:
            out = Matrix(shape=(self.n_rows * M.n_rows, self.n_cols * M.n_cols), dtype=self.dtype)

        assert M
        assert out
        assert M.dtype == self.dtype
        assert out.dtype == self.dtype
        assert out.n_rows == self.n_rows * M.n_rows
        assert out.n_cols == self.n_cols * M.n_cols

        check(backend().spla_Exec_kron(out.hnd, self.hnd, M.hnd, op_mult.hnd,
                                       self._get_desc(desc), self._get_task(None)))

        return out

    def kronpow(self, exponent, op_mult=None):
        """
        Kronecker's expansion, evaluate product for matrix itself giving nice pattern.
        Useful operation for synthetic graphs generation.

        Source 2x2 pattern matrix for expansion.
        >>> M = Matrix.from_lists([0, 0, 1], [0, 1, 1], [1, 2, 3], (2, 2), INT)

        Exponent with 0 is the identinty matrix of the source shape.
        >>> print(M.kronpow(0))
        '
            0 1
         0| 1 .|  0
         1| . 1|  1
            0 1
        '

        Exmponent with 1 is the source matrix.
        >>> print(M.kronpow(1))
        '
            0 1
         0| 1 2|  0
         1| . 3|  1
            0 1
        '

        Exmponent with 2 it is kron product of self by itself.
        >>> print(M.kronpow(2))
        '
            0 1 2 3
         0| 1 2 2 4|  0
         1| . 3 . 6|  1
         2| . . 3 6|  2
         3| . . . 9|  3
            0 1 2 3
        '

        Exmponent with 3 it is effectively prev result matrix kron by itself.
        >>> print(M.kronpow(3).to_string(width=3))
        '
              0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
          0|  1  2  2  4  2  4  4  8  2  4  4  8  4  8  8 16|  0
          1|  .  3  .  6  .  6  . 12  .  6  . 12  . 12  . 24|  1
          2|  .  .  3  6  .  .  6 12  .  .  6 12  .  . 12 24|  2
          3|  .  .  .  9  .  .  . 18  .  .  . 18  .  .  . 36|  3
          4|  .  .  .  .  3  6  6 12  .  .  .  .  6 12 12 24|  4
          5|  .  .  .  .  .  9  . 18  .  .  .  .  . 18  . 36|  5
          6|  .  .  .  .  .  .  9 18  .  .  .  .  .  . 18 36|  6
          7|  .  .  .  .  .  .  . 27  .  .  .  .  .  .  . 54|  7
          8|  .  .  .  .  .  .  .  .  3  6  6 12  6 12 12 24|  8
          9|  .  .  .  .  .  .  .  .  .  9  . 18  . 18  . 36|  9
         10|  .  .  .  .  .  .  .  .  .  .  9 18  .  . 18 36|  10
         11|  .  .  .  .  .  .  .  .  .  .  . 27  .  .  . 54|  11
         12|  .  .  .  .  .  .  .  .  .  .  .  .  9 18 18 36|  12
         13|  .  .  .  .  .  .  .  .  .  .  .  .  . 27  . 54|  13
         14|  .  .  .  .  .  .  .  .  .  .  .  .  .  . 27 54|  14
         15|  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 81|  15
              0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
        '

        :param exponent: int.
            Power to evaluate, must be >= 0.

        :param op_mult: optional: OpBinary. default: None.
            Optional operator for `kron`, by default used `mult`.

        :return: Kronecker power of a matrix.
        """

        assert exponent >= 0

        if op_mult is None:
            op_mult = self.dtype.MULT

        if exponent == 0:
            return Matrix.diag(shape=self.shape, dtype=self.dtype)
        if exponent == 1:
            return self

        result = self

        for _ in range(exponent - 1):
            result = result.kron(result, op_mult)

        return result

    def mxv(self, mask, v, op_mult, op_add, op_select, out=None, init=None, desc=None):
        """
        Masked sparse-matrix by a dense vector product with dense mask.

        >>> M = Matrix.from_lists([0, 1, 2, 2, 3], [1, 2, 0, 3, 2], [1, 2, 3, 4, 5], (4, 4), INT)
        >>> v = Vector.from_lists([2], [1], 4, INT)
        >>> mask = Vector.from_lists(list(range(4)), [1] * 4, 4, INT)
        >>> print(M.mxv(mask, v, INT.LAND, INT.LOR, INT.GTZERO))
        '
         0| .
         1| 1
         2| .
         3| 1
        '

        >>> M = Matrix.from_lists([0, 1, 2], [1, 2, 0], [1, 2, 3], (4, 4), INT)
        >>> v = Vector.from_lists([0, 1, 2], [2, 3, 4], 4, INT)
        >>> mask = Vector.from_lists(list(range(4)), [0] * 4, 4, INT)
        >>> print(M.mxv(mask, v, INT.MULT, INT.PLUS, INT.EQZERO))
        '
         0| 3
         1| 8
         2| 6
         3| .
        '

        :param mask: Vector.
            Vector to select for which values to compute product.

        :param v: Vector.
            Vector for a product.

        :param op_mult: OpBinary.
            Element-wise binary operator for matrix vector elements product.

        :param op_add: OpBinary.
            Element-wise binary operator for matrix vector products sum.

        :param op_select: OpSelect.
            Selection op to filter mask.

        :param out: optional: Vector: default: None.
            Optional vector to store result of product.

        :param init: optional: Scalar: default: 0.
            Optional neutral init value for reduction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Vector with result.
        """

        from .vector import Vector

        if out is None:
            out = Vector(shape=self.n_rows, dtype=self.dtype)
        if init is None:
            init = Scalar(dtype=self.dtype, value=0)

        assert v
        assert out
        assert init
        assert mask
        assert out.dtype == self.dtype
        assert v.dtype == self.dtype
        assert mask.dtype == self.dtype
        assert init.dtype == self.dtype
        assert out.n_rows == self.n_rows
        assert mask.n_rows == self.n_rows
        assert v.n_rows == self.n_cols

        check(backend().spla_Exec_mxv_masked(out.hnd, mask.hnd, self.hnd, v.hnd,
                                             op_mult.hnd, op_add.hnd, op_select.hnd,
                                             init.hnd, self._get_desc(desc), self._get_task(None)))

        return out

    def eadd(self, op_add, M, out=None, desc=None):
        """
        Element-wise addition with other matrix.

        Element-wise addition takes the set union of the patterns of A and B and applies a binary operator
        for all entries that appear in the set intersection of the patterns of A and B, preserving values
        without the pair unchanged in the result.

        >>> A = Matrix.from_lists([0, 1, 2], [1, 0, 2], [1, 2, 3], (3, 3), INT)
        >>> B = Matrix.from_lists([0, 1, 2], [1, 2, 2], [4, 5, 6], (3, 3), INT)
        >>> print(A.eadd(INT.PLUS, B))
        '
            0 1 2
         0| . 5 .|  0
         1| 2 . 5|  1
         2| . . 9|  2
            0 1 2
        '

        >>> M = Matrix.from_lists([0, 0, 1], [0, 1, 1], [1, 2, 3], (2, 2), INT)
        >>> print(M.eadd(INT.MULT, M.transpose()))
        '
            0 1
         0| 1 2|  0
         1| 2 9|  1
            0 1
        '

        :param op_add: OpBinary.
            Binary element-wise operation to apply.

        :param M: Matrix.
            Matrix for operation on the right side.

        :param out: optional: Matrix. default: None.
            Optional matrix to store result.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Matrix with a result.
        """

        if out is None:
            out = Matrix(shape=self.shape, dtype=self.dtype)

        assert M
        assert out
        assert op_add
        assert self.shape == M.shape
        assert self.shape == out.shape
        assert self.dtype == M.dtype
        assert self.dtype == out.dtype

        check(backend().spla_Exec_m_eadd(out.hnd, self.hnd, M.hnd, op_add.hnd,
                                         self._get_desc(desc), self._get_task(None)))

        return out

    def emult(self, op_mult, M, out=None, desc=None):
        """
        Element-wise multiplication with other matrix.

        Element-wise multiplication takes the set intersection of the patterns of A and B and applies a binary operator
        for all entries that appear in the set intersection of the patterns of A and B, removing the values without
        the pair from the result.

        >>> A = Matrix.from_lists([0, 1, 2], [1, 0, 2], [1, 2, 3], (3, 3), INT)
        >>> B = Matrix.from_lists([0, 1, 2], [1, 2, 2], [4, 5, 6], (3, 3), INT)
        >>> print(A.emult(INT.PLUS, B))
        '
            0 1 2
         0| . 5 .|  0
         1| . . .|  1
         2| . . 9|  2
            0 1 2
        '

        >>> M = Matrix.from_lists([0, 0, 1], [0, 1, 1], [1, 2, 3], (2, 2), INT)
        >>> print(M.emult(INT.MULT, M.transpose()))
        '
            0 1
         0| 1 .|  0
         1| . 9|  1
            0 1
        '

        :param op_mult: OpBinary.
            Binary element-wise operation to apply.

        :param M: Matrix.
            Matrix for operation on the right side.

        :param out: optional: Matrix. default: None.
            Optional matrix to store result.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Matrix with a result.
        """

        if out is None:
            out = Matrix(shape=self.shape, dtype=self.dtype)

        assert M
        assert out
        assert op_mult
        assert self.shape == M.shape
        assert self.shape == out.shape
        assert self.dtype == M.dtype
        assert self.dtype == out.dtype

        check(backend().spla_Exec_m_emult(out.hnd, self.hnd, M.hnd, op_mult.hnd,
                                          self._get_desc(desc), self._get_task(None)))

        return out

    def reduce_by_row(self, op_reduce, out=None, init=None, desc=None):
        """
        Reduce matrix elements by a row to a column vector.

        >>> M = Matrix.from_lists([0, 2, 2, 3], [0, 1, 3, 2], [1, 2, 3, 4], (4, 4), INT)
        >>> print(M.reduce_by_row(INT.PLUS))
        '
         0| 1
         1| .
         2| 5
         3| 4
        '

        :param op_reduce: OpBinary.
            Binary operation to apply for reduction of matrix elements.

        :param out: optional: Vector: default: None.
            Optional vector to store result of reduction.

        :param init: optional: Scalar: default: 0.
            Optional neutral init value for reduction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Vector with result.
        """

        from .vector import Vector

        if out is None:
            out = Vector(shape=self.n_rows, dtype=self.dtype)
        if init is None:
            init = Scalar(dtype=self.dtype, value=0)

        assert out
        assert init
        assert out.n_rows == self.n_rows
        assert out.dtype == self.dtype
        assert init.dtype == self.dtype

        check(backend().spla_Exec_m_reduce_by_row(out.hnd, self.hnd, op_reduce.hnd, init.hnd,
                                                  self._get_desc(desc), self._get_task(None)))

        return out

    def reduce_by_column(self, op_reduce, out=None, init=None, desc=None):
        """
        Reduce matrix elements by a column to a row vector.

        >>> M = Matrix.from_lists([0, 1, 2, 3], [0, 3, 3, 2], [1, 2, 3, 4], (4, 4), INT)
        >>> print(M.reduce_by_column(INT.PLUS))
        '
         0| 1
         1| .
         2| 4
         3| 5
        '

        :param op_reduce: OpBinary.
            Binary operation to apply for reduction of matrix elements.

        :param out: optional: Vector: default: None.
            Optional vector to store result of reduction.

        :param init: optional: Scalar: default: 0.
            Optional neutral init value for reduction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Vector with result.
        """

        from .vector import Vector

        if out is None:
            out = Vector(shape=self.n_cols, dtype=self.dtype)
        if init is None:
            init = Scalar(dtype=self.dtype, value=0)

        assert out
        assert init
        assert out.n_rows == self.n_cols
        assert out.dtype == self.dtype
        assert init.dtype == self.dtype

        check(backend().spla_Exec_m_reduce_by_column(out.hnd, self.hnd, op_reduce.hnd, init.hnd,
                                                     self._get_desc(desc), self._get_task(None)))

        return out

    def reduce(self, op_reduce, out=None, init=None, desc=None):
        """
        Reduce matrix elements.

        >>> M = Matrix.from_lists([1, 2, 3, 3], [0, 1, 0, 3], [-1, -4, 4, 2], (4, 4), INT)
        >>> print(M.reduce(op_reduce=INT.MULT, init=Scalar(INT, 1)))
        '
            32
        '

        :param op_reduce: OpBinary.
            Binary operation to apply for reduction of matrix elements.

        :param out: optional: Scalar: default: 0.
            Optional scalar to store result of reduction.

        :param init: optional: Scalar: default: 0.
            Optional neutral init value for reduction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Scalar value with result.
        """

        if out is None:
            out = Scalar(dtype=self.dtype)
        if init is None:
            init = Scalar(dtype=self.dtype, value=0)

        assert out.dtype == self.dtype
        assert init.dtype == self.dtype

        check(backend().spla_Exec_m_reduce(out.hnd, init.hnd, self.hnd, op_reduce.hnd,
                                           self._get_desc(desc), self._get_task(None)))

        return out

    def transpose(self, out=None, op_apply=None, desc=None):
        """
        Transpose matrix.

        Generate 3x4 matrix with int source data.
        >>> M = Matrix.from_lists([0, 1, 2], [3, 2, 0], [-5, 3, 9], (3, 4), INT)
        >>> print(M)
        '
            0 1 2 3
         0| . . .-5|  0
         1| . . 3 .|  1
         2| 9 . . .|  2
            0 1 2 3
        '

        Transpose matrix `M` as usual and print result.
        >>> print(M.transpose())
        '
            0 1 2
         0| . . 9|  0
         1| . . .|  1
         2| . 3 .|  2
         3|-5 . .|  3
            0 1 2
        '

        Transpose by map each value to `1`, discarding prev value.
        >>> print(M.transpose(op_apply=INT.UONE))
        '
            0 1 2
         0| . . 1|  0
         1| . . .|  1
         2| . 1 .|  2
         3| 1 . .|  3
            0 1 2
        '

        Transpose and apply additive-inverse for each value effectively changing the sign of values.
        >>> print(M.transpose(op_apply=INT.AINV))
        '
            0 1 2
         0| . .-9|  0
         1| . . .|  1
         2| .-3 .|  2
         3| 5 . .|  3
            0 1 2
        '

        :param out: optional: Matrix: default: none.
            Optional matrix to store result.

        :param op_apply: optional: OpUnary. default: None.
            Optional unary function to apply on transposition.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Transposed matrix.
        """

        if out is None:
            out = Matrix(shape=(self.n_cols, self.n_rows), dtype=self.dtype)
        if op_apply is None:
            op_apply = self.dtype.IDENTITY

        assert out
        assert op_apply
        assert out.n_rows == self.n_cols
        assert out.n_cols == self.n_rows
        assert out.dtype == self.dtype

        check(backend().spla_Exec_m_transpose(out.hnd, self.hnd, op_apply.hnd,
                                              self._get_desc(desc), self._get_task(None)))

        return out

    def extract_row(self, index, out=None, op_apply=None, desc=None):
        """
        Extract matrix row.

        >>> M = Matrix.from_lists([0, 0, 1, 2], [1, 2, 3, 0], [-1, 1, 2, 3], (3, 4), INT)
        >>> print(M)
        '
            0 1 2 3
         0| .-1 1 .|  0
         1| . . . 2|  1
         2| 3 . . .|  2
            0 1 2 3
        '

        >>> print(M.extract_row(0))
        '
         0| .
         1|-1
         2| 1
         3| .
        '

        >>> print(M.extract_row(0, op_apply=INT.AINV))
        '
         0| .
         1| 1
         2|-1
         3| .
        '

        :param index: int.
            Index of row to extract.

        :param out: optional: Vector: default: none.
            Optional vector to store result.

        :param op_apply: optional: OpUnary. default: None.
            Optional unary function to apply on extraction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Vector.
        """

        from .vector import Vector

        if out is None:
            out = Vector(shape=self.n_cols, dtype=self.dtype)
        if op_apply is None:
            op_apply = self.dtype.IDENTITY

        assert out
        assert op_apply
        assert out.dtype == self.dtype
        assert out.n_rows == self.n_cols
        assert 0 <= index < self.n_rows

        check(backend().spla_Exec_m_extract_row(out.hnd, self.hnd, ctypes.c_uint(index), op_apply.hnd,
                                                self._get_desc(desc), self._get_task(None)))

        return out

    def extract_column(self, index, out=None, op_apply=None, desc=None):
        """
        Extract matrix column.

        >>> M = Matrix.from_lists([0, 1, 1, 2], [1, 0, 3, 1], [-1, 1, 2, 3], (3, 4), INT)
        >>> print(M)
        '
            0 1 2 3
         0| .-1 . .|  0
         1| 1 . . 2|  1
         2| . 3 . .|  2
            0 1 2 3
        '

        >>> print(M.extract_column(1))
        '
         0|-1
         1| .
         2| 3
        '

        >>> print(M.extract_column(1, op_apply=INT.AINV))
        '
         0| 1
         1| .
         2|-3
        '

        :param index: int.
            Index of column to extract.

        :param out: optional: Vector: default: none.
            Optional vector to store result.

        :param op_apply: optional: OpUnary. default: None.
            Optional unary function to apply on extraction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Vector.
        """

        from .vector import Vector

        if out is None:
            out = Vector(shape=self.n_rows, dtype=self.dtype)
        if op_apply is None:
            op_apply = self.dtype.IDENTITY

        assert out
        assert op_apply
        assert out.dtype == self.dtype
        assert out.n_rows == self.n_rows
        assert 0 <= index < self.n_cols

        check(backend().spla_Exec_m_extract_column(out.hnd, self.hnd, ctypes.c_uint(index), op_apply.hnd,
                                                   self._get_desc(desc), self._get_task(None)))

        return out

    def __str__(self):
        return self.to_string()

    def __iter__(self):
        I, J, V = self.to_lists()
        return zip(I, J, V)

    def _get_desc(self, desc: Descriptor):
        return desc.hnd if desc else ctypes.c_void_p(0)

    def _get_task(self, task):
        return ctypes.POINTER(ctypes.c_void_p)()
