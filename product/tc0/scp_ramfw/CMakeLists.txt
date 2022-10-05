#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(tc0-bl2)


# SCP_PLATFORM_VARIANT options:
# - 'TC0_VARIANT_STD' for TC0 standard build
# - 'TC0_VAR_EXPERIMENT_POWER' for TC0 with power/performance plugins used for
#   evaluation purposes


target_compile_definitions(tc0-bl2 PUBLIC -DTC0_VARIANT_STD=0)
target_compile_definitions(tc0-bl2 PUBLIC -DTC0_VAR_EXPERIMENT_POWER=1)


set(SCP_PLATFORM_VARIANT ${SCP_PLATFORM_VARIANT_INIT} CACHE STRING "1")


if (SCP_PLATFORM_VARIANT STREQUAL "1")
    message(NOTICE "SCP_PLATFORM_VARIANT set to EXPERIMENT_POWER (tc0-bl2)\n")

    target_compile_definitions(tc0-bl2
        PUBLIC -DPLATFORM_VARIANT=TC0_VAR_EXPERIMENT_POWER)

    set(SCP_ENABLE_PLUGIN_HANDLER TRUE PARENT_SCOPE)
    set(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS TRUE PARENT_SCOPE)

# The order of the modules in the following list is appended on the list of
# modules defined in Firmware.cmake.

    list(APPEND SCP_MODULES "traffic-cop")
    target_sources(tc0-bl2 PRIVATE "config_traffic_cop.c")

    list(APPEND SCP_MODULES "mpmm")
    target_sources(tc0-bl2 PRIVATE "config_mpmm.c")

    list(APPEND SCP_MODULES "sensor")
    target_sources(tc0-bl2 PRIVATE "config_sensor.c")

    list(APPEND SCP_MODULES "reg-sensor")
    target_sources(tc0-bl2 PRIVATE "config_reg_sensor.c")

    list(APPEND SCP_MODULES "thermal-mgmt")
    target_sources(tc0-bl2 PRIVATE "config_thermal_mgmt.c")

    list(APPEND SCP_MODULES "tc0-power-model")
    list(PREPEND SCP_MODULE_PATHS
        "${CMAKE_CURRENT_LIST_DIR}/../module/tc0_power_model")
    target_sources(tc0-bl2 PRIVATE "config_tc0_power_model.c")

else()
    message(NOTICE "SCP_PLATFORM_VARIANT set to STANDARD (tc0-bl2)\n")

    target_compile_definitions(tc0-bl2
        PUBLIC -DPLATFORM_VARIANT=TC0_VARIANT_STD)
endif()


target_include_directories(
    tc0-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                   "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    tc0-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v1.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu2.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_gtimer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_voltage_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_voltage_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_voltage_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_pll.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_css_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c")

if(SCP_ENABLE_RESOURCE_PERMISSIONS)
    target_sources(tc0-bl2 PRIVATE "config_resource_perms.c")
endif()


#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(tc0-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(tc0-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)

if(SCP_ENABLE_RESOURCE_PERMISSIONS)
    list(APPEND SCP_MODULES "resource-perms")
endif()

set(SCP_MODULES ${SCP_MODULES} PARENT_SCOPE)
set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
