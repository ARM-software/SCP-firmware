#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

cmake_minimum_required(VERSION 3.18.3)

project(
    CMSIS
    VERSION 5.2.0
    DESCRIPTION "Cortex Microcontroller Software Interface Standard (CMSIS)"
    HOMEPAGE_URL "https://developer.arm.com/tools-and-software/embedded/cmsis"
    LANGUAGES C ASM)

#
# Define CMSIS-Core (M).
#

add_library(core-m INTERFACE)

target_include_directories(
    core-m INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/Core/Include")

add_library(cmsis::core-m ALIAS core-m)

#
# Define CMSIS-RTOS2.
#

add_library(rtos2 INTERFACE)

target_include_directories(
    rtos2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/Include")

add_library(cmsis::rtos2 ALIAS rtos2)

#
# Define and build CMSIS-RTOS2 RTX.
#

add_library(rtos2-rtx INTERFACE)

target_include_directories(
    rtos2-rtx
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
              "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Include"
              "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source")

# cmake-lint: disable=E1122
target_sources(
    rtos2-rtx
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_delay.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_evflags.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_evr.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_kernel.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_memory.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_mempool.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_msgqueue.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_mutex.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_semaphore.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_system.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_thread.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/rtx_timer.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/Source/os_systick.c")

target_compile_definitions(rtos2-rtx INTERFACE RTX_NO_MULTITHREAD_CLIB)
target_compile_options(rtos2-rtx INTERFACE -Wno-array-bounds)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m[37]")
    target_sources(
        rtos2-rtx
        INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/RTX/Source/GCC/irq_cm3.S"
    )
endif()

target_link_libraries(rtos2-rtx INTERFACE core-m rtos2)

add_library(cmsis::rtos2-rtx ALIAS rtos2-rtx)
