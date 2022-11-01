#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_FIRMWARE "sgm776-bl2")
set(SCP_FIRMWARE_TARGET "sgm776-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_RESOURCE_PERMISSIONS_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ARCHITECTURE "arm-m")

list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/sgm776_system")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "sid")
list(APPEND SCP_MODULES "system-info")
list(APPEND SCP_MODULES "pcid")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "ddr-phy500")
list(APPEND SCP_MODULES "dmc500")
list(APPEND SCP_MODULES "reg-sensor")
list(APPEND SCP_MODULES "sensor")
list(APPEND SCP_MODULES "ppu-v1")
list(APPEND SCP_MODULES "system-power")
list(APPEND SCP_MODULES "sgm776-system")
list(APPEND SCP_MODULES "power-domain")
list(APPEND SCP_MODULES "psu")
list(APPEND SCP_MODULES "mock-psu")
list(APPEND SCP_MODULES "mhu2")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "system-pll")
list(APPEND SCP_MODULES "pik-clock")
list(APPEND SCP_MODULES "css-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "dvfs")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "scmi-power-domain")
list(APPEND SCP_MODULES "scmi-system-power")
list(APPEND SCP_MODULES "scmi-clock")
list(APPEND SCP_MODULES "scmi-perf")
list(APPEND SCP_MODULES "scmi-sensor")
list(APPEND SCP_MODULES "scmi-apcore")
list(APPEND SCP_MODULES "sds")
