#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_n1sdp_system.c")

target_link_libraries(${SCP_MODULE_TARGET}
    PRIVATE module-sds module-n1sdp-scp2pcc module-clock module-fip
            module-n1sdp-c2c module-power-domain module-n1sdp-dmc620
            module-ppu-v1 module-scmi module-system-power)
