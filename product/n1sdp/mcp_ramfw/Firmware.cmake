#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "n1sdp-mcp-bl2")

set(SCP_FIRMWARE_TARGET "n1sdp-mcp-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT TRUE)

set(SCP_ARCHITECTURE "arm-m")

set(SCP_FIRMWARE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/scmi_agent")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_mcp_system")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "armv7m-mpu")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "pik-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "mhu")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "scmi-agent")
list(APPEND SCP_MODULES "n1sdp-mcp-system")
