<div align="center">
  <img src="https://github.com/SparseLinearAlgebra/spla/raw/main/docs/logos/spla-logo-light.png?raw=true&sanitize=true">
</div>

[![Build](https://github.com/SparseLinearAlgebra/spla/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/SparseLinearAlgebra/spla/actions/workflows/build.yml)
[![Python Package](https://github.com/SparseLinearAlgebra/spla/actions/workflows/deploy.yml/badge.svg)](https://pypi.org/project/pyspla/)
[![Python Package (Test)](https://github.com/SparseLinearAlgebra/spla/actions/workflows/deploy-test.yml/badge.svg)](https://test.pypi.org/project/pyspla/)
[![Docs C/C++](https://github.com/SparseLinearAlgebra/spla/actions/workflows/docs-cpp.yml/badge.svg?branch=main)](https://SparseLinearAlgebra.github.io/spla/docs-cpp/)
[![Docs Python](https://github.com/SparseLinearAlgebra/spla/actions/workflows/docs-python.yml/badge.svg?branch=main)](https://SparseLinearAlgebra.github.io/spla/docs-python/pyspla/)
[![Clang Format](https://github.com/SparseLinearAlgebra/spla/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/SparseLinearAlgebra/spla/actions/workflows/clang-format.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://github.com/SparseLinearAlgebra/spla/blob/master/LICENSE.md)

**spla** is an open-source generalized sparse linear algebra framework for mathematical computations with GPUs
acceleration. It provides linear algebra primitives, such as matrices, vectors and scalars, supports wide variety of
operations. It gives an ability to customize underlying values types treatment and parametrise operations using built-in
or custom user-defined functions.

- **Website**:
  [https://SparseLinearAlgebra.github.io/pyspla/](https://SparseLinearAlgebra.github.io/spla/docs-python/pyspla/)
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
- **C/C++ API reference**:
  [https://SparseLinearAlgebra.github.io/spla/docs-cpp](https://SparseLinearAlgebra.github.io/spla/docs-cpp/)
- **Bug report**:
  [https://github.com/SparseLinearAlgebra/spla/issues](https://github.com/SparseLinearAlgebra/spla/issues)

> Note: project under heavy development! Not ready for usage.

## Installation

Install the release version of the package from **PyPI** repository for Windows, Linux and MacOS:

```shell
$ pip install pyspla
```

Install the latest test version of the package from **Test PyPI** repository for Windows, Linux and MacOS:

```shell
$ pip install -i https://test.pypi.org/simple/ pyspla
```

Delete package if no more required:

```shell
$ pip uninstall pyspla
```

## Building from sources

### Prerequisites

- **Common**:
    - Git (to get source code)
    - CMake (the latest version)
    - Ninja (as build files generator)
    - Python 3.7+
- **Windows 10**:
    - Microsoft Visual C++ Compiler (MSVC) with C++ 17 support
    - x64 Native Tools Command Prompt for VS
- **Ubuntu 20.04**:
    - GNU C++ Compiler with C++ 17 support
- **MaсOS Catalina 10.15**:
    - Clang Compiler with C++ 17 support

### Get source code

The following code snippet downloads project source code repository, and enters project root folder. Must be executed
from the folder where you want to locate project.

```shell
$ git clone https://github.com/SparseLinearAlgebra/spla.git
$ cd spla
```

### Configure and run build

> **Attention!** On Windows platform building commands must be executed in `x64 Native Tools Command Prompt for VS`.

The following code snippet runs `build.py` script, which allows configuring cmake and running of actual build with
selected options. You can specify build directory, build type, number of system threads for build, enable or disable
optionally building of tests and example applications. Must be executed from project root folder.

```shell
$ python ./build.py --build-dir=build --build-type=Release --nt=4 --tests=YES --examples=YES
```

On macOS, you can optionally specify target binaries architecture to build. Pass option `--arch`
with `x86_64` or `arm64` respectively. By default, build falls back to `CMAKE_SYSTEM_PROCESSOR` specified architecture.
See example bellow, replace `<arch>` with desired architecture for your build. Must be executed from project root
folder.

```shell
$ python ./build.py --build-dir=build --build-type=Release --nt=4 --arch=<arch>
```

### Run unit-tests

The following code snippet executed python script, which allows to run all native C++ library unit-tests, located in
build directory, specified in `--build-dir` option. Must be executed from project root folder.

```shell
$ python ./run_tests.py --build-dir=build
```

## Contributors

- Egor Orachyov (Github: [@EgorOrachyov](https://github.com/EgorOrachyov))
- Semyon Grigorev (Github: [@gsvgit](https://github.com/gsvgit))

## Citation

```ignorelang
@online{spla,
  author = {Orachyov, Egor and Grigorev, Semyon},
  title = {spla: An open-source generalized sparse linear algebra framework for GPU computations},
  year = 2022,
  url = {https://github.com/SparseLinearAlgebra/spla},
  note = {Version 1.0.0}
}
```

## Project structure

| Entry                  | Description                                                        |
| :--------------------- | :----------------------------------------------------------------- |
| `📁 .github`           | CI/CD scripts and GitHub related files                             |
| `📁 deps`              | Third-party project dependencies, stored as submodules             |
| `📁 docs`              | Documentations and digital stuff                                   |
| `📁 examples`          | Example applications of library C/C++ usage                        |
| `📁 include`           | Library public C/C++ header files                                  |
| `📁 src`               | Library private compiled source directory                          |
| `📁 tests`             | Library C/C++ unit-tests                                           |
| `📁 python`            | Python package bindings for library API                            |
| `📄 CMakeLists.txt`    | CMake library configuration, add as sub directory to your project  |
| `📄 build.py`          | Script to build library sources, tests and examples                |
| `📄 bump_version.py`   | Script to increment or update version of package before release    |
| `📄 run_tests.py`      | Script to run compiled library unit tests                          |
| `📄 generate.py`       | Script to re-generate `.hpp` bindings from `.cl` source files      |

## License

This project licensed under MIT License. License text can be found in the
[license file](https://github.com/SparseLinearAlgebra/spla/blob/master/LICENSE.md).