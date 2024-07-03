#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(synquacer-bl2)

# cmake-lint: disable=E1122

target_include_directories(
    synquacer-bl2
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
           "${CMAKE_CURRENT_SOURCE_DIR}")

include(${CMAKE_CURRENT_SOURCE_DIR}/../src/CMakeLists.txt)

target_sources(
    synquacer-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ccn512.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_css_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_f_i2c.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_hsspi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_nor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v0_synquacer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_apcore.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_synquacer_memc.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c")

if(SYNQUACER_UART MATCHES "uart0")
    target_sources(synquacer-bl2
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_f_uart3.c")
elseif(SYNQUACER_UART MATCHES "uart1")
    target_sources(synquacer-bl2
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c")
elseif(SYNQUACER_UART MATCHES "debug_uart")
    target_sources(synquacer-bl2
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c")
else()
    target_sources(synquacer-bl2
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_f_uart3.c")
endif()

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(synquacer-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    synquacer-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
