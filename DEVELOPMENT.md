# Spla project development guide

This document covers briefly project development strategy, architectural thoughts, essential and common routine tasks of
project service.

## Building library from sources

Prerequisites: CMake (the latest version), Ninja, a C++17 compiler, Python 3.7+.

The build is driven by `build.py`, which configures CMake with the Ninja generator and runs the actual build. Must be
executed from the project root folder:

```shell
$ python ./build.py --build-dir=build --build-type=Release --nt=4 --tests=YES --examples=YES
```

Options (defaults in parentheses):

- `--build-dir` — build directory (`build`)
- `--build-type` — `Debug`, `Release` or `RelWithDebInfo` (`Release`)
- `--tests` — build unit tests (`YES`)
- `--examples` — build example applications (`YES`)
- `--opencl` — build the OpenCL acceleration backend (`YES`; CI disables it where no GPU is available)
- `--target` — which CMake target to build (`all`)
- `--nt` — number of build threads (`4`)
- `--arch` — target architecture on macOS, `x86_64` or `arm64` (falls back to `CMAKE_SYSTEM_PROCESSOR`)
- `--verbose` — verbose compiler output

The corresponding CMake options are `SPLA_BUILD_TESTS`, `SPLA_BUILD_EXAMPLES` and `SPLA_BUILD_OPENCL`.

## Unit-tests execution

Unit tests are C++ (gtest) and are compiled with `--tests=YES`. Must be executed from the project root folder:

```shell
$ python ./run_tests.py --build-dir=build
```

To run on a specific OpenCL platform/device (GPU), pass `--platform <n> --device <n>`. GPU runs happen on self-hosted
CI runners; local runs use the CPU. A run is successful when its output contains no `FAILED` — this is how CI checks
the results as well.

## Release management of python package

The version of the Python package (`pyspla`) is stored in `python/VERSION.md` and mirrored into
`python/pyspla/version.py`. Update it with:

```shell
$ python ./bump_version.py -inc             # increment the patch component
$ python ./bump_version.py --bump X.Y.Z     # set an explicit version
```

Releases are published by CI after a successful Build workflow run, bundling the native binary built by that workflow
into the package:

- `release` branch → PyPI (`.github/workflows/deploy.yml`)
- `pre-release` branch → Test PyPI (`.github/workflows/deploy-test.yml`)

Record release changes in `CHANGELOG.md`.

## Quality checks

The local quality gate a change must pass before integration. All commands are executed from the project root folder:

1. **Build** — `python ./build.py --build-dir=build --build-type=Release --tests=YES`
2. **Unit tests** — `python ./run_tests.py --build-dir=build`; success is no `FAILED` in the output (GPU runs only on
   self-hosted CI, the local gate is CPU)
3. **Style** — clang-format **v20** (the version must match CI's) over the C/C++ sources of `include/`, `src/`,
   `tests/` and `examples/` with the repository `.clang-format`:

   ```shell
   $ find include src tests examples \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' \) -print0 \
       | xargs -0 clang-format --dry-run -Werror
   ```

4. **Docs build** — the C/C++ API reference with Doxygen (output to `html/`) and the Python API reference with pdoc:

   ```shell
   $ doxygen Doxyfile
   $ SPLA_DOCS=true pdoc -o pydocs python/pyspla
   ```

CI runs the same checks; the workflows are the source of truth for what CI executes:

| Workflow | Runs |
| :------- | :--- |
| `.github/workflows/build.yml` | library build on Windows and macOS |
| `.github/workflows/ubuntu.yml` | build + CPU unit tests on Ubuntu |
| `.github/workflows/self-hosted.yml` | build + GPU unit tests (NVIDIA, AMD) on self-hosted runners |
| `.github/workflows/clang-format.yml` | clang-format v20 check over `include/`, `src/`, `tests/`, `examples/` |
| `.github/workflows/docs-cpp.yml` | Doxygen build and publish (push to `main`) |
| `.github/workflows/docs-python.yml` | pdoc build and publish (push to `main`) |

Local tool requirements: clang-format 20, doxygen (with graphviz `dot` for diagrams), pdoc.
