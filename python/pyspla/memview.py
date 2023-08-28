"""
Wrapped native (spla C API) memory view primitive implementation.
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
from .object import Object


class MemView(Object):
    """
    Wrapper for a memory view object from a library.

    Attributes
    ----------

    - buffer : `ctypes.c_void_p` hnd to native raw memory buffer
    - size : `int` size of the buffer in bytes
    - is_mutable : `bool` whenever buffer content can be modified

    Details
    -------

    Memory view is a wrapper for a raw memory resource. It allows to inspect
    spla library containers data without an extra overhead on copy operations
    and without explicit memory lifetime control.
    """

    __slots__ = ["_buffer", "_owner"]

    def __init__(self, label=None, hnd=None, buffer=None, size=None, mutable=None, owner=None):
        """
        Creates a new memory view from existing hnd or making new for a buffer resource.

        Parameters
        ----------

        label: optional: str. default: None.
            MemView name for debugging.

        hnd: optional: ctypes.c_void_p. default: None.
            MemView native void* handle to a C counterpart.

        buffer: optional: ctypes.c_void_p. default: None.
            Optional buffer in case making new view.

        size: optional: int. default: None.
            Optional size, must be provided if buffer provided.

        mutable: optional: bool. default: None.
            Optional flag if buffer content is mutable.

        owner: optional: Object. default: None.
            Optional owner of a given memory view.
        """

        super().__init__(None, None)

        self._buffer = buffer
        self._owner = owner

        if hnd is None:
            assert buffer
            assert size

            if mutable is None:
                mutable = 0

            hnd = ctypes.c_void_p(0)
            check(backend().spla_MemView_make(ctypes.byref(hnd), buffer, ctypes.c_size_t(size), ctypes.c_int(mutable)))

        super().__init__(label, hnd)

    @property
    def buffer(self):
        """
        Pointer to a native C-buffer viewed by this view.
        """
        c_buffer = ctypes.c_void_p(0)
        check(backend().spla_MemView_get_buffer(self._hnd, ctypes.byref(c_buffer)))
        return c_buffer

    @property
    def size(self):
        """
        Size of viewed memory buffer in bytes.
        """

        c_size = ctypes.c_size_t(0)
        check(backend().spla_MemView_get_size(self._hnd, ctypes.byref(c_size)))
        return int(c_size.value)

    @property
    def is_mutable(self):
        """
        True if the content of viewed memory buffer can be modified.
        """

        c_is_mutable = ctypes.c_int(0)
        check(backend().spla_MemView_get_buffer(self._hnd, ctypes.byref(c_is_mutable)))
        return bool(c_is_mutable.value)
