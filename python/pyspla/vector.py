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


class Vector:
    """
    Generalized statically-typed sparse storage-invariant vector primitive.

    Attributes
    ----------

    - type : `type` type of stored vector elements
    - shape : `2-tuple` shape of the vector in form of two integers tuple (second dim is 1)

    Notes
    -----

    Vector provides features for:

    - incremental creation
    - build from values
    - read-back by value
    - reduction to scalar
    - (tbd) element-wise addition
    - (tbd) element-wise subtraction
    - matrix-vector product

    Vector supports storage schemas:

    - cpu dense
    - acc dense (opencl)

    Vector typical usage:

    - (1) Instantiate vector primitive
    - (2) Build incrementally from yours data source
    - (3) Vector usage in a sequence of math operations
    - (4) Read-back vector data to python to analyse results

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

    def __init__(self):
        pass
