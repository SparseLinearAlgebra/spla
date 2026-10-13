# Comparison of libraries for creating a configuration mechanism
All information is taken from the repositories of the corresponding projects. Access date: 21.04.26.

## Comparison criteria

| Criterion | Metric | Units |
|:---|:---|:---|
| **Dependencies** | presence | +/- |
| **Sources** | CLI (argv) / env / files | +/-, formats |
| **C++ standard** | minimum version | C++xx |
| **Supports standard hierarchy** | Linux/Windows/MacOS | +/- |
| **Distribution** | header-only / static / dynamic | - |
| **OS** | Linux / Windows / macOS | +/-, compilers |
| **Activity** | average number of commits | commits/month (last month) |
| **Issues** | open, closed/open | n, ratio |
| **Community** | GitHub Stars | n |
| **Passes CI** | CI | +/- |

> ? - not specified in the README.


## Comparison of command line parsers (CLI)

| Criterion | CLI11 | cxxopts |
|:---|:---:|:---:|
| Dependencies | - | - |
| **Sources** | | |
| &emsp;CLI (argv) | + | + |
| &emsp;env | + | - |
| &emsp;Files | - | - |
| C++ standard | &ge; C++11 | &ge; C++11 |
| **Supports standard hierarchy** | | |
| &emsp;Linux | + | + |
| &emsp;Windows | + | + |
| &emsp;MacOS | + | + |
| Distribution | header-only | header-only
| **OS** | | |
| &emsp;Linux | + (GCC 4.8+, Clang 3.5+) | + (GCC &ge; 4.9, Clang &ge; 3.1) |
| &emsp;Windows | + (MSVC &ge; 2015) | + (MSVC &ge; 2015) |
| &emsp;macOS | + (AppleClang 7+) | + (Clang &ge; 3.1 with libc++) |
| **Community** | | |
| &emsp;Activity (commits/month) | 4 | 1 |
| &emsp;Total issues | 506 | 307 |
| &emsp;Issues closed/open | 9 | 6 |
| &emsp;Stars | 4.3k | 4.7k |
| Passes CI | + | + | + |


## File parsers

| Criterion | nlohmann/json  | yaml-cpp | toml++ | toml11 | config-cxx | taocpp/config
|:---|:---:|:---:|:---:|:---:|:---:|:---:|
| Dependencies | - | - | - | - | - | - |
| **Sources** |
| &emsp;CLI (argv) | - | - | - | - | - | - |
| &emsp;env | - | - | - | - | + | + |
| &emsp;Files | JSON | YAML | TOML | TOML | JSON, YAML, XML | JSON, JAXN |
| C++ standard | &ge; C++11 | &ge; C++11 | &ge; C++17 | &ge; C++11 | &ge; C++20 | &ge; C++17 |
| **Supports standard hierarchy** |
| &emsp;Linux | + | + | + | + | + | + |
| &emsp;Windows | + | + | + | + | + | + |
| &emsp;MacOS | + | + | + | + | + | + |
| Distribution | header-only | static/dynamic¹ | header-only/static/dynamic¹ | header-only/static/dynamic¹ | header-only | header-only |
| **OS** | | |
| &emsp;Linux | + (GCC 4.8–14.2, Clang 3.4–21.0) | + (GCC, Clang) | + (Clang 8+, GCC 8+) | + (GCC, Clang) | + (GCC 13+, Clang 16+) | +
| &emsp;Windows | + (MSVC 2015–2022) | + (MSVC) | + (MSVC VS2019+) | + (MSVC, MinGW) | + (MSVC 143+ (VS 2022)) | +
| &emsp;macOS | + (AppleClang 9.1–16.0) | + (Xcode, AppleClang) | + (AppleClang) | + (AppleClang) | + (AppleClang 16+) | +
| **Community** | | |
| &emsp;Activity (commits/month) | 2 | 9 | 2 | 4 | 2 | 13 |
| &emsp;Total issues | 3271 | 905 | 193 | 194 | 24 | 5 |
| &emsp;Issues closed/open | 68 | 3 | 8 | 5 | 0 open | 0 open |
| &emsp;Stars | 49.4k | 15k | 2k | 1.3k | 31 | 194 |
| Passes CI | + | + | + | - | + | ? |

> ¹ Optional.

## Comparison of universal tools

| Criterion | Boost.Program_options
|:---|:---:|
| Dependencies | +¹ |
| **Sources** | | |
| &emsp;CLI (argv) | + | + |
| &emsp;env | + | + |
| &emsp;Files | INI |
| C++ standard | &ge; C++3 |
| **Supports standard hierarchy** | | |
| &emsp;Linux | + |
| &emsp;Windows | + |
| &emsp;MacOS | + |
| Distribution | static/dynamic² |
| **OS** | | |
| &emsp;Linux | + (GCC 5+, Clang 3.6+) |
| &emsp;Windows | + (MSVC 2015 (vc140)+) |
| &emsp;macOS | + (AppleClang) |
| **Community** | | |
| &emsp;Activity (commits/month) | 12 |
| &emsp;Total issues | 400 |
| &emsp;Issues closed/open | 1 |
| &emsp;Stars | 9.4k |
| Passes CI | + | + |

> ¹ Dependency list:
> 1. Boost.Any
> 2. Boost.Bind
> 3. Boost.Config
> 4. Boost.Core
> 5. Boost.Detail
> 6. Boost.Function
> 7. Boost.Iterator
> 8. Boost.Lexical Cast
> 9. Boost.Smart Ptr
> 10. Boost.ThrowException
> 11. Boost.Tokenizer
> 12. Boost.Type Traits

> ² Optional.


### Conclusion:
Based on the comparison results, the best choice is likely to use `CLI11` and `nlohmann/json`, since, unlike `Boost.program_options`, they require no dependencies and are header-only.