# Copyright 2023 Lawrence Livermore National Security, LLC and other
# libjustify Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

set(SOURCES cprintf.c)

add_library(cprintf SHARED ${SOURCES})

target_link_libraries(cprintf PUBLIC ${CMAKE_DL_LIBS})

target_include_directories(cprintf PUBLIC ${CMAKE_SOURCE_DIR}/include)

set_target_properties(cprintf PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY ../libjustify/)
