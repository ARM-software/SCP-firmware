#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(juno-bl1)

target_include_directories(
    juno-bl1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

# files removed from build and replaced with _dt_ build versions
# as modules added to dt vars in Firmware.cmake
# "${CMAKE_CURRENT_SOURCE_DIR}/config_bootloader.c"
# "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
# "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
# "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
# "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_ppu.c"
# "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_rom.c"
#         

target_sources(
    juno-bl1
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_dt_juno_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_juno_soc_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_utils.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/juno_id.c")


# add in juno specific headers for device tree bindings processing,
# that are not include by other juno module builds
list(APPEND SCP_DT_BIND_H_GEN_FROM_INCL
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_ppu_idx.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_sds.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/software_mmap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/scp_mmap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/system_clock.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/system_mmap.h"
    )

# create a list of optional .dtsi includes that will be added to the final dt compile
# These are dependent on compile time defines which are generated later,
# so add to define list and file list for later processing
list(APPEND SCP_MODULE_DT_OPTIONS_DEFINE_REQ "BUILD_MODE_DEBUG")
list(APPEND SCP_MODULE_DT_OPTIONS_FILE_REQ "juno-scp-romfw-debug-opts.dtsi")


#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(juno-bl1 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    juno-bl1
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)

# ensure include lists pushed to parent scope
set(SCP_DT_BIND_H_GEN_FROM_INCL ${SCP_DT_BIND_H_GEN_FROM_INCL} PARENT_SCOPE)
set(SCP_MODULE_DT_OPTIONS_DEFINE_REQ ${SCP_MODULE_DT_OPTIONS_DEFINE_REQ} PARENT_SCOPE)
set(SCP_MODULE_DT_OPTIONS_FILE_REQ ${SCP_MODULE_DT_OPTIONS_FILE_REQ} PARENT_SCOPE)
