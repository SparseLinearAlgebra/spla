# spla

[![Build](https://github.com/JetBrains-Research/spla/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/JetBrains-Research/spla/actions/workflows/build.yml)

**spla** is a C++ sparse linear algebra library for multi-GPU computations based
on OpenCL and Boost.Compute technologies. It provides linear algebra primitives,
such as matrices, vectors and scalars, supports matrix-matrix, matrix-vector, vector-vector 
operations, and gives an ability to customize underlying values types and 
parametrise operations with arbitrary user defined functions.

**Core** of the library is C and C++ interfaces, which give access to library features.
Also, library has python **pyspla** package - wrapper for spla C API.
This package exports library features and primitives in high-level manner with automated 
resources management and fancy syntax sugar for native integration into Python-runtime.

> Note: project under heavy development!

### Features

- [ ] Library
- [ ] Arbitrary type
- [ ] Function unary
- [ ] Function binary  
- [ ] Sparse matrix
- [ ] Sparse vector
- [ ] Sparse matrix-matrix multiplication
- [ ] Sparse matrix-vector multiplication
- [ ] Kronecker product
- [ ] Element-wise matrix-matrix addition
- [ ] Element-wise vector-vector addition
- [ ] Sub-matrix extraction
- [ ] Sub-vector extraction

### Platforms

- Windows 10
- Linux-based OS (Ubuntu 20.04)

## Building from sources

### Prerequisites

- Common:
    - Git (to get source code)
    - CMake (the latest version)
    - Ninja (as build files generator)
    - Boost library (the latest version)
    - OpenCL 2.0+ SDK
    - Python 3.7+
- Windows 10: 
    - Microsoft Visual C++ compiler (MSVC) 
    - x64 Native Tools Command Prompt for VS

### Get source code   

The following script snippet downloads project source code repository, enters project root folder 
and runs submodules init in order to get dependencies source code initialized.
Must be executed from the folder where you want to locate project.

```shell
$ git clone https://github.com/JetBrains-Research/spla.git
$ cd spla
$ git submodule update --init --recursive
```

### Configure and run build

The following code snippet runs cmake build configuration process
with output into `build` directory, in `Release` mode with tests `SPLA_BUILD_TESTS=ON` enabled.
Then runs build process for `build` directory in verbose mode with `-j 4` four system threads.
Must be executed from project root folder.

```shell
$ cmake . -B build -DCMAKE_BUILD_TYPE=Release -DSPLA_BUILD_TESTS=ON
$ cmake --build build --target all --verbose -j 4
```

### Run unit-tests

The following code snippet executed python script, which allows to
run all native C++ library unit-tests, located in build directory,
specified in `--build-dir=build`. Must be executed from project root folder.

```shell
python ./scripts/run_tests.py --build-dir=build
```

## Directory structure

```
.
├── .github - GitHub Actions CI/CD setup 
├── docs - documents, text files and various helpful stuff
├── scripts - short utility programs 
├── include 
│   ├── spla-c - library public C API
│   └── spla-cpp - library public C++ API
├── sources - source code for library implementation
├── tests - gtest-based unit-tests collection
├── package - python-package files
│   ├── pyspla - library python wrapper source code
│   └── tests - python package regression tests   
├── deps - project dependencies
│   ├── gtest - google test framework for unit testing
│   └── compute - boost project C++ GPU computing library for OpenCL
└── CMakeLists.txt - library cmake config, add this as sub-directory to your project
```

## Contributors

- Egor Orachyov (Github: [@EgorOrachyov](https://github.com/EgorOrachyov))
- Semyon Grigorev (Github: [@gsvgit](https://github.com/gsvgit))

## Citation

```ignorelang
@online{spla,
  author = {Orachyov, Egor and Grigorev, Semyon},
  title = {spla: C++/Python sparse linear algebra library for multi-GPU computations},
  year = 2021,
  url = {https://github.com/JetBrains-Research/spla},
  note = {Version 0.0.0}
}
```

## License

This project licensed under MIT License. License text can be found in the
[license file](https://github.com/JetBrains-Research/spla/blob/master/LICENSE.md).

## Acknowledgments

This is a research project of the Programming Languages and Tools Laboratory
at JetBrains-Research. Laboratory website [link](https://research.jetbrains.org/groups/plt_lab/).