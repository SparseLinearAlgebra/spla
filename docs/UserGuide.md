# User Guide

The user guide for the spla library. Contains usage scenarios for the configuration mechanism: from a quick start without configuration to running tests in CI. The complete reference of all parameters, flags and environment variables is given in the [User Manual](UserManual.md).

## Contents

1. [Environment check](#environment-check)
2. [Scenario 1. Quick start without configuration](#scenario-1-quick-start-without-configuration)
3. [Scenario 2. Choosing between CPU and GPU without specifying indexes](#scenario-2-choosing-between-cpu-and-gpu-without-specifying-indexes)
4. [Scenario 3. Multiple devices and modes in one file](#scenario-3-multiple-devices-and-modes-in-one-file)
5. [Scenario 4. Running in CI with a file from the repository](#scenario-4-running-in-ci-with-a-file-from-the-repository)
6. [Priority of configuration sources](#priority-of-configuration-sources)
7. [Where the configuration files are located](#where-the-configuration-files-are-located)

## Environment check

After installation, run the command that confirms the library is ready to use:

```bash
spla --softcheck
```

The command:

1. Reads the configuration file shipped with the library.
2. Initializes OpenCL, selects the platform and device.
3. Runs a simple test task `2 + 3`.
4. Reports the status.

Example of successful output:

```bash
$ spla --softcheck
[spla:check] Starting sanity check...
[spla:check] Initializing OpenCL...
[spla:check] Selected accelerator: OpenCL NVIDIA CUDA | device: NVIDIA GeForce RTX 3070 Laptop GPU | vendor: nvidia | mcu: 40 | wave: 32 | mwgs: 1024
[spla:check] Running test task: 2 + 3 = ...
[spla:check] Result: 5 (expected 5)
[spla:check] SUCCESS - spla works correctly
```

## Scenario 1. Quick start without configuration

You want a working application without configuration - nothing needs to be configured. The library uses the default parameters from the configuration file shipped with the library.

```bash
./program
```

The scenario works the same on Linux, macOS and Windows. If GPU is unavailable, the library will automatically switch to CPU by default (the `if_gpu_unavailable: "use_cpu"` parameter), reporting it.

Before the first run it is recommended to run [`spla --softcheck`](#environment-check) to make sure that everything required for spla is installed.

## Scenario 2. Choosing between CPU and GPU without specifying indexes

You want to control the device type, but not specify concrete OpenCL platform and device indexes.

The device type is set by the `backend` parameter in the configuration file:

```json
{
    "gpu_only": { "backend": "gpu" },
    "cpu_only": { "backend": "cpu" },
    "any":      { "backend": "any" }
}
```

- `"gpu"` - use the first available GPU,
- `"cpu"` - use the CPU,
- `"any"` - use the first available device of any type.

The `platform_index` and `device_index` indexes are ignored with these values.

### Behavior when GPU is unavailable

If `backend: "gpu"` is selected, but GPU is unavailable, the behavior is defined by the `if_gpu_unavailable` parameter:

```json
{
    "gpu_or_cpu": {
        "backend": "gpu",
        "if_gpu_unavailable": "use_cpu"
    },
    "gpu_only": {
        "backend": "gpu",
        "if_gpu_unavailable": "abort"
    }
}
```

- `"use_cpu"` - the library switches to CPU and continues working,
- `"abort"` -
 the library returns an error and does not continue.

Running the required configuration:

```bash
./program --spla-config=gpu_or_cpu
./program --spla-config=gpu_only
```

`if_gpu_unavailable` is applied with `backend: "gpu"`, `"any"` and `"by_index"`. With `backend: "cpu"` the parameter is ignored.

## Scenario 3. Multiple devices and modes in one file

You regularly run the same program on different GPUs and in different modes - for example, on GPU 0 and 1, in debug mode and in performance measurement mode. Instead of several configuration files, you describe everything in one.

```json
{
    "default": { "backend": "gpu", "verbosity": 2 },

    "gpu0": {
        "extends": ["default"],
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 0
    },
    "gpu1": {
        "extends": ["default"],
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 1
    },

    "debug": {
        "extends": ["default"],
        "verbosity": 3,
        "profiling": true
    },
    "bench": {
        "extends": ["default"],
        "verbosity": 0
    },

    "debug_gpu1": { "extends": ["debug", "gpu1"] },
    "bench_gpu0": { "extends": ["bench", "gpu0"] }
}
```

Running:

```bash
./program --spla-config=gpu1
./program --spla-config=debug
./program --spla-config=debug_gpu1
./program --spla-config=bench_gpu0
```

Configurations inherited through `extends` are applied in order: each next parent overrides the previous one, and the parameters of the configuration itself override all parents. Circular inheritance is detected and results in an error.

## Scenario 4. Running in CI with a file from the repository

You need to run tests in CI with a separate configuration file.

In a CI environment it is inconvenient and undesirable to touch the system and user configuration files, and the test configurations must live in the repository. Therefore the configurations are described in a separate file inside the repository and its path is given at launch together with the configuration name - these are the command line arguments `--spla-config-file` and `--spla-config`.

```
my_project/
├── .github/
│   └── workflows/
│       ├── ci.yml
│       └── ci_spla_conf.json
├── src/
└── tests/
```

### ci_spla_conf.json

```json
{
    "test_cpu": {
        "backend": "cpu",
        "queues_count": 1,
        "profiling": false,
        "allocator_type": "general",
        "verbosity": 1
    },
    "test_gpu0": {
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 0,
        "if_gpu_unavailable": "abort",
        "queues_count": 2,
        "profiling": false,
        "allocator_type": "general",
        "verbosity": 0
    },
    "test_gpu1": {
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 1,
        "if_gpu_unavailable": "abort",
        "queues_count": 2,
        "profiling": false,
        "allocator_type": "linear",
        "linear_alloc_size": 8,
        "verbosity": 0
    }
}
```

### ci.yml

```yml
name: CI

on: [push, pull_request]

jobs:
  test:
    name: Test (${{ matrix.config }})
    runs-on: [self-hosted, gpu]

    strategy:
      fail-fast: false
      matrix:
        config: [test_cpu, test_gpu0, test_gpu1]

    steps:
      - name: Checkout repository
        uses: actions/checkout@v4

      - name: Set up Python
        uses: actions/setup-python@v5
        with:
          python-version: "3.11"

      - name: Install dependencies
        run: |
          python -m pip install --upgrade pip
          pip install pyspla pytest

      - name: Run tests
        run: python src/my_app.py --spla-config-file=.github/workflows/ci_spla_conf.json --spla-config=${{ matrix.config }}
```

The file path and the configuration name are passed at launch as command arguments, not through environment variables - in CI this makes the launch command self-contained and reproducible.

## Priority of configuration sources

Sources are applied in descending priority order: parameters from a higher-priority source override lower-priority ones. This matters for the scenarios: any flag or environment variable overrides values from files.

| # | Source |
|---:|:---|
| 1 | Command line arguments (`--spla-*`) |
| 2 | Environment variables (`SPLA_*`) |
| 3 | File with an explicitly specified path (`--spla-config-file`) |
| 4 | User configuration file |
| 5 | System configuration file |
| 6 | Configuration file shipped with the library |

If the configuration name is not specified, the `"default"` configuration from the file shipped with the library is used. For details on the collection order and merging of sources, see the [User Manual](UserManual.md#configuration-mechanism-overview).

## Where the configuration files are located

- **Shipped with the library** - next to the library file (in the pyspla package - inside the installed package).
- **System** - Linux: `/etc/spla/spla_conf.json`.
- **User** - Linux: `~/.config/spla/spla_conf.json`.
- **Explicit** - any path via `--spla-config-file`.

Paths for macOS and Windows and the rules for "file missing / file cannot be opened" are in the [User Manual](UserManual.md#configuration-files).