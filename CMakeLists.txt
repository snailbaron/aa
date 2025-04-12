cmake_minimum_required(VERSION 3.12)

project(aa)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(example)
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
