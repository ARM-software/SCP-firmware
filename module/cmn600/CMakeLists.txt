#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(
    ${SCP_MODULE_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_cmn600.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn600.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn600_ccix.c")

target_link_libraries(
    ${SCP_MODULE_TARGET} PRIVATE module-clock module-ppu-v1 module-system-info
                                 module-timer)
