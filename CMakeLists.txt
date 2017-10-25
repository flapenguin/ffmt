cmake_minimum_required(VERSION 2.8)
enable_testing()

project(ffmt)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(ffmt STATIC
    src/u64_to_dec.c
    src/u64_to_hex.c)
target_compile_options(ffmt PRIVATE
    -Wall -Wextra -nostdlib -Wmissing-prototypes -Wmissing-declarations -Werror)

add_executable(dechex test/dechex.c)
set (CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
target_link_libraries(dechex ffmt)
add_test(NAME dechex COMMAND dechex)
