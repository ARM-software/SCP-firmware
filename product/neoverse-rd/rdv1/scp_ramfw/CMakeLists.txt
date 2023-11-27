#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rdv1-bl2)

target_include_directories(
    rdv1-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                        "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    rdv1-bl2
    PRIVATE "config_system_power.c"
            "config_armv7m_mpu.c"
            "config_power_domain.c"
            "config_ppu_v1.c"
            "config_mhu2.c"
            "config_transport.c"
            "config_scmi.c"
            "config_sds.c"
            "config_timer.c"
            "config_gtimer.c"
            "config_cmn650.c"
            "config_scmi_system_power.c"
            "config_system_pll.c"
            "config_pik_clock.c"
            "config_css_clock.c"
            "config_clock.c"
            "config_apcontext.c"
            "config_scmi_power_domain.c"
            "config_psu.c"
            "config_mock_psu.c"
            "config_dvfs.c"
            "config_scmi_perf.c"
            "../src/config_system_info.c"
            "../src/config_pl011.c"
            "../src/config_sid.c")

if(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS)
    target_sources(rdv1-bl2 PRIVATE "config_fch_polled.c")
endif()

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdv1-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rdv1-bl2
   PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
