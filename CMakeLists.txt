cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(spla LANGUAGES CXX)

######################################################################
## public options

option(SPLA_BUILD_TESTS "Build test folder with modules tests" YES)
option(SPLA_BUILD_EXAMPLES "Build library example applications with algorithms" YES)
option(SPLA_BUILD_OPENCL "Build library with opencl backend" YES)

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
set(SPLA_EXT)

####################################################################
## Define platform
##  - WINDOWS = Windows Desktop
##  - MACOSX  = MacOS X
##  - LINUX   = Linux

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SPLA_TARGET_WINDOWS YES)
    set(SPLA_ARCH "x64")
    set(SPLA_EXT "dll")
    list(APPEND SPLA_DEFINES SPLA_TARGET_WINDOWS)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SPLA_TARGET_LINUX YES)
    set(SPLA_ARCH "x64")
    set(SPLA_EXT "so")
    list(APPEND SPLA_DEFINES SPLA_TARGET_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SPLA_TARGET_MACOSX YES)
    set(SPLA_EXT "dylib")
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
        target_compile_definitions(${target} PUBLIC ${DEFINITION})
    endforeach ()

    if (SPLA_DEBUG)
        foreach (DEFINITION ${SPLA_DEFINES_DEBUG})
            target_compile_definitions(${target} PUBLIC ${DEFINITION})
        endforeach ()
    endif ()

    if (SPLA_RELEASE)
        foreach (DEFINITION ${SPLA_DEFINES_RELEASE})
            target_compile_definitions(${target} PUBLIC ${DEFINITION})
        endforeach ()
    endif ()

    if (SPLA_TARGET_WINDOWS)
        target_compile_definitions(${target}
                PUBLIC WIN32
                PUBLIC _WIN32
                PUBLIC _WINDOWS
                PUBLIC UNICODE
                PUBLIC _UNICODE
                PUBLIC _CRT_SECURE_NO_WARNINGS
                PUBLIC _SCL_SECURE_NO_WARNINGS)
    endif ()
endfunction()

function(spla_target_compile_options target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS_RELEASE "/MT")
        set(CMAKE_CXX_FLAGS_DEBUG "/MTd")

        target_compile_options(${target}
                PUBLIC /W1
                PUBLIC /std:c++17
                PUBLIC /source-charset:utf-8)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target}
                PUBLIC -Wall
                PUBLIC -Wextra
                PUBLIC -Wpedantic)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target}
                PUBLIC -Wall
                PUBLIC -Wextra
                PUBLIC -Wpedantic
                PUBLIC -Wno-inconsistent-missing-override)
    endif ()
endfunction()

