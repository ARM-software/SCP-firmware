#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_FIRMWARE "juno-bl2")
set(SCP_FIRMWARE_TARGET "juno-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_FIRMWARE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ARCHITECTURE "arm-m")

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_SCMI_NOTIFICATIONS_INIT FALSE)

set(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS_INIT FALSE)

set(SCP_ENABLE_RESOURCE_PERMISSIONS_INIT FALSE)

set(SCP_ENABLE_DEBUG_UNIT_INIT FALSE)

set(SCP_ENABLE_SCMI_RESET_INIT FALSE)

set(SCP_ENABLE_STATISTICS_INIT FALSE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ENABLE_PMI_INIT FALSE)

# setup Device Tree sources 
# the root source for this firmware variant
set(SCP_FIRMWARE_DTS_SOURCE "${CMAKE_CURRENT_LIST_DIR}/juno-scp_ramfw.dts")

# the includes needed:
# the current include dir where generated headers appear
list(PREPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../include")
# the dts/include/juno specific .dtsi files for this product
list(PREPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_SOURCE_DIR}/dts/include/juno")

list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_reset_domain")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_cdcel937")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_rom")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_ppu")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_soc_clock_ram")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_adc")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_soc_clock")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_thermal")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_ram")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_pvt")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_ddr_phy400")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_dmc400")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_debug")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_system")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_xrp7724")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_hdlcd")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_SOURCE_DIR}/module/dwt_pmi")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_SOURCE_DIR}/module/pmi")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "juno-soc-clock-ram")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "juno-cdcel937")
list(APPEND SCP_MODULES "juno-hdlcd")
list(APPEND SCP_MODULES "dvfs")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "juno-ddr-phy400")
list(APPEND SCP_MODULES "juno-dmc400")
list(APPEND SCP_MODULES "juno-ram")
list(APPEND SCP_MODULES "juno-ppu")
list(APPEND SCP_MODULES "juno-system")
list(APPEND SCP_MODULES "system-power")
list(APPEND SCP_MODULES "power-domain")
list(APPEND SCP_MODULES "mhu")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "scmi-clock")
list(APPEND SCP_MODULES "scmi-perf")
list(APPEND SCP_MODULES "scmi-power-domain")
list(APPEND SCP_MODULES "sensor")
list(APPEND SCP_MODULES "scmi-sensor")
list(APPEND SCP_MODULES "scmi-system-power")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "i2c")
list(APPEND SCP_MODULES "dw-apb-i2c")
list(APPEND SCP_MODULES "juno-adc")
list(APPEND SCP_MODULES "juno-xrp7724")
list(APPEND SCP_MODULES "reg-sensor")
list(APPEND SCP_MODULES "psu")
list(APPEND SCP_MODULES "mock-psu")
list(APPEND SCP_MODULES "juno-pvt")
list(APPEND SCP_MODULES "juno-thermal")
list(APPEND SCP_MODULES "mock-clock")

# select device tree configured modules
# These use the .dts configured variants
# Order is not important for this list
list(APPEND SCP_DT_CONFIG_MODULES_DYN "clock")
list(APPEND SCP_DT_CONFIG_MODULES_STD "pl011")
list(APPEND SCP_DT_CONFIG_MODULES_STD "mock-clock")
list(APPEND SCP_DT_CONFIG_MODULES_STD "power-domain")
list(APPEND SCP_DT_CONFIG_MODULES_STD "gtimer")
list(APPEND SCP_DT_CONFIG_MODULES_STD "timer")
list(APPEND SCP_DT_CONFIG_MODULES_STD "system-power")
list(APPEND SCP_DT_CONFIG_MODULES_STD "mock-psu")
list(APPEND SCP_DT_CONFIG_MODULES_STD "juno-ppu")
list(APPEND SCP_DT_CONFIG_MODULES_DYN "sds")
list(APPEND SCP_DT_CONFIG_MODULES_STD "scmi")
