cmake_minimum_required(VERSION 3.24)
project(.)
include(CTest)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED on)

add_compile_options(-Wall -g -O3 -fPIC)

option(SDL_TEST "" OFF)
add_subdirectory("../submodules/SDL" "submodules/SDL")

file(GLOB_RECURSE INCLUDE "./src/*.h" "../src/*.h")
file(GLOB_RECURSE SOURCES "../src/*.c")

list(FILTER INCLUDE EXCLUDE REGEX ".*nes\\.h$")
list(FILTER SOURCES EXCLUDE REGEX ".*nes\\.c$")

# Compile all tests
file(GLOB TESTS "./src/*.c")
foreach(test_path ${TESTS})
    get_filename_component(test ${test_path} NAME_WE)
    add_executable(${test} ${test_path} ${INCLUDE} ${SOURCES})
    target_link_libraries(${test} PRIVATE SDL3::SDL3-shared)
    add_test(NAME ${test} COMMAND ${test})
endforeach()