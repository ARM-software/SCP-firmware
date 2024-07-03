#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(juno-bl1)

if (NOT SCP_PLATFORM_VARIANT STREQUAL "BOARD" AND
    NOT SCP_PLATFORM_VARIANT STREQUAL "FVP")
        message(FATAL_ERROR "Valid platform variant options for Juno are: \n"
                                    "BOARD (default) \n"
                                    "FVP \n")
endif()

target_include_directories(
    juno-bl1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    juno-bl1
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ppu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_rom.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_soc_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_bootloader.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_utils.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_id.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(juno-bl1 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    juno-bl1
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
