#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_FIRMWARE "sgm775-bl1")
set(SCP_FIRMWARE_TARGET "sgm775-bl1")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY TRUE)

set(SCP_ENABLE_DEBUGGER FALSE)
set(SCP_ENABLE_NOTIFICATIONS TRUE)
set(SCP_ENABLE_RESOURCE_PERMISSIONS FALSE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ARCHITECTURE "arm-m")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "sid")
list(APPEND SCP_MODULES "system-info")
list(APPEND SCP_MODULES "pcid")
list(APPEND SCP_MODULES "ppu-v0")
list(APPEND SCP_MODULES "ppu-v1")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "msys-rom")
list(APPEND SCP_MODULES "bootloader")
list(APPEND SCP_MODULES "system-pll")
list(APPEND SCP_MODULES "css-clock")
list(APPEND SCP_MODULES "pik-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "sds")
