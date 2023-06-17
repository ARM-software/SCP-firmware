#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "n1sdp-bl2")

set(SCP_FIRMWARE_TARGET "n1sdp-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT TRUE)

set(SCP_ARCHITECTURE "arm-m")

list(PREPEND SCP_MODULE_PATHS "${CMAKE_SOURCE_DIR}/module/fip")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_pcie")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_remote_pd")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_ddr_phy")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_scp2pcc")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_system")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_dmc620")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/scmi_ccix_config")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_sensor")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_c2c")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../../../module/cdns_i2c")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/n1sdp_timer_sync")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/scmi_management")


list(APPEND SCP_MODULES "armv7m-mpu")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "cmn600")
list(APPEND SCP_MODULES "apcontext")
list(APPEND SCP_MODULES "power-domain")
list(APPEND SCP_MODULES "ppu-v1")
list(APPEND SCP_MODULES "ppu-v0")
list(APPEND SCP_MODULES "system-power")
list(APPEND SCP_MODULES "sc-pll")
list(APPEND SCP_MODULES "cdns-i2c")
list(APPEND SCP_MODULES "n1sdp-dmc620")
list(APPEND SCP_MODULES "n1sdp-ddr-phy")
list(APPEND SCP_MODULES "mhu")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "pik-clock")
list(APPEND SCP_MODULES "css-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "n1sdp-scp2pcc")
list(APPEND SCP_MODULES "n1sdp-sensor")
list(APPEND SCP_MODULES "sensor")
list(APPEND SCP_MODULES "scmi-power-domain")
list(APPEND SCP_MODULES "scmi-sensor")
list(APPEND SCP_MODULES "scmi-system-power")
list(APPEND SCP_MODULES "scmi-management")
list(APPEND SCP_MODULES "scmi-ccix-config")
list(APPEND SCP_MODULES "fip")
list(APPEND SCP_MODULES "n1sdp-timer-sync")
list(APPEND SCP_MODULES "n1sdp-c2c")
list(APPEND SCP_MODULES "n1sdp-remote-pd")
list(APPEND SCP_MODULES "n1sdp-pcie")
list(APPEND SCP_MODULES "ssc")
list(APPEND SCP_MODULES "system-info")
list(APPEND SCP_MODULES "n1sdp-system")
