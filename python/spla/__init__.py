"""
Python spla package.

Generalized sparse liner algebra python package with GPUs accelerated
computations. Library provides a set of linear algebra primitives such as
`matrix` and `vector` for mathematical computations parametrized using
on of built-in `type`. It allows to define sequence of execution tasks
using `schedule` API. Desired behavior of math operations can be customized
using on of element operations in `op` module.

Library optionally uses GPUs acceleration through OpenCL or CUDA API.
It automatically attempts to initialize accelerator and trys to use
it to speed-up some operations. All GPU communication, data transformations
and transfers done internally automatically with any efforts from user perspective.

Containers
----------

Library provides fundamental generalized linear algebra containers
for data storage and mathematical computations. These containers
are generalized, so any of built-in types may be used to parametrize
type of data. Containers have sparse formats by default, so it is
possible to create large-dimension but low data containers. Containers
are storage-invariant, so the best format for the storage is automatically
managed by container internally. All required format conversations done
in the context of particular primitive usage.

* `Matrix` - Generalized sparse storage-invariant matrix primitive.
* `Vector` - Generalized sparse storage-invariant vector primitive.

Schedule
--------

Schedule allows to build sequence of tasks to be executed. It allows user
control the order of the tasks' execution, parallel execution of tasks
on some level, notification on some steps completion and etc.

* `Schedule` - Schedule object which may be executed by library.
* `Task` - A particular wort to be done inside a step of schedule.

Types
-----

Op
--

Usage information
-----------------

Details
-------

Spla C backend compiled library is automatically loaded and
initialized on package import. State of the library managed
by internal `bridge` module. All resources are unloaded automatically
on package exit. Library state finalized automatically.
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

from . import library
from . import op
from . import schedule
from . import type

from .matrix import *
from .vector import *
from .version import *

from . import bridge
