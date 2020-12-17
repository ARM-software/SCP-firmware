#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_ddr_phy500.c")

if("dmc500" IN_LIST SCP_MODULES)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-dmc500)
endif()

if("dmc620" IN_LIST SCP_MODULES)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-dmc620)
endif()
