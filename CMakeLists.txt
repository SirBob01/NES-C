cmake_minimum_required(VERSION 3.24)
project(.)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED on)

add_compile_options(-Wall -g -O3 -fPIC)

option(SDL_TEST "" OFF)
add_subdirectory("submodules/SDL")

file(GLOB_RECURSE INCLUDE "./src/*.h")
file(GLOB_RECURSE SOURCES "./src/*.c")

add_executable(nesc ${SOURCES})
target_link_libraries(nesc PRIVATE SDL3::SDL3-shared)
