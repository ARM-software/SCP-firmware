#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
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
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_morello_system.c")

target_link_libraries(
    ${SCP_MODULE_TARGET}
    PRIVATE module-sds
            module-clock
            module-power-domain
            module-ppu-v1
            module-scmi
            module-system-power
            module-dmc-bing)

if(NOT SCP_ENABLE_PLAT_FVP)
target_link_libraries(
        ${SCP_MODULE_TARGET} PRIVATE module-morello-scp2pcc
                                     module-morello-sensor module-sensor)
endif()
