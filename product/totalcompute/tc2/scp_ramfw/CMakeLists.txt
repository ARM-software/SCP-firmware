#
# Arm SCP/MCP Software
# Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(tc2-bl2)

# SCP_PLATFORM_VARIANT options:
# - 'TC2_VARIANT_STD' for TC2 standard build
# - 'TC2_VAR_EXPERIMENT_POWER' for TC2 with power/performance/thermal additions
#   used for evaluation purpose:
#   - TRAFFIC_COP on CORTEX_A520 cores
#   - MPMM on CORTEX_A520, CORTEX_A720, CORTEX_X4 cores
#   - THERMAL_MANAGEMENT for the entire system, with a simplified/dummy power
#     model

target_compile_definitions(tc2-bl2 PUBLIC -DTC2_VARIANT_STD=0)
target_compile_definitions(tc2-bl2 PUBLIC -DTC2_VAR_EXPERIMENT_POWER=1)

if (SCP_PLATFORM_VARIANT STREQUAL "1")
    message(NOTICE "SCP_PLATFORM_VARIANT set to: EXPERIMENT_POWER (tc2-bl2)\n")

    target_compile_definitions(tc2-bl2
        PUBLIC -DPLATFORM_VARIANT=TC2_VAR_EXPERIMENT_POWER)

    set(SCP_ENABLE_PLUGIN_HANDLER TRUE PARENT_SCOPE)
    set(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS TRUE PARENT_SCOPE)

# The following list of modules will be appended in Firmware.cmake in the same
# order

    list(APPEND SCP_MODULES "traffic-cop")
    target_sources(tc2-bl2 PRIVATE "config_traffic_cop.c")

    list(APPEND SCP_MODULES "amu-mmap")
    target_sources(tc2-bl2 PRIVATE "config_amu_mmap.c")
    target_include_directories(tc2-bl2 PUBLIC
        "${CMAKE_SOURCE_DIR}/interface/amu")

    list(APPEND SCP_MODULES "mpmm")
    target_sources(tc2-bl2 PRIVATE "config_mpmm.c")

    list(APPEND SCP_MODULES "sensor")
    target_sources(tc2-bl2 PRIVATE "config_sensor.c")

    list(APPEND SCP_MODULES "reg-sensor")
    target_sources(tc2-bl2 PRIVATE "config_reg_sensor.c")

    list(APPEND SCP_MODULES "thermal-mgmt")
    target_sources(tc2-bl2 PRIVATE "config_thermal_mgmt.c")

    list(APPEND SCP_MODULES "tc2-power-model")
    list(PREPEND SCP_MODULE_PATHS
        "${CMAKE_CURRENT_LIST_DIR}/../module/tc2_power_model")
    target_sources(tc2-bl2 PRIVATE "config_tc2_power_model.c")

else()
    target_compile_definitions(tc2-bl2
        PUBLIC -DPLATFORM_VARIANT=TC2_VARIANT_STD)
endif()

if(SCP_ENABLE_RESOURCE_PERMISSIONS)
    list(APPEND SCP_MODULES "resource-perms")
endif()

target_include_directories(
    tc2-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../../common/include"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw"
                   "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    tc2-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_scmi_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_mhu2.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_ppu_v1.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_scmi_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../common/scp_ramfw/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_gtimer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_pll.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_css_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c")

if(SCP_ENABLE_RESOURCE_PERMISSIONS)
    target_sources(tc2-bl2 PRIVATE "config_resource_perms.c")
endif()

if(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS)
    list(APPEND SCP_MODULES "fch-polled")
    target_sources(tc2-bl2 PRIVATE "config_fch_polled.c")
endif()

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(tc2-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(tc2-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)

if(SCP_ENABLE_PLUGIN_HANDLER)
    message(NOTICE "SCP_ENABLE_PLUGIN_HANDLER enabled")
endif()

set(SCP_MODULES ${SCP_MODULES} PARENT_SCOPE)
set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
