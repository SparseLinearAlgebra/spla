# spla

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
- Linux-based OS 

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