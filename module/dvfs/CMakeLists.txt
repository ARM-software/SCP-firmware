#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=E1122

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/doc/module_dvfs_architecture.md"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_dvfs.c")

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-clock module-psu
                                                   module-timer
                                                   module-scmi-perf
                                                   module-perf-controller)
