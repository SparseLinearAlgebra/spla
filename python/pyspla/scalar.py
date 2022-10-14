"""
Wrapped native (spla C API) scalar primitive implementation.
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


class Scalar:
    """
    Generalized statically-typed scalar primitive.

    Attributes
    ----------

    - type : `type` type of stored matrix elements
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

    def __init__(self):
        pass
