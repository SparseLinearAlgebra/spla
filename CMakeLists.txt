cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(spla LANGUAGES CXX)

######################################################################
## Public options

set(SPLA_BUILD_TESTS YES CACHE BOOL "Build test folder with modules tests")
set(SPLA_BUILD_EXAMPLES YES CACHE BOOL "Build library example applications with algorithms")
set(SPLA_BUILD_BACKEND "reference" CACHE STRING "Backend name to build")

######################################################################
## Internal definitions

set(SPLA_DEFINES)
set(SPLA_DEFINES_DEBUG)
set(SPLA_DEFINES_RELEASE)
set(SPLA_DEBUG NO)
set(SPLA_RELEASE NO)
set(SPLA_TARGET_LINUX NO)
set(SPLA_TARGET_WINDOWS NO)
set(SPLA_TARGET_MACOSX NO)
set(SPLA_ARCH)

####################################################################
## Define platform
##  - WINDOWS = Windows Desktop
##  - MACOSX  = MacOS X
##  - LINUX   = Linux

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SPLA_TARGET_WINDOWS YES)
    set(SPLA_ARCH "x64")
    list(APPEND SPLA_DEFINES SPLA_TARGET_WINDOWS)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SPLA_TARGET_LINUX YES)
    set(SPLA_ARCH "x64")
    list(APPEND SPLA_DEFINES SPLA_TARGET_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SPLA_TARGET_MACOSX YES)
    if (NOT SPLA_ARCH)
        if (NOT CMAKE_OSX_ARCHITECTURES)
            if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
                set(SPLA_ARCH "arm64")
            else ()
                set(SPLA_ARCH "x64")
            endif ()
        else ()
            if ("${CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64")
                set(SPLA_ARCH "arm64")
            else ()
                set(SPLA_ARCH "x64")
            endif ()
        endif ()
    endif ()
    message(STATUS "Build macos binaries for ${SPLA_ARCH} architecture")
    list(APPEND SPLA_DEFINES SPLA_TARGET_MACOSX)
else ()
    message(FATAL_ERROR "Unsupported target platform")
endif ()

####################################################################
## Define Build type

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(SPLA_DEBUG YES)
    list(APPEND SPLA_DEFINES SPLA_DEBUG)
    message(STATUS "Build spla in debug mode (specified)")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    set(SPLA_RELEASE YES)
    list(APPEND SPLA_DEFINES SPLA_RELEASE)
    message(STATUS "Build spla in release mode (specified)")
elseif (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(SPLA_RELEASE YES)
    list(APPEND SPLA_DEFINES SPLA_RELEASE)
    list(APPEND SPLA_DEFINES SPLA_DEBUG_RELEASE)
    message(STATUS "Build spla in release mode with debug info (specified)")
else ()
    set(SPLA_RELEASE YES)
    set(CMAKE_BUILD_TYPE Release)
    list(APPEND SPLA_DEFINES SPLA_RELEASE)
    message(STATUS "Build spla in release mode (default, not specified)")
endif ()

##################################################################
## Compiler and language specifics

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (SPLA_TARGET_WINDOWS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        list(APPEND SPLA_DEFINES SPLA_MSVC)
    else ()
        message(FATAL_ERROR "Supported only MSVC compiler for Windows platform")
    endif ()
endif ()

##################################################################
## Target options and defs functions
## (use this functions to configure build targets)

function(spla_target_compile_definitions target)
    foreach (DEFINITION ${SPLA_DEFINES})
        target_compile_definitions(${target} INTERFACE ${DEFINITION})
    endforeach ()

    if (SPLA_DEBUG)
        foreach (DEFINITION ${SPLA_DEFINES_DEBUG})
            target_compile_definitions(${target} INTERFACE ${DEFINITION})
        endforeach ()
    endif ()

    if (SPLA_RELEASE)
        foreach (DEFINITION ${SPLA_DEFINES_RELEASE})
            target_compile_definitions(${target} INTERFACE ${DEFINITION})
        endforeach ()
    endif ()

    if (SPLA_TARGET_WINDOWS)
        target_compile_definitions(${target}
                INTERFACE WIN32
                INTERFACE _WIN32
                INTERFACE _WINDOWS
                INTERFACE UNICODE
                INTERFACE _UNICODE
                INTERFACE _CRT_SECURE_NO_WARNINGS
                INTERFACE _SCL_SECURE_NO_WARNINGS)
    endif ()
endfunction()

function(spla_target_compile_options target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS_RELEASE "/MT")
        set(CMAKE_CXX_FLAGS_DEBUG "/MTd")

        target_compile_options(${target}
                INTERFACE /W1
                INTERFACE /std:c++17
                INTERFACE /source-charset:utf-8)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target}
                INTERFACE -Wall
                INTERFACE -Wextra
                INTERFACE -Wpedantic)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target}
                INTERFACE -Wall
                INTERFACE -Wextra
                INTERFACE -Wpedantic
                INTERFACE -Wno-inconsistent-missing-override)
    endif ()
