cmake_minimum_required(VERSION 3.5)

project(firmware LANGUAGES C ASM)

include(FetchContent)

message(STATUS "Fetching dependencies")

FetchContent_Declare(
    freertos
    GIT_REPOSITORY "https://github.com/FreeRTOS/FreeRTOS-Kernel.git"
    GIT_TAG "V10.5.0"
)

FetchContent_Declare(
    tracerecorder
    GIT_REPOSITORY "https://github.com/percepio/TraceRecorderSource.git"
    GIT_TAG "Tz4/4.8/v4.8.2"
)

FetchContent_Declare(
    stm32cubef4
    GIT_REPOSITORY "https://github.com/STMicroelectronics/STM32CubeF4.git"
    GIT_TAG "v1.27.1"
)

# Use FetchContent_Populate instead of FetchContent_MakeAvailable because
# we're defining build rules locally and don't want cmake to automatically
# call add_subdirectory on these
FetchContent_Populate(freertos)
FetchContent_Populate(tracerecorder)
FetchContent_Populate(stm32cubef4)

set(
    KERNEL_SOURCES
    ${freertos_SOURCE_DIR}/portable/GCC/ARM_CM4F/port.c
    ${freertos_SOURCE_DIR}/portable/MemMang/heap_4.c
    ${freertos_SOURCE_DIR}/queue.c
    ${freertos_SOURCE_DIR}/tasks.c
    ${freertos_SOURCE_DIR}/timers.c
    ${freertos_SOURCE_DIR}/event_groups.c
    ${freertos_SOURCE_DIR}/stream_buffer.c
    ${freertos_SOURCE_DIR}/list.c)
set(
    KERNEL_INCLUDES
    ${freertos_SOURCE_DIR}/include
    ${freertos_SOURCE_DIR}/portable/GCC/ARM_CM4F)

set(
    TRACING_SOURCES
    ${tracerecorder_SOURCE_DIR}/kernelports/FreeRTOS/trcKernelPort.c
    ${tracerecorder_SOURCE_DIR}/trcAssert.c
    ${tracerecorder_SOURCE_DIR}/trcCounter.c
    ${tracerecorder_SOURCE_DIR}/trcDiagnostics.c
    ${tracerecorder_SOURCE_DIR}/trcEntryTable.c
    ${tracerecorder_SOURCE_DIR}/trcError.c
    ${tracerecorder_SOURCE_DIR}/trcEventBuffer.c
    ${tracerecorder_SOURCE_DIR}/trcEvent.c
    ${tracerecorder_SOURCE_DIR}/trcExtension.c
    ${tracerecorder_SOURCE_DIR}/trcHardwarePort.c
    ${tracerecorder_SOURCE_DIR}/trcHeap.c
    ${tracerecorder_SOURCE_DIR}/trcInternalEventBuffer.c
    ${tracerecorder_SOURCE_DIR}/trcInterval.c
    ${tracerecorder_SOURCE_DIR}/trcISR.c
    ${tracerecorder_SOURCE_DIR}/trcMultiCoreEventBuffer.c
    ${tracerecorder_SOURCE_DIR}/trcObject.c
    ${tracerecorder_SOURCE_DIR}/trcPrint.c
    ${tracerecorder_SOURCE_DIR}/trcStackMonitor.c
    ${tracerecorder_SOURCE_DIR}/trcStateMachine.c
    ${tracerecorder_SOURCE_DIR}/trcStaticBuffer.c
    ${tracerecorder_SOURCE_DIR}/trcString.c
    ${tracerecorder_SOURCE_DIR}/trcTask.c
    ${tracerecorder_SOURCE_DIR}/trcTimestamp.c
    ${tracerecorder_SOURCE_DIR}/trcStreamingRecorder.c
    ${tracerecorder_SOURCE_DIR}/streamports/Jlink_RTT/SEGGER_RTT.c
    ${tracerecorder_SOURCE_DIR}/streamports/Jlink_RTT/trcStreamPort.c)
set(
    TRACING_INCLUDES
    ${tracerecorder_SOURCE_DIR}/kernelports/FreeRTOS/include
    ${tracerecorder_SOURCE_DIR}/include
    ${tracerecorder_SOURCE_DIR}/streamports/Jlink_RTT/include)

set(
    HAL_SOURCES
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c)
set(
    HAL_INCLUDES
    ${stm32cubef4_SOURCE_DIR}/Drivers/CMSIS/Include
    ${stm32cubef4_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
    ${stm32cubef4_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc)

set(
    APP_SOURCES
    src/main.c
    src/hooks.c
    src/led.c
    src/logging.c
    src/shell_cmds.c
    src/task_shell.c
    src/task_can1.c
    vendor/src/FreeRTOS_CLI.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
    ${stm32cubef4_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f429xx.s)
set(
    APP_INCLUDES
    include
    include/config
    vendor/include)

add_library(
    hal
    STATIC
    ${HAL_SOURCES})

target_include_directories(
    hal
    PRIVATE
    include/config
    vendor/include
    PUBLIC
    ${HAL_INCLUDES})

target_compile_definitions(
    hal
    PUBLIC
    STM32F4xx
    STM32F429xx
    USE_STM32F4XX_NUCLEO_144
    HSE_VALUE=8000000
    _FPU_PRESENT=1
    _FPU_USED=1
    ARM_MATH_CM4
    USE_FULL_LL_DRIVER
    USE_FULL_ASSERT)

add_library(
    freertos
    STATIC
    ${TRACING_SOURCES}
    ${KERNEL_SOURCES})

target_include_directories(
    freertos
    PRIVATE
    include/config
    include
    ${HAL_INCLUDES}
    PUBLIC
    ${TRACING_INCLUDES}
    ${KERNEL_INCLUDES})

add_executable(
    ${PROJECT_NAME}
    ${APP_SOURCES})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${APP_INCLUDES})

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    HEAP4)

target_link_libraries(
    ${PROJECT_NAME}
    hal
    freertos)

add_custom_target(
    openocd
    DEPENDS
    ${PROJECT_NAME})

add_custom_command(
    TARGET openocd
    POST_BUILD
    MAIN_DEPENDENCY ${PROJECT_NAME}
    COMMAND
    ./scripts/start_openocd.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Starting openocd")

add_custom_target(
    debug
    DEPENDS
    ${PROJECT_NAME})

add_custom_command(
    TARGET debug
    POST_BUILD
    MAIN_DEPENDENCY ${PROJECT_NAME}
    COMMAND
    gdb-multiarch build/${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Starting gdb session")

add_custom_target(
    flash
    DEPENDS
    ${PROJECT_NAME})

add_custom_command(
    TARGET flash
    POST_BUILD
    MAIN_DEPENDENCY ${PROJECT_NAME}
    COMMAND
    ./scripts/flash.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Flashing target")

print_exec_size()
elf_to_bin()
