#
# Arm SCP/MCP Software
# Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_library(${SCP_FIRMWARE_TARGET})

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../common/"
    "${CMAKE_CURRENT_BINARY_DIR}/optee/module/")

# cmake-lint: disable=E1122

target_include_directories(
    ${SCP_FIRMWARE_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
           "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(
    ${SCP_FIRMWARE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_mbx_smt.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_vpll.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_ppu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sensor.c")

set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
