# Copyright 2023 Lawrence Livermore National Security, LLC and other
# libjustify Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(libjustify VERSION "0.1.0" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

option(VERBOSE     "Verbose output" OFF)

set(SOURCES cprintf.c)

add_library(cprintf SHARED ${SOURCES})

target_link_libraries(cprintf PUBLIC ${CMAKE_DL_LIBS})

target_include_directories(cprintf PUBLIC ${CMAKE_SOURCE_DIR})

set_target_properties(cprintf PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)

install(TARGETS cprintf
        EXPORT  cprintf
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib)

install(FILES cprintf.h
        DESTINATION include)
