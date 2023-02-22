#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET} PRIVATE "src/mod_morello_mcp_system.c")

target_link_libraries(
    ${SCP_MODULE_TARGET} PRIVATE module-clock module-pik-clock
                                 module-power-domain module-scmi-agent
                         PUBLIC module-timer)