endfunction()

function(spla_target_link_options target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_options(${target}
                INTERFACE /INCREMENTAL:NO
                INTERFACE /machine:X64)
    endif ()
endfunction()

######################################################################
## Dependencies config

message(STATUS "Add OpenCL package required header files")
find_package(OpenCL REQUIRED)

message(STATUS "Add Taskflow library")
set(TF_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(TF_BUILD_CUDA OFF CACHE BOOL "" FORCE)
set(TF_BUILD_SYCL OFF CACHE BOOL "" FORCE)
set(TF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TF_BUILD_PROFILER OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/taskflow)

if (SPLA_BUILD_EXAMPLES)
    message(STATUS "Add cxxopts as arguments parser for example applications")
    set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(CXXOPTS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(CXXOPTS_ENABLE_WARNINGS OFF CACHE BOOL "" FORCE)
    set(CXXOPTS_USE_UNICODE_HELP OFF CACHE BOOL "" FORCE)
    add_subdirectory(deps/cxxopts)
endif ()

if (SPLA_BUILD_TESTS)
    message(STATUS "Add Google test as unit test framework")
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
    set(gtest_hide_internal_symbols ON CACHE BOOL "" FORCE)
    add_subdirectory(deps/gtest)
endif ()

######################################################################
## Source code configuration

if (SPLA_BUILD_BACKEND STREQUAL "cuda")
    message(STATUS "Add library CUDA backend")
    list(APPEND SPLA_DEFINES SPLA_BACKEND_CUDA)
endif ()

if (SPLA_BUILD_BACKEND STREQUAL "opencl")
    message(STATUS "Add library OpenCL backend")
    list(APPEND SPLA_DEFINES SPLA_BACKEND_OPENCL)
endif ()

if (SPLA_BUILD_BACKEND STREQUAL "reference")
    message(STATUS "Add library reference (CPU) backend")
    list(APPEND SPLA_DEFINES SPLA_BACKEND_REFERENCE)
endif ()

add_library(spla INTERFACE)

target_include_directories(spla INTERFACE "${CMAKE_CURRENT_LIST_DIR}/.")
target_include_directories(spla INTERFACE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(spla INTERFACE ${OpenCL_LIBRARIES})
target_link_libraries(spla INTERFACE Taskflow)

target_compile_definitions(spla INTERFACE CL_TARGET_OPENCL_VERSION=120)

spla_target_compile_definitions(spla)
spla_target_compile_options(spla)
spla_target_link_options(spla)

######################################################################
## Add examples directory

if (SPLA_BUILD_EXAMPLES)
    message(STATUS "Add examples to build")

    function(spla_example_application target)
        message(STATUS "Add example application ${target}")
        add_executable(${target} examples/${target}.cpp)
        target_link_libraries(${target} PRIVATE spla)
        target_link_libraries(${target} PRIVATE cxxopts)
    endfunction()

    spla_example_application(bfs)
endif ()

######################################################################
## Add unit-tests directory

if (SPLA_BUILD_TESTS)
    message(STATUS "Add tests directory")
    add_subdirectory(tests)
endif ()