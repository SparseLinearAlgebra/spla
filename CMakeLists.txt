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

message(STATUS "Add OpenCL package required header files")
find_package(OpenCL REQUIRED)

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

add_library(spla SHARED
        include/spla/library.hpp
        include/spla/matrix.hpp
        include/spla/object.hpp
        include/spla/op.hpp
        include/spla/ref.hpp
        include/spla/schedule.hpp
        include/spla/type.hpp
        src/core/matrix.cpp)

target_include_directories(spla PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(spla PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(spla PUBLIC ${OpenCL_INCLUDE_DIRS})
target_link_libraries(spla PUBLIC ${OpenCL_LIBRARIES})

set_target_properties(spla PROPERTIES OUTPUT_NAME "spla_${SPLA_ARCH}")
target_compile_definitions(spla PRIVATE SPLA_EXPORTS)
target_compile_definitions(spla PRIVATE CL_TARGET_OPENCL_VERSION=120)

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
        set(COMMON_NAME "${CMAKE_BINARY_DIR}/spla_${SPLA_ARCH}")
        set(TARGET_FILES "${COMMON_NAME}.dll")
        message(STATUS "${TARGET_FILES}")

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