#
# Arm SCP/MCP Software
# Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "tc2-bl1")

set(SCP_FIRMWARE_TARGET "tc2-bl1")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ENABLE_INBAND_MSG_SUPPORT TRUE)

set(SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT TRUE)

set(SCP_PLATFORM_VARIANT_INIT 0)

set(SCP_ARCHITECTURE "arm-m")

list(PREPEND SCP_MODULE_PATHS "${CMAKE_SOURCE_DIR}/module/cmn_booker")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_SOURCE_DIR}/product/totalcompute/tc2/module/tc2_bl1")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "ppu-v1")
list(APPEND SCP_MODULES "tc2-bl1")
list(APPEND SCP_MODULES "bootloader")
list(APPEND SCP_MODULES "system-pll")
list(APPEND SCP_MODULES "pik-clock")
list(APPEND SCP_MODULES "css-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "cmn-booker")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "mhu2")
list(APPEND SCP_MODULES "transport")
