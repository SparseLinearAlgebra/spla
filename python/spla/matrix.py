"""
Wrapped native (spla C API) matrix primitive python implementation.
"""

__copyright__ = "Copyright (c) 2021-2022 JetBrains-Research"

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


class Matrix:
    """
    Generalized storage-invariant matrix primitive.

    Attributes
    __________

    * type : type
        type of stored matrix elements
    * shape : 2-tuple
        shape of the matrix in form of two integers tuple
    * hnd: p_void
        handle to the native matrix object in spla C API

    Notes
    _____

    Matrix class support all spla C API matrix functions.
    It provides bind functionality as well as new functions/methods for better python user experience.

    Matrix provides features for: incremental creation, build from full content,
    element-wise addition, subtraction, matrix-matrix, matrix-vector products.

    Matrix internally manages optimal format of stored data. Use hints to
    force matrix state and format changes.

    Matrix optional uses dedicated/integrated GPU to speedup computations
    using one of built-in OpenCL or CUDA accelerators.
    """

    def __init__(self):
        pass
