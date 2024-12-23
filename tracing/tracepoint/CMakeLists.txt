cmake_minimum_required(VERSION 3.5)

add_library(
    tracepoint
    INTERFACE)

target_include_directories(
    tracepoint
    INTERFACE
    include)
