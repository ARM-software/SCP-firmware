#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_scp_platform.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/platform_lcp.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/platform_power_mgmt.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/platform_rss.c")

target_link_libraries(
    ${SCP_MODULE_TARGET}
    PRIVATE module-power-domain module-system-power
            module-system-info module-transport module-timer)
