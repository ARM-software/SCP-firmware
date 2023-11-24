#
# Arm SCP/MCP Software
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "rdfremont-lcp-bl2")

set(SCP_FIRMWARE_TARGET "rdfremont-lcp-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ARCHITECTURE "arm-m")

set(SCP_ENABLE_DEBUGGER_INIT FALSE)

set(SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT TRUE)

list(PREPEND SCP_MODULE_PATHS
    "${CMAKE_CURRENT_LIST_DIR}/../module/mod_lcp_platform")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "armv8m-mpu")
list(APPEND SCP_MODULES "lcp-platform")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "mhu3")