function(spla_target_link_options target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_options(${target}
                PUBLIC /INCREMENTAL:NO
                PUBLIC /machine:X64)
    endif ()
endfunction()

######################################################################
## Dependencies config

add_subdirectory(deps/robin_hood)
add_subdirectory(deps/svector)

if (SPLA_BUILD_OPENCL)
    if (SPLA_TARGET_MACOSX)
        message(STATUS "Add standard Apple OpenCL package")
        find_package(OpenCL REQUIRED)

        add_library(OpenCLHeaders INTERFACE)
        add_library(OpenCL::Headers ALIAS OpenCLHeaders)
        target_include_directories(OpenCLHeaders INTERFACE ${OpenCL_INCLUDE_DIRS})

        add_library(OpenCL INTERFACE)
        target_include_directories(OpenCL INTERFACE ${OpenCL_INCLUDE_DIRS})
        target_link_libraries(OpenCL INTERFACE ${OpenCL_LIBRARIES})
    else ()
        message(STATUS "Add Khronos OpenCL C API header files")
        add_subdirectory(deps/opencl-headers)

        message(STATUS "Add Khronos OpenCL ICD loaded library")
        set(OPENCL_ICD_LOADER_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(OPENCL_ICD_LOADER_BUILD_TESTING OFF CACHE BOOL "" FORCE)
        add_subdirectory(deps/opencl-icd-loader)
    endif ()


    message(STATUS "Add Khronos OpenCL HPP bindings")
    add_library(HeadersCpp INTERFACE)
    add_library(OpenCL::HeadersCpp ALIAS HeadersCpp)
    target_include_directories(HeadersCpp INTERFACE deps/opencl-headers-hpp/include)
    target_link_libraries(HeadersCpp INTERFACE OpenCL::Headers)
endif ()

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

set(SRC_OPENCL)

if (SPLA_BUILD_OPENCL)
    set(SRC_OPENCL
            src/opencl/cl_debug.hpp
            src/opencl/cl_accelerator.cpp
            src/opencl/cl_accelerator.hpp
            src/opencl/cl_algo_registry.cpp
            src/opencl/cl_algo_registry.hpp
            src/opencl/cl_program.cpp
            src/opencl/cl_program.hpp
            src/opencl/cl_program_builder.cpp
            src/opencl/cl_program_builder.hpp
            src/opencl/cl_program_cache.cpp
            src/opencl/cl_program_cache.hpp
            src/opencl/cl_format_dense_vec.hpp
            src/opencl/cl_format_coo_vec.hpp
            src/opencl/cl_format_csr.hpp
            src/opencl/cl_formats.hpp
            src/opencl/cl_mxv.hpp
            src/opencl/cl_vxm.hpp
            src/opencl/cl_vector_assign.hpp
            src/opencl/cl_vector_count_nz.hpp
            src/opencl/cl_vector_reduce.hpp
            src/opencl/cl_fill.hpp
            src/opencl/cl_prefix_sum.hpp
            src/opencl/cl_reduce.hpp
            src/opencl/cl_reduce_by_key.hpp
            src/opencl/cl_sort_by_key.hpp
            )
endif ()

add_library(spla SHARED
        # C++ core
        include/spla.hpp
        include/spla/algorithm.hpp
        include/spla/config.hpp
        include/spla/descriptor.hpp
        include/spla/exec.hpp
        include/spla/io.hpp
        include/spla/library.hpp
        include/spla/matrix.hpp
        include/spla/object.hpp
        include/spla/op.hpp
        include/spla/ref.hpp
        include/spla/scalar.hpp
        include/spla/schedule.hpp
        include/spla/timer.hpp
        include/spla/type.hpp
        include/spla/vector.hpp
        src/core/accelerator.hpp
        src/core/common.hpp
        src/core/dispatcher.cpp
        src/core/dispatcher.hpp
        src/core/logger.cpp
        src/core/logger.hpp
        src/core/registry.cpp
        src/core/registry.hpp
        src/core/tdecoration.hpp
        src/core/tmatrix.hpp
        src/core/top.hpp
        src/core/tscalar.hpp
        src/core/ttype.hpp
        src/core/tvector.hpp
        src/storage/storage_manager.hpp
        src/storage/storage_manager_matrix.hpp
        src/storage/storage_manager_vector.hpp
        src/schedule/schedule_tasks.cpp
        src/schedule/schedule_tasks.hpp
        src/schedule/schedule_st.cpp
        src/schedule/schedule_st.hpp
        src/cpu/cpu_algo_callback.hpp
        src/cpu/cpu_algo_registry.cpp
        src/cpu/cpu_algo_registry.hpp
        src/cpu/cpu_format_coo.hpp
        src/cpu/cpu_format_coo_vec.hpp
        src/cpu/cpu_format_csr.hpp
        src/cpu/cpu_format_dense_vec.hpp
        src/cpu/cpu_format_dok.hpp
        src/cpu/cpu_format_dok_vec.hpp
        src/cpu/cpu_format_lil.hpp
        src/cpu/cpu_formats.hpp
        src/cpu/cpu_mxv.hpp
        src/cpu/cpu_vxm.hpp
        src/cpu/cpu_vector_assign.hpp
        src/cpu/cpu_vector_count_nz.hpp
        src/cpu/cpu_vector_reduce.hpp
        src/util/pair_hash.hpp
        src/profiling/time_profiler.cpp
        src/profiling/time_profiler.hpp
        src/descriptor.cpp
        src/io.cpp
        src/exec.cpp
        src/library.cpp
        src/matrix.cpp
        src/algorithm.cpp
        src/op.cpp
        src/scalar.cpp
        src/schedule.cpp
        src/timer.cpp
        src/type.cpp
        src/vector.cpp
        # C bindings
        include/spla.h
        src/binding/c_config.hpp
        src/binding/c_library.cpp
        # C++ optional part
        ${SRC_OPENCL})

target_include_directories(spla PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(spla PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

set_target_properties(spla PROPERTIES OUTPUT_NAME "spla_${SPLA_ARCH}")
target_compile_definitions(spla PRIVATE SPLA_EXPORTS)

target_link_libraries(spla PRIVATE robin_hood)
target_link_libraries(spla PRIVATE svector)

if (SPLA_BUILD_OPENCL)
    target_link_libraries(spla PUBLIC OpenCL)
    target_link_libraries(spla PUBLIC OpenCL::HeadersCpp)
    target_compile_definitions(spla PUBLIC CL_TARGET_OPENCL_VERSION=120)
    list(APPEND SPLA_DEFINES SPLA_BUILD_OPENCL)
endif ()

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

    if (SPLA_TARGET_WINDOWS)
        set(SPLA_LIB_NAME "${CMAKE_BINARY_DIR}/spla_${SPLA_ARCH}.${SPLA_EXT}")
        set(TARGET_FILES ${SPLA_LIB_NAME})

        foreach (TARGET_FILE ${TARGET_FILES})
            add_custom_command(
                    TARGET spla POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E
                    copy
                    ${TARGET_FILE}
                    "${CMAKE_BINARY_DIR}/tests"
                    COMMENT "Copy ${TARGET_FILE} into test directory")
        endforeach ()
    endif ()
endif ()