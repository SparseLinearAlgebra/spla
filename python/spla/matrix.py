"""
Wrapped native (spla C API) matrix primitive python implementation.
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
