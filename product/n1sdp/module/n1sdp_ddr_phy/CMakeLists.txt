#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ddr_phy_values_1333.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ddr_phy_values_800.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_n1sdp_ddr_phy.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ddr_phy_values_1200.c"
)

target_link_libraries(${SCP_MODULE_TARGET}
    PRIVATE module-n1sdp-dmc620)
