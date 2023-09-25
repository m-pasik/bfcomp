cmake_minimum_required(VERSION 3.10)
project(bfcomp VERSION 0.0.1 DESCRIPTION "Simple brainfuck compiler")
CMAKE_POLICY(SET CMP0072 NEW)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS_DEBUG "-Wall -O3")

set(CMAKE_C_FLAGS_DEBUG "-Wall -O0 -fsanitize=address -ggdb")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/release)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debug)
endif()

file(GLOB_RECURSE SOURCES src/*.c)

include_directories(bfcomp ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(bfcomp ${SOURCES})


