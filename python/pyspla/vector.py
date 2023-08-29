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
from .type import Type, INT, UINT, FLOAT
from .object import Object
from .memview import MemView
from .scalar import Scalar
from .matrix import Matrix
from .descriptor import Descriptor
from .op import OpUnary, OpBinary, OpSelect
import random as rnd


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

    __slots__ = ["_dtype", "_shape"]

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
        self._shape = (shape, 1)

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
        return self._shape[0]

    @property
    def shape(self):
        """
        2-Tuple with shape of vector where second value is always 1.
        """

        return self._shape

    def build(self, keys_view: MemView, values_view: MemView):
        """
        Builds vector content from a raw memory view resources.

        :param keys_view: MemView.
            View to keys of vector to assign.

        :param values_view: MemView.
            View to actual values to store.
        """

        assert keys_view
        assert values_view

        check(backend().spla_Vector_build(self._hnd, keys_view._hnd, values_view._hnd))

    def read(self):
        """
        Read the content of the vector as a MemView of keys and values.

        :return: tuple (MemView, MemView) objects with view to the vector keys and vector values.
        """

        keys_view_hnd = ctypes.c_void_p(0)
        values_view_hnd = ctypes.c_void_p(0)
        check(backend().spla_Vector_read(self._hnd, ctypes.byref(keys_view_hnd), ctypes.byref(values_view_hnd)))
        return MemView(hnd=keys_view_hnd, owner=self), MemView(hnd=values_view_hnd, owner=self)

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

    def to_list(self):
        """
        Read vector data as a python lists of tuples where key and value stored together.

        :return: List of vector entries.
        """

        keys, values = self.to_lists()
        return list(zip(keys, values))

    @classmethod
    def from_lists(cls, keys: list, values: list, shape, dtype=INT):
        """
        Build vector from a list of sorted keys and associated values to store in vector.
        List with keys `keys` must index entries from range [0, shape-1] and all keys must be sorted.

        :param keys: list[UINT].
             List with integral keys of entries.

        :param values: list[Type].
             List with values to store in the vector.

        :param shape: int.
             Vector size.

        :param dtype:
             Type of storage parametrization for vector.

        :return: Created vector filled with values.
        """

        assert len(keys) == len(values)
        assert shape > 0

        if not keys:
            return Vector(shape, dtype)

        count = len(keys)

        c_keys = (UINT._c_type * count)(*keys)
        c_values = (dtype._c_type * count)(*values)

        view_keys = MemView(buffer=c_keys, size=ctypes.sizeof(c_keys), mutable=False)
        view_values = MemView(buffer=c_values, size=ctypes.sizeof(c_values), mutable=False)

        v = Vector(shape=shape, dtype=dtype)
        v.build(view_keys, view_values)

        return v

    @classmethod
    def generate(cls, shape, dtype=INT, density=0.1, seed=None, dist=(0, 1)):
        """
        Creates new vector of desired type and shape and fills its content
        with random values, generated using specified distribution.

        :param shape: int.
            Size of the array (number of values).

        :param dtype: optional: Type. default: INT.
            Type of values vector will have.

        :param density: optional: float. default: 0.1.
            Density of vector or how many entries to generate.

        :param seed: optional: int. default: None.
            Optional seed to randomize generator.

        :param dist: optional: tuple. default: [0,1].
            Optional distribution for uniform generation of values.

        :return: Created array filled with values.
        """

        if seed is not None:
            rnd.seed(seed)

        keys = [i for i in range(0, shape) if rnd.uniform(0, 1) < density]
        count = len(keys)

        if dtype is INT:
            values = [rnd.randint(dist[0], dist[1]) for i in range(count)]
        elif dtype is UINT:
            values = [rnd.randint(dist[0], dist[1]) for i in range(count)]
        elif dtype is FLOAT:
            values = [rnd.uniform(dist[0], dist[1]) for i in range(count)]
        else:
            raise Exception("unknown type")

        return cls.from_lists(keys, values, shape=shape, dtype=dtype)

    def eadd(self, op_add, v, out=None, desc=None):
        """
        Element-wise add one vector to another and return result.

        :param op_add: OpBinary.
            Binary operation to sum values.

        :param v: Vector.
            Other right vector to sum with this.

        :param out: optional: Vector. default: None.
            Optional vector to store result.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: Vector with result.
        """

        if out is None:
            out = Vector(shape=self.n_rows, dtype=self.dtype)

        assert v
        assert v.n_rows == self.n_rows
        assert out.n_rows == self.n_rows
        assert v.dtype == self.dtype
        assert out.dtype == out.dtype
        assert op_add

        check(backend().spla_Exec_v_eadd(out.hnd, self.hnd, v.hnd, op_add.hnd,
                                         self._get_desc(desc), self._get_task(None)))

        return out

    def assign(self, mask, value, op_assign, op_select, desc=None):
        """
        Assign scalar value to a vector by mask.

        :param mask: Vector.
            Mask vector which structure will be used to select entries for assignment.

        :param value: Scalar.
            Value to assign.

        :param op_assign: OpBinary.
            Binary operation used to combine existing value in vector and scalar.

        :param op_select: OpSelect.
            Predicate to select entries in the mask to assign.

        :param desc: optional: Descriptor. default: None.
            Optional descriptor object to configure the execution.

        :return: This vector.
        """

        assert mask
        assert value
        assert mask.n_rows == self.n_rows
        assert op_assign
        assert op_select

        check(backend().spla_Exec_v_assign_masked(self.hnd, mask.hnd, value.hnd, op_assign.hnd, op_select.hnd,
                                                  self._get_desc(desc), self._get_task(None)))

        return self

    def reduce(self, op_reduce, out=None, init=None, desc=None):
        """
        Reduce vector elements.

        :param op_reduce: OpBinary.
            Binary operation to apply for reduction of vector elements.

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

        check(backend().spla_Exec_v_reduce(out.hnd, init.hnd, self.hnd, op_reduce.hnd,
                                           self._get_desc(desc), self._get_task(None)))

        return out

    def __iter__(self):
        keys, values = self.to_lists()
        return zip(keys, values)

    def _get_desc(self, desc: Descriptor):
        return desc.hnd if desc else ctypes.c_void_p(0)

    def _get_task(self, task):
        return ctypes.POINTER(ctypes.c_void_p)()
