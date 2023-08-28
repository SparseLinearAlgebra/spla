"""
Wrapped native (spla C API) type support implementation.
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

from .bridge import backend

__all__ = [
    'Type',
    'INT',
    'UINT',
    'FLOAT',
    'BUILT_IN'
]


class Type:
    """Spla base Type for storage parametrization."""

    _c_type = None
    _c_type_p = None
    _code = ''
    _scalar_get = None
    _scalar_set = None
    _array_get = None
    _array_set = None
    _vector_get = None
    _vector_set = None
    _matrix_get = None
    _matrix_set = None
    _hnd = None

    @classmethod
    def get_code(cls):
        return cls._code

    @classmethod
    def to_py(cls, value):
        pass


class INT(Type):
    """Spla integral INT-32 type."""

    _c_type = ctypes.c_int
    _c_type_p = ctypes.POINTER(ctypes.c_int)
    _code = 'I'

    @classmethod
    def _setup(cls):
        cls._scalar_get = backend().spla_Scalar_get_int
        cls._scalar_set = backend().spla_Scalar_set_int
        cls._array_get = backend().spla_Array_get_int
        cls._array_set = backend().spla_Array_set_int
        cls._vector_get = backend().spla_Vector_get_int
        cls._vector_set = backend().spla_Vector_set_int
        cls._matrix_get = backend().spla_Matrix_get_int
        cls._matrix_set = backend().spla_Matrix_set_int
        cls._hnd = backend().spla_Type_int()

    @classmethod
    def to_py(cls, value):
        return int(value.value)


class UINT(Type):
    """Spla integral UINT-32 type."""

    _c_type = ctypes.c_uint
    _c_type_p = ctypes.POINTER(ctypes.c_uint)
    _code = 'U'

    @classmethod
    def _setup(cls):
        cls._scalar_get = backend().spla_Scalar_get_uint
        cls._scalar_set = backend().spla_Scalar_set_uint
        cls._array_get = backend().spla_Array_get_uint
        cls._array_set = backend().spla_Array_set_uint
        cls._vector_get = backend().spla_Vector_get_uint
        cls._vector_set = backend().spla_Vector_set_uint
        cls._matrix_get = backend().spla_Matrix_get_uint
        cls._matrix_set = backend().spla_Matrix_set_uint
        cls._hnd = backend().spla_Type_uint()

    @classmethod
    def to_py(cls, value):
        return int(value.value)


class FLOAT(Type):
    """Spla floating-point FLOAT-32 type."""

    _c_type = ctypes.c_float
    _c_type_p = ctypes.POINTER(ctypes.c_float)
    _code = 'F'

    @classmethod
    def _setup(cls):
        cls._scalar_get = backend().spla_Scalar_get_float
        cls._scalar_set = backend().spla_Scalar_set_float
        cls._array_get = backend().spla_Array_get_float
        cls._array_set = backend().spla_Array_set_float
        cls._vector_get = backend().spla_Vector_get_float
        cls._vector_set = backend().spla_Vector_set_float
        cls._matrix_get = backend().spla_Matrix_get_float
        cls._matrix_set = backend().spla_Matrix_set_float
        cls._hnd = backend().spla_Type_float()

    @classmethod
    def to_py(cls, value):
        return float(value.value)


BUILT_IN = [INT, UINT, FLOAT]
