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
        """
        return self._dtype

    @property
    def n_rows(self):
        """
        Number of rows in the matrix.
        """
        return self._shape[0]

    @property
    def n_cols(self):
        """
        Number of cols in the matrix.
        """
        return self._shape[1]

    @property
    def shape(self):
        """
        2-Tuple with shape of matrix.
        """

        return self._shape

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

    def to_lists(self):
        """
        Read matrix data as a python lists of I, J and V.

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

        :return: List of matrix entries as (I, J, V).
        """

        I, J, V = self.to_lists()
        return list(zip(I, J, V))

    @classmethod
    def from_lists(cls, I: list, J: list, V: list, shape, dtype=INT):
        """
        Build matrix from a list of sorted keys and associated values to store in matrix.
        List with keys `I` and `J` must index entries from range [0, shape-1] and all keys must be sorted.

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
    def generate(cls, shape, dtype=INT, density=0.1, seed=None, dist=(0, 1)):
        """
        Creates new matrix of desired type and shape and fills its content
        with random values, generated using specified distribution.

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

    def reduce(self, op_reduce, out=None, init=None, desc=None):
        """
        Reduce matrix elements.

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

    def __iter__(self):
        I, J, V = self.to_lists()
        return zip(I, J, V)

    def _get_desc(self, desc: Descriptor):
        return desc.hnd if desc else ctypes.c_void_p(0)

    def _get_task(self, task):
        return ctypes.POINTER(ctypes.c_void_p)()
