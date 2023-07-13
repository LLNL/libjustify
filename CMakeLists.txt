cmake_minimum_required(VERSION 3.16)
project(MultiplyTest LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate tests
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)