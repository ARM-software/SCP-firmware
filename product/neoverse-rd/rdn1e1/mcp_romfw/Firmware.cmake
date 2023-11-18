#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "rdn1e1-mcp-bl1")

set(SCP_FIRMWARE_TARGET "rdn1e1-mcp-bl1")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ARCHITECTURE "arm-m")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "bootloader")
list(APPEND SCP_MODULES "isys-rom")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "clock")
