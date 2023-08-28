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
from .type import INT
from .object import Object


class Scalar(Object):
    """
    Generalized statically-typed scalar primitive.

    Attributes
    ----------

    - dtype : `type` type of stored matrix elements
    - shape : `2-tuple` shape of the scalar in form of two integers tuple (always 1x1)

    Notes
    -----

    Scalar provides features for:

    - pack native value
    - unpack native value
    - pass value as a param to some operation
    - get scalar as an operation result

    Scalar typical usage:

    - (1) Create scalar from python value
    - (2) Pass scalar as argument to matrix/vector operation
    - (3) Get scalar as a return value of some operation
    - (4) Unpack value on python side

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

        Parameters
        ----------

        dtype: optional: Type. default: INT.
            Type of the scalar value.

        value: optional: Any. default: None.
            Optional value to store in scalar on creation.

        hnd: optional: ctypes.c_void_p. default: None.
            Optional handle to C object to retain in this scalar instance.

        label: optional: str. default: None.
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
        """

        return self._dtype

    @property
    def shape(self):
        """
        2-tuple shape of the storage. For scalar object it is always 1 by 1.
        """

        return 1, 1

    @property
    def n_vals(self):
        """
        Number of stored values in the scalar. Always 1.
        """

        return 1

    def set(self, value=None):
        """
        Set the value stored in the scalar. If no value passed the default value is set.

        Parameters
        ----------

        value: optional: Any. default: None.
            Optional value to store in scalar.
        """

        check(self._dtype._scalar_set(self._hnd, self._dtype._c_type(value)))

    def get(self):
        """
        Read the value stored in the scalar.

        Returns
        -------

        Value from scalar.
        """

        value = self._dtype._c_type(0)
        check(self._dtype._scalar_get(self._hnd, ctypes.byref(value)))
        return self._dtype.to_py(value)

    def __str__(self):
        return str(self.get())

    def __iter__(self):
        return iter([self.get()])
