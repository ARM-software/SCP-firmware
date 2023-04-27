#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(tc1-bl1)

target_include_directories(
    tc1-bl1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../../common/include"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_romfw"
                   "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    tc1-bl1
    PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_romfw/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_romfw/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_romfw/config_bootloader.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v1.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_cmn_booker.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_pll.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_css_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_gtimer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_msys_rom.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(tc1-bl1 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(tc1-bl1
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
