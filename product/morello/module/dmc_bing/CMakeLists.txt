#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

if(SCP_ENABLE_PLAT_FVP)
target_compile_definitions(${SCP_MODULE_TARGET}  PUBLIC -DPLAT_FVP=1)
endif()

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_dmc_bing.c")

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-clock module-timer)

# source and include files and libraries required by morello soc platform
if(NOT SCP_ENABLE_PLAT_FVP)
target_include_directories(${SCP_MODULE_TARGET}
                PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")


target_sources(${SCP_MODULE_TARGET}
                 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/dimm_spd.c"
                         "${CMAKE_CURRENT_SOURCE_DIR}/src/morello_ddr_phy.c"
                         "${CMAKE_CURRENT_SOURCE_DIR}/src/ddr_phy_values_800.c"
                         "${CMAKE_CURRENT_SOURCE_DIR}/src/ddr_phy_values_1200.c"
                         "${CMAKE_CURRENT_SOURCE_DIR}/src/ddr_phy_values_1333.c"
                         "${CMAKE_CURRENT_SOURCE_DIR}/src/ddr_phy_values_1466.c")

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-cdns-i2c)
endif()
