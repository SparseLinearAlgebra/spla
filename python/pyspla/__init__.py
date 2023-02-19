"""
Python wrapper for spla library
===============================

Cross-platform generalized sparse linear algebra framework for efficient mathematical
computations over sparse matrices and vectors with vendor-agnostic GPUs
acceleration to speed-up processing of large and complex data.
Library underling core witten using C++ with optional C-compatible interface.

Links:

- **Package page**:
  [https://pypi.org/project/pyspla](https://pypi.org/project/pyspla/)
- **Package page (test)**:
  [https://test.pypi.org/project/pyspla](https://test.pypi.org/project/pyspla/)
- **Source code**:
  [https://github.com/SparseLinearAlgebra/spla](https://github.com/SparseLinearAlgebra/spla)
- **Contributing**:
  [https://github.com/SparseLinearAlgebra/spla/CONTRIBUTING.md](https://github.com/SparseLinearAlgebra/spla/blob/main/CONTRIBUTING.md)
- **Development**:
  [https://github.com/SparseLinearAlgebra/spla/DEVELOPMENT.md](https://github.com/SparseLinearAlgebra/spla/blob/main/DEVELOPMENT.md)
- **Examples**:
  [https://github.com/SparseLinearAlgebra/spla/EXAMPLES.md](https://github.com/SparseLinearAlgebra/spla/blob/main/EXAMPLES.md)
- **C/C++ API Reference**:
  [https://SparseLinearAlgebra.github.io/spla/docs-cpp](https://SparseLinearAlgebra.github.io/spla/docs-cpp/)
- **Bug report**:
  [https://github.com/SparseLinearAlgebra/spla/issues](https://github.com/SparseLinearAlgebra/spla/issues)

We are welcome for contributions. Join project development on [GitHub](https://github.com/SparseLinearAlgebra/spla)!

Installation
------------

Install the release version of the package from **PyPI** repository for Windows, Linux and MacOS:

    $ pip install pyspla

Install the latest test version of the package from **Test PyPI** repository for Windows, Linux and MacOS:

    $ pip install -i https://test.pypi.org/simple/ pyspla

Delete package if no more required:

    $ pip uninstall pyspla

Summary
-------

Generalized sparse liner algebra python package with GPUs accelerated computations. Library provides a set of
linear algebra primitives such as `matrix`, `vector` and `scalar` for mathematical computations parametrized
using one of built-in `type`. It allows to define sequence of execution tasks using `schedule` API. Desired
behavior of math operations can be customized using pre-defined or custom user-defined element operations in `op`
module.

Library optionally uses GPUs acceleration through OpenCL or CUDA API.
It automatically attempts to initialize accelerator and trys to use
it to speed-up some operations. All GPU communication, data transformations
and transfers done internally automatically without any efforts from user perspective.

Containers
----------

Library provides fundamental generalized linear algebra containers
for data storage and mathematical computations. These containers
are generalized, so any of built-in types may be used to parametrize
type of data. Containers have sparse formats by default, so it is
possible to create large-dimension but low data containers. Containers
are storage-invariant, so the best format for the storage is automatically
managed by container internally. All required format conversion done
in the context of particular primitive usage.

- `Matrix` - Generalized statically-typed sparse storage-invariant matrix primitive.
- `Vector` - Generalized statically-typed sparse storage-invariant vector primitive.
- `Scalar` - Generalized statically-typed scalar primitive.

Schedule
--------

Schedule allows to build sequence of tasks to be executed. It allows user
control the order of the tasks' execution, parallel execution of tasks
on some level, notification on some steps completion and etc.

- `Schedule` - Schedule object which may be executed by library.
- `Task` - A particular wort to be done inside a step of schedule.

Types
-----

Library provides a set of standard and common built-in data types. Library value types
differ a bit from a classic type definition. In spla library type is essentially is a
storage characteristic, which defines count and layout of bytes per element. User
can interpret stored data as her/she wants. Spla types set is limited due to the nature
of GPUs accelerations, where arbitrary layout of data causes significant performance penalties.

- `INT`   - 4-byte-sized signed integral value
- `UINT`  - 4-byte-sized unsigned integral value
- `FLOAT` - 4-byte-sized single-precision floating point value

Op
--

Library provides a set of unary, binary and select ops for values data manipulation inside
matrix and vector containers.

- `PLUS`   - binary(x,y): r = x + y
- `MINUS`  - binary(x,y): r = x - y
- `MULT`   - binary(x,y): r = x * y
- `DIV`    - binary(x,y): r = x / y
- `FIRST`  - binary(x,y): r = x
- `SECOND` - binary(x,y): r = y
- `ONE`    - binary(x,y): r = 1
- `MIN`    - binary(x,y): r = min(x, y)
- `MAX`    - binary(x,y): r = max(x, y)
- `BOR`    - binary(x,y): r = x | y, for integral only
- `BAND`   - binary(x,y): r = x & y, for integral only
- `BXOR`   - binary(x,y): r = x ^ y, for integral only

Usage information
-----------------

TBD.

Details
-------

Spla C/C++ backend compiled library is automatically loaded and initialized on package import.
State of the library managed by internal `bridge` module. All resources are unloaded automatically
on package exit. Library state finalized automatically.
"""

__copyright__ = "Copyright (c) 2021-2022 SparseLinearAlgebra"

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

from .library import *
from .op import *
from .object import *
from .schedule import *
from .type import *
from .matrix import *
from .vector import *
from .scalar import *
from .version import *
from .bridge import *

__version__ = VERSIONS[-1]

__all__ = [
    "Object",
    "Matrix",
    "Vector",
    "Scalar",
    "VERSIONS"
]
