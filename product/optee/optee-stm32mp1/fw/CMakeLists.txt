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

# Include common optee modules in the module list
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
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_clocks.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_reset_domains.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_voltage_domains.c")

set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
