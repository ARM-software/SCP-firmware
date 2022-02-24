#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/rcar_pwc.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/FreeRTOS_tick_config.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/rcar_iic_dvfs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_rcar_system.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/rcar_common.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/rcar_call_sram.S")

target_link_libraries(${SCP_MODULE_TARGET}
    PRIVATE module-sds module-rcar-clock module-rcar-reg-sensor module-sensor
            module-rcar-scif module-scmi module-system-power
            ${SCP_ARCHITECTURE_TARGET})
