#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "morello-fvp-bl2")

set(SCP_FIRMWARE_TARGET "morello-fvp-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT TRUE)

set(SCP_ARCHITECTURE "arm-m")

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ENABLE_DEBUGGER_INIT FALSE)

set(SCP_ENABLE_PLAT_FVP TRUE)

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/morello_pll")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/dmc_bing")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/morello_system")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/scmi_management")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "cmn-skeena")
list(APPEND SCP_MODULES "apcontext")
list(APPEND SCP_MODULES "power-domain")
list(APPEND SCP_MODULES "ppu-v1")
list(APPEND SCP_MODULES "ppu-v0")
list(APPEND SCP_MODULES "system-power")
list(APPEND SCP_MODULES "morello-pll")
list(APPEND SCP_MODULES "dmc-bing")
list(APPEND SCP_MODULES "mhu")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "pik-clock")
list(APPEND SCP_MODULES "css-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "scmi-power-domain")
list(APPEND SCP_MODULES "scmi-system-power")
list(APPEND SCP_MODULES "scmi-management")
list(APPEND SCP_MODULES "ssc")
list(APPEND SCP_MODULES "system-info")
list(APPEND SCP_MODULES "psu")
list(APPEND SCP_MODULES "mock-psu")
list(APPEND SCP_MODULES "dvfs")
list(APPEND SCP_MODULES "scmi-perf")
list(APPEND SCP_MODULES "morello-system")
list(APPEND SCP_MODULES "resource-perms")
