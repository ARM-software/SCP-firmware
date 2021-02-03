#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(n1sdp-bl2)

target_include_directories(
    n1sdp-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                          "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    n1sdp-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/rtx_config.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/n1sdp_core.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ssc.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_info.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v0.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v1.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_i2c.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_dmc620.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_ddr_phy.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_smt.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_cmn600.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_pll.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_css_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_pcie.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_scp2pcc.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sensor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_apcontext.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_c2c_i2c.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_remote_pd.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_n1sdp_timer_sync.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c")

if(SCP_ENABLE_DEBUGGER)
    target_sources(n1sdp-bl2 PRIVATE "config_debugger_cli.c")
endif()

if(SCP_ENABLE_MULTITHREADING)
    target_sources(n1sdp-bl2
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rtx_config.c")
    target_link_libraries(n1sdp-bl2 PRIVATE cmsis::rtos2-rtx)
endif()

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(n1sdp-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(n1sdp-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)