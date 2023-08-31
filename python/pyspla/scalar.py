"""
Wrapped native (spla C API) scalar primitive implementation.
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
from .type import INT, FLOAT
from .object import Object


class Scalar(Object):
    """
    Generalized statically-typed scalar primitive.

    Notes
    -----

    Scalar typical usage:

    - Create scalar from python value
    - Pass scalar as argument to matrix/vector operation
    - Get scalar as a return value of some operation
    - Unpack value on python side

    Avoid intensive creation of scalar values. Prefer python native types.
    Use scalars only if you want to parametrise matrix/vector operations.

    Details
    -------

    Scalar class support all spla C API scalar functions.
    It provides bind functionality as well as new functions/methods for better python user experience.

    Scalar internally stored in the native format of the type it has.
    Meta-data additionally stored with each scalar value.
    """

    __slots__ = ["_dtype"]

    def __init__(self, dtype=INT, value=None, hnd=None, label=None):
        """
        Creates new scalar of desired type or retains existing C object.

        >>> s = Scalar(INT, 10)
        >>> print(s)
        '
            10
        '

        :param dtype: optional: Type. default: INT.
            Type of the scalar value.

        :param value: optional: Any. default: None.
            Optional value to store in scalar on creation.

        :param hnd: optional: ctypes.c_void_p. default: None.
            Optional handle to C object to retain in this scalar instance.

        :param label: optional: str. default: None.
            Optional debug label of the scalar.
        """

        super().__init__(None, None)

        self._dtype = dtype

        if hnd is None:
            hnd = ctypes.c_void_p(0)
            check(backend().spla_Scalar_make(ctypes.byref(hnd), dtype._hnd))

        super().__init__(label, hnd)
        self.set(value)

    @property
    def dtype(self):
        """
        Returns the type of stored value in the scalar.

        >>> s = Scalar(INT)
        >>> print(s.dtype)
        '
            <class 'pyspla.type.INT'>
        '
        """

        return self._dtype

    @property
    def shape(self):
        """
        2-tuple shape of the storage. For scalar object it is always 1 by 1.

        >>> s = Scalar(INT)
        >>> print(s.shape)
        '
            (1, 1)
        '
        """

        return 1, 1

    @property
    def n_vals(self):
        """
        Number of stored values in the scalar. Always 1.

        >>> s = Scalar(INT)
        >>> print(s.n_vals)
        '
            1
        '
        """

        return 1

    @classmethod
    def from_value(cls, value):
        """
        Create scalar and infer type.

        >>> s = Scalar.from_value(0.5)
        >>> print(s.dtype)
        '
            <class 'pyspla.type.FLOAT'>
        '

        :param value: any.
            Value to create scalar from.

        :return: Scalar with value.
        """

        if isinstance(value, float):
            return Scalar(dtype=FLOAT, value=value)
        elif isinstance(value, int):
            return Scalar(dtype=INT, value=value)
        elif isinstance(value, bool):
            return Scalar(dtype=INT, value=value)
        else:
            raise Exception("cannot infer type")

    def set(self, value=None):
        """
        Set the value stored in the scalar. If no value passed the default value is set.

        >>> s = Scalar(INT)
        >>> s.set(10)
        >>> print(s)
        '
            10
        '

        :param value: optional: Any. default: None.
            Optional value to store in scalar.
        """

        check(self._dtype._scalar_set(self._hnd, self._dtype._c_type(value if value else 0)))

    def get(self):
        """
        Read the value stored in the scalar.

        >>> s = Scalar(INT, 10)
        >>> print(s.get())
        '
            10
        '

        :return: Value from scalar.
        """

        value = self._dtype._c_type(0)
        check(self._dtype._scalar_get(self._hnd, ctypes.byref(value)))
        return self._dtype.cast_value(value)

    def __str__(self):
        return str(self.get())

    def __iter__(self):
        return iter([self.get()])

    def __add__(self, other):
        return Scalar(dtype=self.dtype, value=self.get() + Scalar._value(other))

    def __sub__(self, other):
        return Scalar(dtype=self.dtype, value=self.get() + Scalar._value(other))

    def __mul__(self, other):
        return Scalar(dtype=self.dtype, value=self.get() * Scalar._value(other))

    def __truediv__(self, other):
        return Scalar(dtype=self.dtype, value=self.get() / Scalar._value(other))

    def __floordiv__(self, other):
        return Scalar(dtype=self.dtype, value=self.get() // Scalar._value(other))

    def __iadd__(self, other):
        self.set(self.get() + Scalar._value(other))
        return self

    def __isub__(self, other):
        self.set(self.get() - Scalar._value(other))
        return self

    def __imul__(self, other):
        self.set(self.get() * Scalar._value(other))
        return self

    def __idiv__(self, other):
        self.set(self.get() / Scalar._value(other))
        return self

    @classmethod
    def _value(cls, other):
        if isinstance(other, Scalar):
            return other.get()
        else:
            return other
