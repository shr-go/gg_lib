cmake_minimum_required(VERSION 3.0)
project(gg_lib)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

option(GG_LIB_TEST "Build gg_lib's test" OFF)

include_directories(${CMAKE_SOURCE_DIR}/third_party)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(gg_lib)
add_subdirectory(example)

if(GG_LIB_TEST)
    add_subdirectory(test)
endif()
