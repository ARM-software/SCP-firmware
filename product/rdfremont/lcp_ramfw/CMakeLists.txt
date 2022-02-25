#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rdfremont-lcp)

target_include_directories(
    rdfremont-lcp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    rdfremont-lcp
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_armv8m_mpu.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdfremont-lcp PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rdfremont-lcp
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
