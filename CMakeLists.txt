# Copyright 2023 Lawrence Livermore National Security, LLC and other
# libjustify Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

# avoid warnings for project commands w/o VERSION
cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(libjustify VERSION "0.1.0" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS               "Build tests"                            ON)

### Enable warnings
if(ENABLE_WARNINGS)
    ENABLE_WARNINGS()
endif()

# Generate tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(src)
