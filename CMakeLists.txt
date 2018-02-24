cmake_minimum_required(VERSION 3.0)
enable_testing()

if(NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "_d")
endif()

project(ffmt)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(ffmt STATIC
    src/ffmt_formatter.c
    src/ffmt_puts_pad.c
    src/ffmt_u64_to_dec.c
    src/ffmt_u64_to_hex.c
    src/ffmt_write_to_string.c
    src/ffmt_write.c
    src/ffmt_ez.c
    src/ffmt.c
)

set_target_properties(ffmt PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin/")

set_target_properties(ffmt PROPERTIES C_STANDARD 99)
target_compile_options(ffmt PRIVATE
    -Wall -Wextra -nostdlib -Wmissing-prototypes -Wmissing-declarations -Werror)

function(create_test name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} ffmt)

    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "test/")

    if("${src}" MATCHES "\\.c$")
        set_target_properties(${name} PROPERTIES C_STANDARD 11)
    else()
        set_target_properties(${name} PROPERTIES CXX_STANDARD 14)
    endif()

    add_test(NAME ${name} COMMAND ${name})
endfunction(create_test)

create_test(dechex test/dechex.c)
create_test(format test/format.cxx)
create_test(c_api test/c_api.c)
