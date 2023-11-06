#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "rcar-bl2")

set(SCP_FIRMWARE_TARGET "rcar-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_SCMI_RESET_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT TRUE)

set(SCP_ARCHITECTURE "armv8-a")

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_arch_timer")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_reset")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_pmic")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_pd_pmic_bd9571")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_scif")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_mfismh")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_system_power")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_system")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_sd_clock")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_pd_core")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_reg_sensor")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_clock")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_ext_clock")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_pd_sysc")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_mock_pmic_bd9571")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/rcar_mstp_clock")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "rcar-scif")
list(APPEND SCP_MODULES "rcar-system")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "rcar-clock")
list(APPEND SCP_MODULES "rcar-sd-clock")
list(APPEND SCP_MODULES "rcar-mstp-clock")
list(APPEND SCP_MODULES "rcar-ext-clock")
list(APPEND SCP_MODULES "rcar-system-power")
list(APPEND SCP_MODULES "dvfs")
list(APPEND SCP_MODULES "psu")
list(APPEND SCP_MODULES "rcar-arch-timer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "rcar-pmic")
list(APPEND SCP_MODULES "rcar-mock-pmic-bd9571")
list(APPEND SCP_MODULES "rcar-mfismh")
list(APPEND SCP_MODULES "power-domain")
list(APPEND SCP_MODULES "rcar-pd-sysc")
list(APPEND SCP_MODULES "rcar-pd-core")
list(APPEND SCP_MODULES "rcar-pd-pmic-bd9571")
list(APPEND SCP_MODULES "rcar-reg-sensor")
list(APPEND SCP_MODULES "sensor")
list(APPEND SCP_MODULES "scmi-perf")
list(APPEND SCP_MODULES "scmi-power-domain")
list(APPEND SCP_MODULES "scmi-clock")
list(APPEND SCP_MODULES "scmi-sensor")
list(APPEND SCP_MODULES "scmi-system-power")
list(APPEND SCP_MODULES "scmi-apcore")
list(APPEND SCP_MODULES "sds")
