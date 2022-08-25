#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
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

target_sources(
    juno-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_sds.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_ut.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests_entry.c"
        )
#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(juno-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    juno-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)

set(SCP_MODULES ${SCP_MODULES} PARENT_SCOPE)
set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
