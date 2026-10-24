# User Manual

A manual for the spla library configuration mechanism. This is a reference: a complete description of the parameters, configuration sources, validation rules and statuses. Practical usage scenarios are given in the [User Guide](UserGuide.md).

## Contents

1. [NAME](#name)
2. [CONFIGURATION MECHANISM OVERVIEW](#configuration-mechanism-overview)
3. [COMMAND LINE PARAMETERS](#command-line-parameters)
4. [ENVIRONMENT VARIABLES](#environment-variables)
5. [CONFIGURATION FILES](#configuration-files)
6. [CONFIGURATIONS AND INHERITANCE](#configurations-and-inheritance)
7. [PARAMETER VALIDATION](#parameter-validation)
8. [STATUSES AND ERROR BEHAVIOR](#statuses-and-error-behavior)

## NAME

spla - a framework for GPU-accelerated mathematical computations. The library provides sparse linear algebra primitives (matrices, vectors, scalars) and a wide range of operations.

For most users it matters on which device their project runs, so the library provides a configuration mechanism: explicit GPU selection, OpenCL platform, number of command queues and other parameters that affect performance and user convenience.

## CONFIGURATION MECHANISM OVERVIEW

The spla configuration is assembled from six sources, applied in descending order of priority.

| # | Source | How it is set |
|---:|:---|:---|
| 1 | Command line arguments | `--spla-*` flags |
| 2 | Environment variables | `SPLA_*` variables |
| 3 | File with an explicitly specified path | the `--spla-config-file` flag or `SPLA_CONFIG_FILE` |
| 4 | User file | `~/.config/spla/spla_conf.json` (Linux) and equivalents |
| 5 | System file | `/etc/spla/spla_conf.json` (Linux) and equivalents |
| 6 | Library file | `spla_conf.json` next to the library file |

Collection order:

1. Configuration files are read (6 → 5 → 4 → 3) and **merged by configuration name**: configurations with the same name from different files are merged parameter by parameter, unique ones are added.
2. The active configuration is selected: the name from `--spla-config` / `SPLA_CONFIG`, `"default"` by default.
3. The selected configuration is applied together with the parents from `extends`.
4. Environment variables are applied on top, then command line arguments (highest priority).

The result goes into the final configuration, which is used when initializing the accelerator (platform and device selection, creation of command queues, allocator, verbosity level).

## COMMAND LINE PARAMETERS

All spla flags start with `--spla-`. Flags that do not start with `--spla-` belong to the program using the library and are ignored by the configuration mechanism.

| Flag | Type | Description |
|:---|:---|:---|
| `--spla-help` | - | Show help and exit |
| `--spla-version` | - | Show the library version and exit |
| `--spla-config` | string | Name of the configuration to apply. By default the `"default"` configuration is used |
| `--spla-config-file` | path | Path to a configuration file. |
| `--spla-backend` | string | Backend: `opencl_gpu`, `cpu`, `by_index` |
| `--spla-platform-index` | int | OpenCL platform index. Only when `backend=by_index` |
| `--spla-device-index` | int | Index of the OpenCL device within the platform. Only when `backend=by_index` |
| `--spla-if-opencl-unavailable` | string | Behavior when OpenCL acceleration is unavailable: `use_cpu` - switch to pure native CPU, `abort` - return an error |
| `--spla-queues-count` | int | Number of OpenCL command queues |
| `--spla-profiling` | bool | Profiling of command queues: `true`, `false` |
| `--spla-allocator-type` | string | Allocator type: `general`, `linear` |
| `--spla-linear-alloc-size` | int | Linear allocator size in bytes. Required when `allocator_type=linear`, ignored when `general` |
| `--spla-verbosity` | int | Verbosity level: `0` - no output, `1` - errors only, `2` - errors and warnings, `3` - all messages |

### Example

```bash
./my_program --spla-config-file=/etc/spla/spla_conf.json --spla-config=bench --spla-verbosity=0
```

> **Logging.** Library diagnostics are printed to `stdout` (informational messages) and `stderr` (errors and warnings). `--spla-verbosity` controls the amount of output: `0` disables it completely, `3` prints the full configuration used to initialize the accelerator (backend, platform, device, queues, allocator). This works in release builds too.

## ENVIRONMENT VARIABLES

For each command line parameter (except `--spla-help` and `--spla-version`) there is an environment variable equivalent.

| Variable | Flag equivalent |
|:---|:---|
| `SPLA_CONFIG` | `--spla-config` |
| `SPLA_CONFIG_FILE` | `--spla-config-file` |
| `SPLA_BACKEND` | `--spla-backend` |
| `SPLA_PLATFORM_INDEX` | `--spla-platform-index` |
| `SPLA_DEVICE_INDEX` | `--spla-device-index` |
| `SPLA_IF_OPENCL_UNAVAILABLE` | `--spla-if-opencl-unavailable` |
| `SPLA_QUEUES_COUNT` | `--spla-queues-count` |
| `SPLA_PROFILING` | `--spla-profiling` |
| `SPLA_ALLOCATOR_TYPE` | `--spla-allocator-type` |
| `SPLA_LINEAR_ALLOC_SIZE` | `--spla-linear-alloc-size` |
| `SPLA_VERBOSITY` | `--spla-verbosity` |

### Example

```bash
SPLA_CONFIG=bench SPLA_VERBOSITY=1 ./my_program
```

## CONFIGURATION FILES

The file format is JSON. A file consists of a set of named configurations; each configuration is a set of library parameters.

### Configuration keys

| Key | Type | Description | Requirements |
|:---|:---|:---|:---|
| `extends` | array of string | List of parent configurations | Names of configurations from all file levels |
| `backend` | string | Backend: `opencl_gpu`, `cpu`, `by_index` | Required |
| `platform_index` | int | OpenCL platform index | Required when `backend=by_index`, otherwise ignored |
| `device_index` | int | OpenCL device index | Required when `backend=by_index`, otherwise ignored |
| `if_opencl_unavailable` | string | Behavior when OpenCL acceleration is unavailable: `use_cpu`, `abort` | Required when `backend != cpu`, ignored when `backend=cpu` |
| `queues_count` | int | Number of command queues | Required, &ge; 1 |
| `profiling` | bool | Queue profiling | Required |
| `allocator_type` | string | Allocator type: `general`, `linear` | Required |
| `linear_alloc_size` | int | Linear allocator size (bytes) | Required when `allocator_type=linear`, ignored when `general` |
| `verbosity` | int | Verbosity level 0..3 | Required |

### File shipped with the library

The configuration name `"default"` can be defined **only** in the file shipped with the library. Defining a configuration named `"default"` in any other file is a parsing error.

The shipped file has the following structure:

```json
{
    "default": {
        "backend": "opencl_gpu",
        "if_opencl_unavailable": "use_cpu",

        "queues_count": 1,
        "profiling": false,
        "allocator_type": "general",

        "verbosity": 2
    }
}
```

### File locations

**Library file** - next to the library file: Windows - next to the DLL, Linux/macOS - next to the `.so`/`.dylib` (the path is determined via the address of the loaded library). Within the pyspla package - inside the installed Python package directory.

**System file:**

> Linux: `/etc/spla/spla_conf.json` \
> macOS: `/Library/Application Support/spla/spla_conf.json` \
> Windows: `%ProgramData%\spla\spla_conf.json`

**User file:**

> Linux: `~/.config/spla/spla_conf.json` \
> macOS: `~/Library/Application Support/spla/spla_conf.json` \
> Windows: `%APPDATA%\spla\spla_conf.json`

**Explicitly set file** - any path given via `--spla-config-file` / `SPLA_CONFIG_FILE`.

### Semantics of missing and unopenable files

| File | Missing | Exists but cannot be opened |
|:---|:---|:---|
| Library file | error `FileNotExists` (required) | error `OpenFileError` |
| System | skipped | error `OpenFileError` |
| User | skipped | error `OpenFileError` |
| Explicit (if specified) | error `FileNotExists` | error `OpenFileError` |

## CONFIGURATIONS AND INHERITANCE

### Choosing the active configuration

```bash
./program --spla-config=gpu1     # flag
SPLA_CONFIG=gpu1 ./program       # or environment variable
```

If the name is not specified, the `"default"` configuration from the file shipped with the library is used.

### Merging files

Configurations from the library, system, user and explicit files are merged by name: configurations with the same name are merged parameter by parameter, unique ones are added. Parameters of a configuration from a file with higher priority override the parameters of the same-named configuration from a file with lower priority.

### Inheritance via `extends`

If a configuration contains an `extends` field, the library first applies the configurations listed in it in order: the last one in the list overrides the previous ones. The parameters of the configuration itself are applied last and override all parents.

If a configuration does not contain `extends`, only its own parameters are applied.

Names in `extends` are taken from all configurations defined in all files. This allows inheriting configurations defined at another level, for example, a user configuration can inherit `"default"` from the library file.

Circular inheritance is detected and results in the `ConfigurationCycle` status.

Example of a cycle - configurations inherit each other directly:

```json
{
    "a": { "extends": ["b"], "backend": "opencl_gpu" },
    "b": { "extends": ["a"], "backend": "cpu" }
}
```

Such a configuration is rejected when loading (`ConfigurationCycle`). A self-reference is also a cycle (`"extends": ["a"]` inside configuration `a`). Diamond inheritance (`a` → `b`, `a` → `c`, where `b` and `c` both inherit `d`) is not a cycle.

### Example

File shipped with the library:

```json
{
    "default": {
        "backend": "opencl_gpu",
        "if_opencl_unavailable": "use_cpu",
        "queues_count": 1,
        "profiling": false,
        "allocator_type": "general",
        "verbosity": 2
    }
}
```

User file:

```json
{
    "gpu0": {
        "extends": ["default"],
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 0,
        "if_opencl_unavailable": "abort",
        "queues_count": 1,
        "profiling": true
    }
}
```

Final set of parameters of the `gpu0` configuration:

| Parameter | Value | Source |
|:---|:---|:---|
| backend | `by_index` | configuration `gpu0` |
| platform_index | `0` | configuration `gpu0` |
| device_index | `0` | configuration `gpu0` |
| if_opencl_unavailable | `abort` | configuration `gpu0` |
| queues_count | `1` | library file (`default`) |
| profiling | `true` | configuration `gpu0` |
| allocator_type | `general` | library file (`default`) |
| verbosity | `2` | library file (`default`) |

## PARAMETER VALIDATION

After assembly, the configuration from all sources is validated.

### Errors (the configuration is invalid)

| Parameter | Requirement |
|:---|:---|
| `backend` | Required, one of: `opencl_gpu`, `cpu`, `by_index` |
| `platform_index`, `device_index` | Required when `backend=by_index`, must be &ge; 0 |
| `if_opencl_unavailable` | Required when `backend != cpu`, one of: `use_cpu`, `abort` |
| `queues_count` | Required, &ge; 1 |
| `profiling` | Required |
| `allocator_type` | Required, one of: `general`, `linear` |
| `linear_alloc_size` | Required when `allocator_type=linear` |
| `verbosity` | Required, 0..3 |

### Warnings (the parameter is ignored, but the configuration is valid)

| Situation | Message |
|:---|:---|
| `platform_index`/`device_index` set when `backend != by_index` | the parameter is ignored |
| `if_opencl_unavailable` set when `backend=cpu` | the parameter is ignored |
| `linear_alloc_size` set when `allocator_type=general` | the parameter is ignored |

### Behavior when the device is unavailable

Implemented in accelerator initialization:

| Configuration | OpenCL available | OpenCL unavailable |
|:---|:---|:---|
| `backend=opencl_gpu`, `if_opencl_unavailable=use_cpu` | the GPU is used | computations fall back to the pure native CPU; OpenCL is not used |
| `backend=opencl_gpu`, `if_opencl_unavailable=abort` | the GPU is used | error `DeviceNotFound`; the process exits with that status (through `pyspla`, `SplaDeviceNotFound` is raised) |
| `backend=by_index` | the device with the given indexes is used | the platform/device is selected by indexes; if it is unavailable, `if_opencl_unavailable` decides |
| `backend=cpu` | - | pure native CPU is used; OpenCL is never initialized |

## STATUSES AND ERROR BEHAVIOR

The configuration mechanism returns one of the following statuses.

| Status | Meaning |
|:---|:---|
| `Ok` | The configuration is assembled and valid |
| `HelpRequested` | Help requested (`--spla-help`) - the text has already been printed |
| `VersionRequested` | Version requested (`--spla-version`) - already printed |
| `CliOrEnvParseError` | Error parsing command line arguments or environment variables |
| `FileNotExists` | A required file is missing (the library file or an explicitly specified one) |
| `OpenFileError` | The file exists but cannot be opened |
| `ParseConfError` | JSON parsing error, invalid value type, or `"default"` in a non-library file |
| `ConfigurationNotFound` | The specified configuration name was not found in any file |
| `ConfigurationCycle` | A cycle in `extends` was detected |
| `MissedParameters` | Required parameters are missing |
| `PlatformNotFound` / `DeviceNotFound` | The device required by the configuration is unavailable |
| `InvalidConfigParams` | Invalid parameter value |

### Python (`pyspla`)

The same statuses are propagated to Python:

- `configure()` and `init_library()` raise the matching `Spla*` exception
  (`SplaInvalidArgument`, `SplaPlatformNotFound`, `SplaDeviceNotFound`, ...)
  or `SplaFailedConfigure` / `SplaFailedInitialize` for generic errors;
- every raised exception keeps the numeric status in the `status` attribute;
- `acceleration_status()` returns the current accelerator status
  (`0` - accelerator is used, `2` - pure CPU, other values - initialization error);
- `has_acceleration()` returns `True` when an accelerated backend is active.

### CLI behavior

The `spla` command line tool:

- `spla --softcheck` - environment sanity check (configuration, OpenCL initialization, task 2 + 3),
- `spla --spla-help` - help, exit code 0,
- `spla --spla-version` - library version (for example, `SPLA version: 0.0.5`), exit code 0,
- on a configuration error - an error message and exit code 1.