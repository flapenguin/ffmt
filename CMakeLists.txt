cmake_minimum_required(VERSION 3.9)
enable_testing()

if(NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "_d")
endif()

project(ffmt)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(ffmt_sources
    src/ffmt_formatter.c
    src/ffmt_puts_pad.c
    src/ffmt_u64_to_dec.c
    src/ffmt_u64_to_hex.c
    src/ffmt_write_to_string.c
    src/ffmt_write.c
    src/ffmt.c
)

set(ffmt_compile_options
    -nostdlib
    -Wall
    -Wextra
    -Wmissing-prototypes
    -Wmissing-declarations
    -Werror
)

add_library(ffmt_static STATIC ${ffmt_sources})
add_library(ffmt_dynamic SHARED ${ffmt_sources})

set_target_properties(ffmt_static ffmt_dynamic PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "bin/"
    ARCHIVE_OUTPUT_DIRECTORY "bin/"
    C_STANDARD 99
    OUTPUT_NAME "ffmt"
)

set_target_properties(ffmt_dynamic PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    INTERPROCEDURAL_OPTIMIZATION TRUE
    LINK_FLAGS "-Wl,--version-script,${PROJECT_SOURCE_DIR}/src/ffmt.map -nostdlib"
)

target_compile_options(ffmt_static PRIVATE ${ffmt_compile_options})
target_compile_options(ffmt_dynamic PRIVATE ${ffmt_compile_options})

function(create_test type name src)
    set(test_name test_${type}_${name})
    add_executable(${test_name} ${src})

    target_link_libraries(${test_name} ffmt_${type})
    set_property(TARGET ${test_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY "test/")

    if("${src}" MATCHES "\\.c$")
        set_property(TARGET ${test_name} PROPERTY C_STANDARD 11)
    else()
        set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 14)
    endif()

    add_test(NAME "${name}-${type}" COMMAND test/${test_name})
endfunction(create_test)

create_test(static dechex test/dechex.c)
create_test(dynamic dechex test/dechex.c)
create_test(static format test/format.cxx)
create_test(dynamic format test/format.cxx)
create_test(static c_api test/c_api.c)
create_test(dynamic c_api test/c_api.c)
