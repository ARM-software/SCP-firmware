#
# Arm SCP/MCP Software
# Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(juno-bl2)

# SCP_PLATFORM_VARIANT options:
# - 'BOARD' for building to run on SoC (default)
# - 'FVP' for building to run on an FVP

target_compile_definitions(juno-bl2 PUBLIC -DJUNO_VARIANT_BOARD=0)
target_compile_definitions(juno-bl2 PUBLIC -DJUNO_VARIANT_FVP=1)

if (SCP_PLATFORM_VARIANT STREQUAL "BOARD")
    message(NOTICE "SCP_PLATFORM_VARIANT set to BOARD (juno-bl2)\n")
    target_compile_definitions(juno-bl2
        PUBLIC -DPLATFORM_VARIANT=JUNO_VARIANT_BOARD)

    target_sources(juno-bl2 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_xrp7724.c")
    target_sources(juno-bl2 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_pvt.c")
    target_sources(juno-bl2 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_adc.c")

elseif (SCP_PLATFORM_VARIANT STREQUAL "FVP")
    message(NOTICE "SCP_PLATFORM_VARIANT set to FVP (juno-bl2)\n")
    target_compile_definitions(juno-bl2
        PUBLIC -DPLATFORM_VARIANT=JUNO_VARIANT_FVP)

    target_sources(juno-bl2 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_clock.c")
    target_sources(juno-bl2 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.c")

else ()
    message(FATAL_ERROR "Valid platform variant options for Juno are: \n"
                                "BOARD (default) \n"
                                "FVP \n")
endif()

target_include_directories(
    juno-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    juno-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_soc_clock_ram.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_cdcel937.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_hdlcd.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ddr_phy400.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_dmc400.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ram.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ppu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_i2c.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sensor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_reg_sensor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_thermal.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_utils.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_id.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_scmi_clock.c")

if(SCP_ENABLE_DEBUG_UNIT)
    target_sources(
        juno-bl2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_debug.c"
                         "${CMAKE_CURRENT_SOURCE_DIR}/config_debug.c")
endif()

if(SCP_ENABLE_SCMI_RESET)
    target_sources(
        juno-bl2
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/config_reset_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_reset_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_reset_domain.c")
endif()

if(SCP_ENABLE_STATISTICS)
    target_sources(juno-bl2
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_stats.c")
endif()

if(${SCP_ENABLE_RESOURCE_PERMISSIONS})
    target_sources(
        juno-bl2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_resource_perms.c")
endif()

if(SCP_ENABLE_PMI)
    target_sources(
        juno-bl2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_dwt_pmi.c"
                         "${CMAKE_CURRENT_SOURCE_DIR}/config_pmi.c")
endif()

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(juno-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    juno-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)

if(SCP_ENABLE_DEBUG_UNIT)
    list(APPEND SCP_MODULES "debug")
    list(APPEND SCP_MODULES "juno-debug")
endif()

if(SCP_ENABLE_SCMI_RESET)
    list(APPEND SCP_MODULES "reset-domain")
    list(APPEND SCP_MODULES "scmi-reset-domain")
    list(APPEND SCP_MODULES "juno-reset-domain")
endif()

if(SCP_ENABLE_RESOURCE_PERMISSIONS)
    list(APPEND SCP_MODULES "resource-perms")
endif()

if(SCP_ENABLE_STATISTICS)
    list(APPEND SCP_MODULES "statistics")
endif()

set(SCP_MODULES ${SCP_MODULES} PARENT_SCOPE)
set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
