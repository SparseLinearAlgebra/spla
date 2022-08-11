# spla <img align="right" width="20%" src="https://github.com/JetBrains-Research/spla/raw/main/docs/logos/spla-logo.png?raw=true&sanitize=true">

[![JB Research](https://jb.gg/badges/research-flat-square.svg)](https://research.jetbrains.org/)
[![Build](https://github.com/JetBrains-Research/spla/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/JetBrains-Research/spla/actions/workflows/build.yml)
[![Docs C/C++](https://github.com/JetBrains-Research/spla/actions/workflows/docs-cpp.yml/badge.svg?branch=main)](https://jetbrains-research.github.io/spla/docs-cpp/)
[![Docs Python](https://github.com/JetBrains-Research/spla/actions/workflows/docs-python.yml/badge.svg?branch=main)](https://jetbrains-research.github.io/spla/docs-python/)
[![Clang Format](https://github.com/JetBrains-Research/spla/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/JetBrains-Research/spla/actions/workflows/clang-format.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://github.com/JetBrains-Research/spla/blob/master/LICENSE.md)

**spla** is an open-source generalized sparse linear algebra framework for mathematical computations with GPUs
acceleration. It provides linear algebra primitives, such as matrices, vectors and scalars, supports wide variety of
operations. It gives an ability to customize underlying values types treatment and parametrise operations using rich
pre-defined functions' set.

- **Source code**:
  [https://github.com/JetBrains-Research/spla](https://github.com/JetBrains-Research/spla)
- **Contributing**:
  [https://github.com/JetBrains-Research/spla/blob/main/CONTRIBUTING.md](https://github.com/JetBrains-Research/spla/blob/main/CONTRIBUTING.md)
- **Python API reference**:
  [https://jetbrains-research.github.io/spla/docs-python/spla](https://jetbrains-research.github.io/spla/docs-python/spla/)
- **C/C++ API reference**:
  [https://jetbrains-research.github.io/spla/docs-cpp](https://jetbrains-research.github.io/spla/docs-cpp/)
- **Bug report**:
  [https://github.com/JetBrains-Research/spla/issues](https://github.com/JetBrains-Research/spla/issues)

> Note: project under heavy development! Not ready for usage.

## Building from sources

### Prerequisites

- Common:
    - Git (to get source code)
    - CMake (the latest version)
    - Ninja (as build files generator)
    - OpenCL 1.2+ SDK
    - Python 3.7+
- Windows 10:
    - Microsoft Visual C++ Compiler (MSVC) with C++ 17 support
    - x64 Native Tools Command Prompt for VS
- Ubuntu 20.04:
    - GNU C++ Compiler with C++ 17 support
- MaсOS Catalina 10.15:
    - Clang Compiler with C++ 17 support

### Get source code

The following code snippet downloads project source code repository, enters project root folder and runs submodules init
in order to get dependencies source code initialized. Must be executed from the folder where you want to locate project.

```shell
$ git clone https://github.com/JetBrains-Research/spla.git
$ cd spla
$ git submodule update --init --recursive
```

### Configure and run build

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
- Gleb Marin (Github: [@Glebanister](https://github.com/Glebanister))
- Semyon Grigorev (Github: [@gsvgit](https://github.com/gsvgit))

## Citation

```ignorelang
@online{spla,
  author = {Orachyov, Egor and Marin, Gleb and Grigorev, Semyon},
  title = {spla: An open-source generalized sparse linear algebra framework for GPU computations},
  year = 2022,
  url = {https://github.com/JetBrains-Research/spla},
  note = {Version 1.0.0}
}
```

## Project structure

| Folder        | Description                                                 |
| :------------ | :---------------------------------------------------------- |
| `📁 .github`  | CI/CD scripts and GitHub related files                      |
| `📁 deps`     | Third-party project dependencies, stored as submodules      |
| `📁 docs`     | Documentations and digital stuff                            |
| `📁 examples` | Example applications of library C/C++ usage                 |
| `📁 include`  | Library public C/C++ header files                           |
| `📁 src`      | Library private compiled source directory                   |
| `📁 tests`    | Library C/C++ unit-tests                                    |
| `📁 python`   | Python package bindings for library API                     |

## License

This project licensed under MIT License. License text can be found in the
[license file](https://github.com/JetBrains-Research/spla/blob/master/LICENSE.md).

## Acknowledgments <img align="right" width="15%" src="https://github.com/JetBrains-Research/spla/raw/main/docs/logos/jetbrains-logo.png?raw=true&sanitize=true">

This is a research project of the Programming Languages and Tools Laboratory at JetBrains-Research. Laboratory
website [link](https://research.jetbrains.org/groups/plt_lab/).
