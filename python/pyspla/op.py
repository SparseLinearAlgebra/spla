"""
Wrapped native (spla C API) op implementation.
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

from .object import Object

__all__ = [
    "Op",
    "OpUnary",
    "OpBinary",
    "OpSelect"
]


class Op(Object):
    """
    Wrapper for a spla Op (operation) object.

    Notes
    -----

    Operation represents some callable function with a return value.
    This function may be used for mathematical calculations with
    library primitives such as scalars, vectors and matrices.
    """

    __slots__ = ["_name", "_dtype_res"]

    def __init__(self, hnd, name, dtype_res, label=None):
        """
        Creates new operation from a native C api object handle.

        :param  hnd: ctypes.c_void_p.
            Mandatory handle to native C object.

        :param name: str.
            Mandatory name of the operation.

        :param dtype_res: Type.
            Type of the return value of the operation.

        :param label: optional: str. default: None.
            Optional debug label to set for the object.
        """

        assert hnd
        assert name
        assert dtype_res

        super().__init__(hnd, label)

        self._name = name
        self._dtype_res = dtype_res

    @property
    def name(self):
        """
        Name of operation.
        """
        return self._name

    @property
    def dtype_res(self):
        """
        Data type of the return value of the operation.
        """
        return self._dtype_res


class OpUnary(Op):
    """
    Wrapper for a spla OpUnary (unary operation) object.

    Notes
    -----

    Unary operation is a callable function with a signature `T0 -> R`.
    This function may be used in transformation and apply operations.
    """

    __slots__ = ["_dtype_arg0"]

    def __init__(self, hnd, name, dtype_res, dtype_arg0, label=None):
        """
        Creates new unary operation from a native C api object handle.

        :param hnd: ctypes.c_void_p.
            Mandatory handle to native C object.

        :param name: str.
            Mandatory name of the operation.

        :param dtype_res: Type.
            Type of the return value of the operation.

        :param dtype_arg0: Type.
            Type of the first operation argument.

        :param label: optional: str. default: None.
            Optional debug label to set for the object.
        """

        super().__init__(hnd, name, dtype_res, label)

        self._dtype_arg0 = dtype_arg0

    @property
    def dtype_arg0(self):
        """
        Data type of the first argument of the unary operation.
        """
        return self._dtype_arg0


class OpBinary(Op):
    """
    Wrapper for a spla OpBinary (binary operation) object.

    Notes
    -----

    Binary operation is a callable function with signature `T0 x T1 -> R`.
    It is commonly used in math matrix vector products, reductions, etc.
    """

    __slots__ = ["_dtype_arg0", "_dtype_arg1"]

    def __init__(self, hnd, name, dtype_res, dtype_arg0, dtype_arg1, label=None):
        """
        Creates new binary operation from a native C api object handle.

        :param hnd: ctypes.c_void_p.
            Mandatory handle to native C object.

        :param name: str.
            Mandatory name of the operation.

        :param dtype_res: Type.
            Type of the return value of the operation.

        :param dtype_arg0: Type.
            Type of the first operation argument.

        :param dtype_arg1: Type.
            Type of the second operation argument.

        :param  label: optional: str. default: None.
            Optional debug label to set for the object.
        """

        super().__init__(hnd, name, dtype_res, label)

        self._dtype_arg0 = dtype_arg0
        self._dtype_arg1 = dtype_arg1

    @property
    def dtype_arg0(self):
        """
        Data type of the first argument of the binary operation.
        """
        return self._dtype_arg0

    @property
    def dtype_arg1(self):
        """
        Data type of the second argument of the binary operation.
        """
        return self._dtype_arg1


class OpSelect(Op):
    """
    Wrapper for a spla OpSelect (select operation) object.

    Notes
    -----

    Selection operation is a special kind of callable function with signature `T0 -> BOOL`.
    This type of `unary predicate` usually used for masking and filtering operations.
    """

    __slots__ = ["_dtype_arg0"]

    def __init__(self, hnd, name, dtype_arg0, label=None):
        """
        Creates new select operation from a native C api object handle.

        :param hnd: ctypes.c_void_p.
            Mandatory handle to native C object.

        :param name: str.
            Mandatory name of the operation.

        :param dtype_arg0: Type.
            Type of the first operation argument.

        :param label: optional: str. default: None.
            Optional debug label to set for the object.
        """

        from .type import BOOL

        super().__init__(hnd, name, BOOL, label)

        self._dtype_arg0 = dtype_arg0

    @property
    def dtype_arg0(self):
        """
        Data type of the first argument of the select operation.
        """
        return self._dtype_arg0
