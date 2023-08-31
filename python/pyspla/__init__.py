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

Performance
-----------

**Comparison on a Nvidia GPU**

![stats](../../docs/stats/rq1_rel_compact.png)
Description: Relative speedup of GraphBLAST, Gunrock and Spla compared to a LaGraph (SuiteSparse) used a baseline. Logarithmic scale is used.

> **Configuration**: Ubuntu 20.04, 3.40Hz Intel Core i7-6700 4-core CPU, DDR4 64Gb RAM, Nvidia GeForce GTX 1070
> dedicated GPU with 8Gb on-board VRAM.

**Scalability on Intel, Amd and Nvidia GPUs**

![stats](../../docs/stats/rq2_cores_compact.png)
Description: Throughput of Spla library shown as a number of processed edges/s per single GPU core. Logarithmic scale is used.

> **Configuration**: Nvidia GeForce GTX 1070 dedicated GPU with 8Gb on-board VRAM, Intel Arc A770 flux dedicated GPU
> with 8GB on-board VRAM and or AMD Radeon Vega Frontier Edition dedicated GPU with 16GB on-board VRAM.

**Comparison running on integrated Intel and Amd GPUs**

![stats](../../docs/stats/rq3_int_compact.png)
Description: Relative speedup of Spla compared to a LaGraph (SuiteSparse) used a baseline running on a single CPU device with integrated GPU.

> **Configuration**: Ubuntu 20.04, 3.40Hz Intel Core i7-6700 4-core CPU, DDR4 64Gb RAM, Intel HD Graphics 530 integrated
> GPU and Ubuntu 22.04, 4.70Hz AMD Ryzen 9 7900x 12-core CPU, DDR4 128 GB RAM, AMD GFX1036 integrated GPU.

**Dataset**

| Name              | Vertices |   Edges | Avg Deg | Sd Deg |   Max Deg |                                                                                              Link |
|:------------------|---------:|--------:|--------:|-------:|----------:|--------------------------------------------------------------------------------------------------:|
| coAuthorsCiteseer |   227.3K |    1.6M |     7.2 |   10.6 |    1372.0 | [link](https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/coAuthorsCiteseer.tar.gz) |
| coPapersDBLP      |   540.5K |   30.5M |    56.4 |   66.2 |    3299.0 |      [link](https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/coPapersDBLP.tar.gz) |
| amazon-2008       |   735.3K |    7.0M |     9.6 |    7.6 |    1077.0 |                                                    [link](http://sparse.tamu.edu/LAW/amazon-2008) |
| hollywood-2009    |     1.1M |  112.8M |    98.9 |  271.9 |   11467.0 |         [link](https://suitesparse-collection-website.herokuapp.com/MM/LAW/hollywood-2009.tar.gz) |
| belgium_osm       |     1.4M |    3.1M |     2.2 |    0.5 |      10.0 |                                               [link](http://sparse.tamu.edu/DIMACS10/belgium_osm) |
| roadNet-CA        |     2.0M |    5.5M |     2.8 |    1.0 |      12.0 |            [link](https://suitesparse-collection-website.herokuapp.com/MM/SNAP/roadNet-CA.tar.gz) |
| com-Orkut         |     3.1M |  234.4M |    76.3 |  154.8 |   33313.0 |             [link](https://suitesparse-collection-website.herokuapp.com/MM/SNAP/com-Orkut.tar.gz) |
| cit-Patents       |     3.8M |   33.0M |     8.8 |   10.5 |     793.0 |           [link](https://suitesparse-collection-website.herokuapp.com/MM/SNAP/cit-Patents.tar.gz) |
| rgg_n_2_22_s0     |     4.2M |   60.7M |    14.5 |    3.8 |      36.0 |     [link](https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/rgg_n_2_22_s0.tar.gz) |
| soc-LiveJournal   |     4.8M |   85.7M |    17.7 |   52.0 |   20333.0 |      [link](https://suitesparse-collection-website.herokuapp.com/MM/SNAP/soc-LiveJournal1.tar.gz) |
| indochina-2004    |     7.4M |  302.0M |    40.7 |  329.6 |  256425.0 |         [link](https://suitesparse-collection-website.herokuapp.com/MM/LAW/indochina-2004.tar.gz) |
| rgg_n_2_23_s0     |     8.4M |  127.0M |    15.1 |    3.9 |      40.0 |     [link](https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/rgg_n_2_23_s0.tar.gz) |
| road_central      |    14.1M |   33.9M |     2.4 |    0.9 |       8.0 |                                              [link](http://sparse.tamu.edu/DIMACS10/road_central) |

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

Types
-----

Library provides a set of standard and common built-in data types. Library value types
differ a bit from a classic type definition. In spla library type is essentially is a
storage characteristic, which defines count and layout of bytes per element. User
can interpret stored data as her/she wants. Spla types set is limited due to the nature
of GPUs accelerations, where arbitrary layout of data causes significant performance penalties.

Ops
---

Library provides a set of unary, binary and select ops for values data manipulation inside
matrix and vector containers. Unary operations commonly used for apply and transformation
operations, binary operations used for reductions and products, select operations used for
filtration and mask application.

Details
-------

Spla C/C++ backend compiled library is automatically loaded and initialized on package import.
State of the library managed by internal `bridge` module. All resources are unloaded automatically
on package exit. Library state finalized automatically.

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

from .bridge import *

bridge.initialize()

from .library import *
from .descriptor import *
from .op import *
from .object import *
from .schedule import *
from .type import *
from .array import *
from .matrix import *
from .vector import *
from .scalar import *
from .version import *

__version__ = VERSIONS[-1]

__all__ = [
    "Type",
    "BOOL",
    "INT",
    "UINT",
    "FLOAT",
    "FormatMatrix",
    "FormatVector",
    "Descriptor",
    "Op",
    "OpUnary",
    "OpBinary",
    "OpSelect",
    "MemView",
    "Object",
    "Array",
    "Matrix",
    "Vector",
    "Scalar",
    "VERSIONS"
]
