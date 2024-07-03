#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rcar-bl2)

target_include_directories(
    rcar-bl2
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/CMSIS-FreeRTOS/Source/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/CMSIS-FreeRTOS/Source/portable/GCC/ARM_CA53_64_Rcar"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
        "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

set(freertos_source "${CMAKE_CURRENT_SOURCE_DIR}/../src/CMSIS-FreeRTOS/Source")
set(freertos2_source
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/CMSIS-FreeRTOS/CMSIS/RTOS2/FreeRTOS/Source")

target_sources(
    rcar-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_scif.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_pd_sysc.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_pd_core.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_pd_pmic_bd9571.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sensor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_sd_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_mstp_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_ext_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_pmic.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_mock_pmic_bd9571.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_mfismh.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_apcore.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_system.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/rcar_core.c"
            "${freertos_source}/timers.c"
            "${freertos_source}/list.c"
            "${freertos_source}/queue.c"
            "${freertos_source}/tasks.c"
            "${freertos_source}/portable/MemMang/heap_1.c"
            "${freertos_source}/portable/GCC/ARM_CA53_64_Rcar/portASM.S"
            "${freertos_source}/portable/GCC/ARM_CA53_64_Rcar/port.c"
            "${freertos2_source}/cmsis_os2_tiny4scp.c")

if(SCP_ENABLE_RESOURCE_PERMISSIONS)
    target_sources(rcar-bl2
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_resource_perms.c")
endif()

if(SCP_ENABLE_SCMI_RESET)
    target_sources(rcar-bl2
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_rcar_reset.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/config_reset_domain.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_reset_domain.c")
endif()

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rcar-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::rtos2,INTERFACE_INCLUDE_DIRECTORIES>)

target_include_directories(rcar-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)

if(SCP_ENABLE_SCMI_RESET)
    list(APPEND SCP_MODULES "reset-domain")
    list(APPEND SCP_MODULES "rcar-reset")
    list(APPEND SCP_MODULES "scmi-reset-domain")
endif()

if(SCP_ENABLE_RESOURCE_PERMISSIONS)
    list(APPEND SCP_MODULES "resource-perms")
endif()

set(SCP_MODULES ${SCP_MODULES} PARENT_SCOPE)
set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
