cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(spla LANGUAGES CXX)

######################################################################
## Public options

option(SPLA_BUILD_TESTS "Build test folder with modules tests" YES)

######################################################################
## internal definitions

set(SPLA_DEFINES)
set(SPLA_DEBUG NO)
set(SPLA_RELEASE NO)
set(SPLA_TARGET_LINUX NO)
set(SPLA_TARGET_WINDOWS NO)
set(SPLA_ARCH x64)

####################################################################
## Define platform
##  - WINDOWS = Windows Desktop
##  - MACOSX  = MacOS X
##  - LINUX   = Linux

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SPLA_TARGET_WINDOWS YES)
    list(APPEND SPLA_DEFINES SPLA_TARGET_WINDOWS)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SPLA_TARGET_LINUX YES)
    list(APPEND SPLA_DEFINES SPLA_TARGET_LINUX)
else()
    message(FATAL_ERROR "Unsupported target platform")
endif()

####################################################################
## Define Build type

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SPLA_DEBUG YES)
    list(APPEND SPLA_DEFINES SPLA_DEBUG)
    message(STATUS "Build spla in debug mode (specified)")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(SPLA_RELEASE YES)
    list(APPEND SPLA_DEFINES SPLA_RELEASE)
    message(STATUS "Build spla in release mode (specified)")
else()
    set(SPLA_RELEASE YES)
    list(APPEND SPLA_DEFINES SPLA_RELEASE)
    message(STATUS "Build spla in release mode (default, not specified)")
endif()

##################################################################
## Compiler and language specifics

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (SPLA_TARGET_WINDOWS)
    if (MSVC)
        list(APPEND SPLA_DEFINES SPLA_MSVC)
    else()
        message(FATAL_ERROR "Supported only MSVC compiler for Windows platform")
    endif()
endif()

##################################################################
## Target options and defs functions
## (use this functions to configure build targets)

function(spla_target_compile_definitions target)
    foreach(DEFINITION ${SPLA_DEFINES})
        target_compile_definitions(${target} PUBLIC ${DEFINITION})
    endforeach()

    if (SPLA_TARGET_WINDOWS)
        target_compile_definitions(${target}
                PUBLIC WIN32
                PUBLIC _WIN32
                PUBLIC _WINDOWS
                PUBLIC UNICODE
                PUBLIC _UNICODE
                PUBLIC _CRT_SECURE_NO_WARNINGS
                PUBLIC _SCL_SECURE_NO_WARNINGS)
    endif()
endfunction()

function(spla_target_compile_options target)
    if (MSVC)
        target_compile_options(${target}
                PUBLIC /source-charset:utf-8
                PUBLIC $<$<CONFIG:Debug>: /MDd>
                PUBLIC $<$<CONFIG:Release>: /MD>)
    endif()
endfunction()

function(spla_target_link_options target)
    if (MSVC)
        target_link_options(${target}
                PRIVATE /INCREMENTAL:NO
                PRIVATE /NODEFAULTLIB:MSVCRT
                PRIVATE /machine:X64)
    endif()
endfunction()

######################################################################
## Dependencies config

if (SPLA_BUILD_TESTS)
    message(STATUS "Add Google test as unit test framework")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(gtest_hide_internal_symbols ON CACHE BOOL "" FORCE)
    add_subdirectory(deps/gtest)
endif()

######################################################################
## Source code configuration

include(include/CMakeLists.txt)
include(sources/CMakeLists.txt)

add_library(spla SHARED
        ${SPLA_CPP_HEADERS}
        ${SPLA_CPP_SOURCES})

set_target_properties(spla PROPERTIES OUTPUT_NAME "spla_${SPLA_ARCH}")
target_include_directories(spla PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

spla_target_compile_definitions(spla)
spla_target_compile_options(spla)
spla_target_link_options(spla)

######################################################################
## Add unit-tests directory

if (SPLA_BUILD_TESTS)
    message(STATUS "Add tests directory")
    add_subdirectory(tests)
endif()