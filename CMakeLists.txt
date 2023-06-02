cmake_minimum_required(VERSION 3.24)
project(.)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED on)

add_compile_options(-Wall -g -O3 -fPIC)

file(GLOB_RECURSE INCLUDE "./src/*.h")
file(GLOB_RECURSE SOURCES "./src/*.c")

add_executable(nesc ${SOURCES})
