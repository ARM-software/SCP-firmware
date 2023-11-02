#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           PUBLIC "${CMAKE_SOURCE_DIR}/interface/cmn"
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(
    ${SCP_MODULE_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_cmn700.c"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn700.c"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn700_ccg.c")

target_link_libraries(${SCP_MODULE_TARGET}
    PRIVATE module-clock module-timer module-sds module-system-info)
