"""
Wrapped native (spla C API) array primitive implementation.
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
import random as rnd


class Array(Object):
    """
    Generalized statically-typed dense linear array primitive.

    Attributes
    ----------

    - dtype  : `Type` type of stored array elements
    - n_vals : `int` number of values in the array
    - shape  : `2-tuple` shape of the array in form of two integers tuple (second dim is 1)

    Notes
    -----

    Array provides features for:

    - fill with values
    - read-back by value
    - source for vector construction
    - source for matrix construction

    Array typical usage:

    - (1) Instantiate array primitive
    - (2) Fill with values from your data
    - (3) Use array to build entire vector or matrix
    - (4) Read-back results of computations from vector or matrix

    Details
    -------

    Array class support all spla C API vector functions.
    It provides bind functionality as well as new functions/methods for better python user experience.

    Array internally manages native optimized storage for values.
    Reading and writing values into array is fast and has nearly no overhead.
    """

    __slots__ = ["_dtype"]

    def __init__(self, dtype=INT, shape=None, hnd=None, label=None):
        """
        Creates new array of specified type and shape.

        Parameters
        ----------

        dtype: Type.
            Type parametrization of a storage.

        shape: optional: int. default: None.
            Size of the array.

        hnd: optional: ctypes.c_void_p. default: None.
            Optional native handle to retain.

        label: optional: str. default: None.
            Optional label to assign.
        """

        super().__init__(None, None)

        assert dtype
        assert issubclass(dtype, Type)

        self._dtype = dtype

        if not shape:
            shape = 0

        if not hnd:
            hnd = ctypes.c_void_p(0)
            check(backend().spla_Array_make(ctypes.byref(hnd), ctypes.c_uint(shape), dtype._hnd))

        super().__init__(label, hnd)

    @property
    def dtype(self):
        """
        Type used for storage parametrization of this container.
        """
        return self._dtype

    @property
    def n_vals(self):
        """
        Number of values in the array.
        """

        n_values = ctypes.c_uint(0)
        check(backend().spla_Array_get_n_values(self._hnd, ctypes.byref(n_values)))
        return int(n_values.value)

    @property
    def shape(self):
        """
        2-Tuple with shape of array where second value is always 1.
        """

        return self.n_vals, 1

    @property
    def empty(self):
        """
        Checks if array is empty (has 0-size) and returns true. True if array is empty.
        """

        return self.shape[0] == 0

    def set(self, index, value):
        """
        Set value at specified index.

        Parameters
        ----------

        index: int.
            Index at which value to set.

        value: any.
            Value to set, must be convertible to destination type.
        """

        check(self._dtype._array_set(self._hnd, ctypes.c_uint(index), self._dtype._c_type(value)))

    def get(self, index):
        """
        Get value at specified index.

        Parameters
        ----------

        index: int.
            Index at which to get value.

        Returns
        -------

        Value at specified index.
        """

        value = self._dtype._c_type(0)
        check(self._dtype._array_get(self._hnd, ctypes.c_uint(index), ctypes.byref(value)))
        return self._dtype.cast_value(value)

    def build(self, view: MemView):
        """
        Builds array from a raw memory view resource.

        Parameters
        ----------

        view: MemView.
            View to a memory to build the array content from.
        """

        check(backend().spla_Array_build(self._hnd, view._hnd))

    def read(self):
        """
        Read the content of the array as a MemView.

        Returns
        -------

        MemView object with view to the array content.
        """

        view_hnd = ctypes.c_void_p(0)
        check(backend().spla_Array_read(self._hnd, ctypes.byref(view_hnd)))
        return MemView(hnd=view_hnd, owner=self)

    def resize(self, shape=0):
        """
        Resizes array to new size with desired num of values specified as shape.

        Parameters
        ----------

        shape: optional: int. default: 0.
            New array capacity.
        """

        check(backend().spla_Array_resize(self._hnd, ctypes.c_uint(shape)))

    def clear(self):
        """
        Clears array removing all elements, so it has 0 values.
        """

        check(backend().spla_Array_clear(self._hnd))

    def to_list(self):
        """
        Read array data as a python list of values.

        Returns
        -------

        List with values stored in the array.
        """

        buffer = (self.dtype._c_type * self.n_vals)()
        size = ctypes.c_size_t(ctypes.sizeof(buffer))
        view = self.read()
        check(backend().spla_MemView_read(view._hnd, ctypes.c_size_t(0), size, buffer))
        return list(buffer)

    @classmethod
    def from_list(cls, values, dtype=INT):
        """
        Creates new array of desired type from list of `values`.

        Parameters
        ----------

        values: List.
            List with values to fill array.

        dtype: Type.
            Type of the array stored value.

        Returns
        -------

        Created array filled with values.
        """

        buffer = (dtype._c_type * len(values))(*values)
        view = MemView(buffer=buffer, size=ctypes.sizeof(buffer), mutable=False)
        array = Array(dtype=dtype)
        array.build(view)
        return array

    @classmethod
    def generate(cls, dtype=INT, shape=0, seed=None, dist=(0, 1)):
        """
        Creates new array of desired type and shape and fills its content
        with random values, generated using specified distribution.

        Parameters
        ----------

        dtype: Type.
            Type of values array will have.

        shape: optional: int. default: 0.
            Size of the array (number of values).

        seed: optional: int. default: None.
            Optional seed to randomize generator.

        dist: optional: tuple. default: [0,1].
            Optional distribution for uniform generation of values.

        Returns
        -------

        Created array filled with values.
        """

        array = Array(dtype=dtype, shape=shape)

        if seed is not None:
            rnd.seed(seed)

        if dtype is INT:
            for i in range(shape):
                array.set(i, rnd.randint(dist[0], dist[1]))
        elif dtype is UINT:
            for i in range(shape):
                array.set(i, rnd.randint(dist[0], dist[1]))
        elif dtype is FLOAT:
            for i in range(shape):
                array.set(i, rnd.uniform(dist[0], dist[1]))

        return array

    def __str__(self):
        return str(self.to_list())

    def __iter__(self):
        return iter(self.to_list())

    def __setitem__(self, key, value):
        assert isinstance(key, int)
        self.set(key, value)

    def __getitem__(self, item):
        assert isinstance(item, int)
        return self.get(item)
