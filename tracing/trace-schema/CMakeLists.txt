cmake_minimum_required(VERSION 3.5)

file(GLOB_RECURSE BARECTF_CONFIG_FILES config/*.yaml)

set(BARECTF_GENERATED_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/generated/include/trace_schema/barectf-bitfield.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/include/trace_schema/barectf.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/barectf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/metadata)

# NOTE: need to be able to pick a system schema (config.yaml)
# Probably have a per-system libary that does this sort of thing or build option
add_custom_command(
    OUTPUT ${BARECTF_GENERATED_FILES}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated/include/trace_schema
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/schema
    COMMAND barectf generate
        --metadata-dir ${CMAKE_CURRENT_SOURCE_DIR}/schema
        --code-dir ${CMAKE_CURRENT_BINARY_DIR}/generated
        --headers-dir ${CMAKE_CURRENT_BINARY_DIR}/generated/include/trace_schema
        --include-dir ${CMAKE_CURRENT_SOURCE_DIR}/config
        --include-dir ${CMAKE_CURRENT_SOURCE_DIR}/config/test-system
        --include-dir ${CMAKE_CURRENT_SOURCE_DIR}/config/test-system/caneth
        --include-dir ${CMAKE_CURRENT_SOURCE_DIR}/config/test-system/led
        ${CMAKE_CURRENT_SOURCE_DIR}/config/test-system/config.yaml
    DEPENDS ${BARECTF_CONFIG_FILES}
    COMMENT "Generating trace-schema barectf files"
    VERBATIM)

add_custom_target(
    trace_schema_barectf_generated_files
    DEPENDS
    ${BARECTF_GENERATED_FILES})

add_library(
    trace_schema
    ${CMAKE_CURRENT_BINARY_DIR}/generated/barectf.c)

target_include_directories(
    trace_schema
    PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/generated/include
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated/include/trace_schema)

target_compile_options(
    trace_schema
    PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wshadow -Wmissing-include-dirs -Wstrict-prototypes
    -Wno-sign-conversion -Wno-unused-function)

add_dependencies(
    trace_schema
    trace_schema_barectf_generated_files)
