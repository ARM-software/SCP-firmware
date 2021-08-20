#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_FIRMWARE "rdv1-bl1")
set(SCP_FIRMWARE_TARGET "rdv1-bl1")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_FIRMWARE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(SCP_GENERATE_FLAT_BINARY TRUE)
set(SCP_ARCHITECTURE "arm-m")

set(SCP_ENABLE_NOTIFICATIONS TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "sid")
list(APPEND SCP_MODULES "system-info")
list(APPEND SCP_MODULES "pcid")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "bootloader")
list(APPEND SCP_MODULES "isys-rom")
