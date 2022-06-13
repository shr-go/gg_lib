cmake_minimum_required(VERSION 3.22)
project(gg_lib)

set(CMAKE_CXX_STANDARD 11)
include_directories(${CMAKE_SOURCE_DIR}/third_party)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(gg_lib)
add_subdirectory(test)
