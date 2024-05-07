#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
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

set(SCP_PLATFORM_VARIANT_INIT "BOARD")

#
# If SCP_PLATFORM_VARIANT hasn't been set yet, it will be set to the default
# value (BOARD)
#

set(SCP_PLATFORM_VARIANT ${SCP_PLATFORM_VARIANT_INIT} CACHE STRING
    "Platform variant for the build")

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_rom")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/juno_ppu")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/juno_soc_clock")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "juno-ppu")
list(APPEND SCP_MODULES "juno-rom")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "bootloader")
list(APPEND SCP_MODULES "juno-soc-clock")
