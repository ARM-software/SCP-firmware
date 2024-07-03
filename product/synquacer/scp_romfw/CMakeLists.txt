#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(synquacer-bl1)

target_include_directories(
    synquacer-bl1
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
           "${CMAKE_CURRENT_SOURCE_DIR}"
           "${CMAKE_SOURCE_DIR}/contrib/cmsis/git/CMSIS/Core/Include")

# cmake-lint: disable=E1122

target_sources(
    synquacer-bl1
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_synquacer_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_synquacer_rom.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(synquacer-bl1 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    synquacer-bl1
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
