# spla <img align="right" width="20%" src="https://github.com/SparseLinearAlgebra/spla/raw/main/docs/logos/spla-logo.png?raw=true&sanitize=true">

[![Build](https://github.com/SparseLinearAlgebra/spla/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/SparseLinearAlgebra/spla/actions/workflows/build.yml)
[![Docs C/C++](https://github.com/SparseLinearAlgebra/spla/actions/workflows/docs-cpp.yml/badge.svg?branch=main)](https://SparseLinearAlgebra.github.io/spla/docs-cpp/)
[![Docs Python](https://github.com/SparseLinearAlgebra/spla/actions/workflows/docs-python.yml/badge.svg?branch=main)](https://SparseLinearAlgebra.github.io/spla/docs-python/)
[![Clang Format](https://github.com/SparseLinearAlgebra/spla/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/SparseLinearAlgebra/spla/actions/workflows/clang-format.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://github.com/SparseLinearAlgebra/spla/blob/master/LICENSE.md)

**spla** is an open-source generalized sparse linear algebra framework for mathematical computations with GPUs
acceleration. It provides linear algebra primitives, such as matrices, vectors and scalars, supports wide variety of
operations. It gives an ability to customize underlying values types treatment and parametrise operations using built-in
or custom user-defined functions.

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

## License

This project licensed under MIT License. License text can be found in the
[license file](https://github.com/SparseLinearAlgebra/spla/blob/master/LICENSE.md).