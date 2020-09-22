#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(sgm775-bl1)

target_include_directories(
    sgm775-bl1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                      "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    sgm775-bl1
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_msys_rom.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_bootloader.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_pll.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_css_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v0.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v1.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/sgm775_core.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/config_sid.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/config_system_info.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(sgm775-bl1 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    sgm775-bl1
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
