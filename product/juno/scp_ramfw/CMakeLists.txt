#
# Arm SCP/MCP Software
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(juno-bl2)

target_include_directories(
    juno-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

# files removed from build and replaced with _dt_ build versions
# as modules added to dt vars in Firmware.cmake
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_clock.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_system_power.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ppu.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
#           "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"


target_sources(
    juno-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_dt_juno_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dt_juno_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dt_juno_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_soc_clock_ram.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_cdcel937.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_hdlcd.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ddr_phy400.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_dmc400.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ram.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_transport.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_system_power.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_i2c.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_adc.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_xrp7724.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_sensor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_reg_sensor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_pvt.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_thermal.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_utils.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_id.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_scmi_clock.c")


# add in juno specific headers for device tree bindings processing,
# that are not include by other juno module builds
list(APPEND SCP_DT_BIND_H_GEN_FROM_INCL
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/fmw_cmsis.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_clock.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_mmap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_sds.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/software_mmap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_scmi.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/system_clock.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/system_mmap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_power_domain.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_ppu_idx.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_alarm_idx.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../module/juno_cdcel937/include/mod_juno_cdcel937.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../module/juno_hdlcd/include/mod_juno_hdlcd.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../module/juno_soc_clock_ram/include/mod_juno_soc_clock_ram.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.h"
  )

# create a list of optional .dtsi includes that will be added to the final dt compile
# These are dependent on compile time defines which are generated later,
# so add to define list and file list for later processing
list(APPEND SCP_MODULE_DT_OPTIONS_DEFINE_REQ "BUILD_HAS_SCMI_NOTIFICATIONS")
list(APPEND SCP_MODULE_DT_OPTIONS_FILE_REQ "juno-scp-scmi-opts.dtsi") 
          
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
set(SCP_DT_BIND_H_GEN_FROM_INCL ${SCP_DT_BIND_H_GEN_FROM_INCL} PARENT_SCOPE)
set(SCP_MODULE_DT_OPTIONS_DEFINE_REQ ${SCP_MODULE_DT_OPTIONS_DEFINE_REQ} PARENT_SCOPE)
set(SCP_MODULE_DT_OPTIONS_FILE_REQ ${SCP_MODULE_DT_OPTIONS_FILE_REQ} PARENT_SCOPE)
