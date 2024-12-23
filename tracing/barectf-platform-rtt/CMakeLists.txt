cmake_minimum_required(VERSION 3.5)

add_library(
    barectf_platform_rtt
    barectf_platform_rtt.c)

target_include_directories(
    barectf_platform_rtt
    PUBLIC
    include
    # For critical section impl
    ${stm32cubef4_SOURCE_DIR}/Drivers/CMSIS/Include
    ${stm32cubef4_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
    PRIVATE
    ${SYSTEM_TRACE_CONFIG_PATH})

target_link_libraries(
    barectf_platform_rtt
    rtt)
