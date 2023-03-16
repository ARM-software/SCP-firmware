#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_FIRMWARE "juno-bl1-bypass")
set(SCP_FIRMWARE_TARGET "juno-bl1-bypass")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_FIRMWARE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")


set(SCP_GENERATE_FLAT_BINARY TRUE)

set(SCP_ARCHITECTURE "arm-m")

set(SCP_ENABLE_NOTIFICATIONS TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)


# setup Device Tree sources 
# the root source for this firmware variant
set(SCP_FIRMWARE_DTS_SOURCE "${CMAKE_CURRENT_LIST_DIR}/juno-scp_romfw_bypass.dts")

# the includes needed:
# the current include dir where generated headers appear
list(PREPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../include")
# the dts/include/juno specific .dtsi files for this product
list(PREPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_SOURCE_DIR}/dts/include/juno")

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_rom")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_ppu")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_soc_clock")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "juno-ppu")
list(APPEND SCP_MODULES "juno-rom")
list(APPEND SCP_MODULES "juno-soc-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "bootloader")

# select device tree configured modules
# These use the .dts configured variants
# Order is not important for this list
list(APPEND SCP_DT_CONFIG_MODULES_STD "juno-ppu")
list(APPEND SCP_DT_CONFIG_MODULES_STD "juno-rom")
list(APPEND SCP_DT_CONFIG_MODULES_STD "clock")
list(APPEND SCP_DT_CONFIG_MODULES_STD "gtimer")
list(APPEND SCP_DT_CONFIG_MODULES_DYN "sds")
list(APPEND SCP_DT_CONFIG_MODULES_STD "bootloader")
