#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(juno-bl1-bypass)

if (NOT SCP_PLATFORM_VARIANT STREQUAL "BOARD" AND
    NOT SCP_PLATFORM_VARIANT STREQUAL "FVP")
        message(FATAL_ERROR "Valid platform variant options for Juno are: \n"
                                    "BOARD (default) \n"
                                    "FVP \n")
endif()

target_compile_definitions(juno-bl1-bypass PUBLIC -DSCP_ROM_BYPASS=1)

target_include_directories(
    juno-bl1-bypass PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                           "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    juno-bl1-bypass
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ppu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_rom.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_bootloader.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/juno_pll_workaround.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_utils.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_id.c")

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#
target_include_directories(
    juno-bl1-bypass
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_options(juno-bl1-bypass
        PUBLIC "LINKER:--wrap=arch_exception_reset")
endif()
