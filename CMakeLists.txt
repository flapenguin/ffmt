cmake_minimum_required(VERSION 2.8)
enable_testing()

project(ffmt)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(ffmt src/ffmt.c)

add_executable(smoke test/smoke.c)
target_link_libraries(smoke ffmt)
add_test(NAME "smoke" COMMAND smoke)
