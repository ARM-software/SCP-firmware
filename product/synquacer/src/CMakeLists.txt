#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(DEVICE_NOR "MACRONIX")
#set(DEVICE_NOR "MICRON")
#set(DEVICE_NOR "INFINEON")

if(DEVICE_NOR STREQUAL "MACRONIX")
    add_compile_definitions(PUBLIC DEVICE_NOR_MX25)
    target_sources(
        synquacer-bl2
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/device_nor_mx25.c")
endif()

if(DEVICE_NOR STREQUAL "MICRON")
    add_compile_definitions(PUBLIC DEVICE_NOR_MT25)
    target_sources(
        synquacer-bl2
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/device_nor_mt25.c")
endif()

if(DEVICE_NOR STREQUAL "INFINEON")
    add_compile_definitions(PUBLIC DEVICE_NOR_S25)
    target_sources(
        synquacer-bl2
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/device_nor_s25.c")
endif()
