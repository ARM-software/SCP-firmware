#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rdv1-mcp-bl1)

target_include_directories(
    rdv1-mcp-bl1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                        "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    rdv1-mcp-bl1
    PRIVATE "config_clock.c" "config_bootloader.c"
            "config_pl011.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdv1-mcp-bl1 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rdv1-mcp-bl1
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
