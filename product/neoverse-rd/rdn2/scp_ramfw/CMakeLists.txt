#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rdn2-bl2)

# SCP_PLATFORM_VARIANT value for supported RD-N2 platform variants:
# - '0' for RD-N2 platform
# - '1' for RD-N2-Cfg1 platform
# - '2' for RD-N2-Cfg2 platform
# - '3' for RD-N2-Cfg3 platform

if (SCP_PLATFORM_VARIANT STREQUAL "0")
      target_compile_definitions(rdn2-bl2
                                 PUBLIC -DPLATFORM_VARIANT=0)
elseif (SCP_PLATFORM_VARIANT STREQUAL "1")
      target_compile_definitions(rdn2-bl2
                                 PUBLIC -DPLATFORM_VARIANT=1)
elseif (SCP_PLATFORM_VARIANT STREQUAL "2")
      target_compile_definitions(rdn2-bl2
                                 PUBLIC -DPLATFORM_VARIANT=2)
             set(SCP_LOG_LEVEL "INFO" PARENT_SCOPE)
elseif (SCP_PLATFORM_VARIANT STREQUAL "3")
      target_compile_definitions(rdn2-bl2
                                 PUBLIC -DPLATFORM_VARIANT=3)
else()
      message(FATAL_ERROR "Please set SCP_PLATFORM_VARIANT to \n"
                                "'0' for RD-N2 \n"
                                "'1' for RD-N2-Cfg1 \n"
                                "'2' for RD-N2-Cfg2 \n"
                                "'3' for RD-N2-Cfg3 \n")
endif()

target_include_directories(
    rdn2-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../common/include")

# cmake-lint: disable=E1122

target_sources(
    rdn2-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v1.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu2.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_gtimer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_cmn700.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_system_pll.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pik_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_css_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_apcontext.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pcie_integ_ctrl.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_apremap.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_platform_system.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sp805.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/config_system_info.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/config_sid.c")

if(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS)
    if(BUILD_HAS_MOD_TRANSPORT_FC)
        target_sources(rdn2-bl2 PRIVATE "config_fch_polled.c")
    endif()
endif()

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdn2-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rdn2-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
