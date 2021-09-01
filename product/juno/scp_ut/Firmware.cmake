#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
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

set(SCP_ENABLE_FAST_CHANNELS_INIT FALSE)

set(SCP_ENABLE_RESOURCE_PERMISSIONS_INIT FALSE)

set(SCP_ENABLE_DEBUG_UNIT_INIT FALSE)

set(SCP_ENABLE_SCMI_RESET_INIT FALSE)

set(SCP_ENABLE_STATISTICS_INIT FALSE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ENABLE_PMI_INIT FALSE)

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_SOURCE_DIR}/module/ut")

list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "ut")
