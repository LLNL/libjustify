# Copyright 2023 Lawrence Livermore National Security, LLC and other
# libjustify Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

message(STATUS "Downloading & Configuring Google Test (This may take a minute)...")

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)



add_executable(cprintf_test cprintf_test.cpp)

target_link_libraries(cprintf_test
 PRIVATE
  GTest::GTest
  cprintf)

set_target_properties(cprintf_test PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/gtest/)

add_test(cprintf_gtests cprintf_test)
