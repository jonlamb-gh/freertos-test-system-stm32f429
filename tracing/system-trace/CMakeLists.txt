cmake_minimum_required(VERSION 3.5)

option(SYSTEM_TRACE_CONFIG_PATH "Path to directory containing system_trace_config.h")

add_library(
    system_trace
    system_trace.c)

target_include_directories(
    system_trace
    PUBLIC
    include
    ${SYSTEM_TRACE_CONFIG_PATH})

target_link_libraries(
    system_trace
    barectf_platform_rtt)
