"""
Wrapped native (spla C API) vector primitive implementation.
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
from .type import Type, INT, UINT
from .object import Object
from .memview import MemView
from .scalar import Scalar
from .matrix import Matrix
from .descriptor import Descriptor
from .op import OpUnary, OpBinary, OpSelect


class Vector(Object):
    """
    Generalized statically-typed sparse storage-invariant vector primitive.

    Notes
    -----

    Vector typical usage:

    - Instantiate vector primitive
    - Build incrementally from yours data source
    - Vector usage in a sequence of math operations
    - Read-back vector data to python to analyse results

    Steps (2) and (4) requires internal format transformations and possible transfer of data
    from acc (GPU) side if acceleration was employed in computations. These steps may be very
    intensive, so you have to avoid them in critical parts of computations. If you need faster
    data reads, prefer usage of batched reads, where all content of storage read at once.

    Details
    -------

    Vector class support all spla C API vector functions.
    It provides bind functionality as well as new functions/methods for better python user experience.

    Vector internally manages optimal format of stored data. Use hints to
    force vector state and format changes.

    Vector optional uses dedicated/integrated GPU to speedup computations
    using one of built-in OpenCL or CUDA accelerators.
    """

    def __init__(self, shape, dtype=INT, hnd=None, label=None):
        """
        Creates new vector of specified type and shape.

        :param dtype: Type.
            Type parametrization of a storage.

        :param shape: int.
            Vector size.

        :param hnd: optional: ctypes.c_void_p. default: None.
            Optional native handle to retain.

        :param label: optional: str. default: None.
            Optional label to assign.
        """

        super().__init__(None, None)

        assert dtype
        assert shape
        assert shape > 0
        assert issubclass(dtype, Type)

        self._dtype = dtype

        if not hnd:
            hnd = ctypes.c_void_p(0)
            check(backend().spla_Vector_make(ctypes.byref(hnd), ctypes.c_uint(shape), dtype._hnd))

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
        Number of rows in the vector.
        """
        count = ctypes.c_uint(0)
        check(backend().spla_Vector_get_n_rows(self._hnd, ctypes.byref(count)))
        return int(count.value)

    @property
    def shape(self):
        """
        2-Tuple with shape of vector where second value is always 1.
        """

        return self.n_rows, 1

    def build(self, ii_view: MemView, vv_view: MemView):
        """
        Builds vector content from a raw memory view resources.

        :param ii_view: MemView.
            View to keys of vector to assign.

        :param vv_view: MemView.
            View to actual values to store.
        """

        assert ii_view
        assert vv_view

        check(backend().spla_Vector_build(self._hnd, ii_view._hnd, vv_view._hnd))

    def read(self):
        """
        Read the content of the vector as a MemView of keys and values.

        :return: tuple (MemView, MemView) objects with view to the vector keys and vector values.
        """

        ii_view_hnd = ctypes.c_void_p(0)
        vv_view_hnd = ctypes.c_void_p(0)
        check(backend().spla_Vector_read(self._hnd, ctypes.byref(ii_view_hnd), ctypes.byref(vv_view_hnd)))
        return MemView(hnd=ii_view_hnd, owner=self), MemView(hnd=vv_view_hnd, owner=self)

    def to_lists(self):
        """
        Read vector data as a python lists of keys and values.

        :return: Tuple (List, List) with the vector keys and vector values.
        """

        keys, values = self.read()
        count = int(keys.size / ctypes.sizeof(UINT._c_type))

        buffer_keys = (UINT._c_type * count)()
        buffer_values = (self._dtype._c_type * count)()

        check(backend().spla_MemView_read(keys._hnd, ctypes.c_size_t(0), ctypes.sizeof(buffer_keys), buffer_keys))
        check(backend().spla_MemView_read(values._hnd, ctypes.c_size_t(0), ctypes.sizeof(buffer_values), buffer_values))

        return list(buffer_keys), list(buffer_values)

    @classmethod
    def from_lists(cls, ii: list, vv: list, shape, dtype=INT):
        """
        Build vector from a list of sorted keys and associated values to store in vector.
        List with keys `ii` must index entries from range [0, shape-1] and all keys must be sorted.

        :param ii: list[UINT].
             List with integral keys of entries.

        :param vv: list[Type].
             List with values to store in the vector.

        :param shape: int.
             Vector size.

        :param dtype:
             Type of storage parametrization for vector.

        :return: Created vector filled with values.
        """

        assert ii
        assert vv
        assert len(ii) == len(vv)
        assert shape > 0

        count = len(ii)

        c_ii = (UINT._c_type * count)(*ii)
        c_vv = (dtype._c_type * count)(*vv)

        view_ii = MemView(buffer=c_ii, size=ctypes.sizeof(c_ii), mutable=False)
        view_vv = MemView(buffer=c_vv, size=ctypes.sizeof(c_vv), mutable=False)

        v = Vector(shape=shape, dtype=dtype)
        v.build(view_ii, view_vv)

        return v

    def reduce(self, op_reduce, r=None, init=None, desc=None):
        """
        Reduce vector elements.

        :param op_reduce: OpBinary.
            Binary operation to apply for reduction of vector elements.

        :param r: optional: Scalar: default: 0.
            Optional scalar to store result of reduction.

        :param init: optional: Scalar: default: 0.
            Optional neutral init value for reduction.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Scalar value with result.
        """

        if r is None:
            r = Scalar(dtype=self.dtype)
        if init is None:
            init = Scalar(dtype=self.dtype, value=0)

        check(backend().spla_Exec_v_reduce(r.hnd, init.hnd, self.hnd, op_reduce.hnd,
                                           self._get_desc(desc), self._get_task(None)))

        return r

    def __iter__(self):
        keys, values = self.to_lists()
        return zip(keys, values)

    def _get_desc(self, desc: Descriptor):
        return desc.hnd if desc else ctypes.c_void_p(0)

    def _get_task(self, task):
        return ctypes.POINTER(ctypes.c_void_p)()
