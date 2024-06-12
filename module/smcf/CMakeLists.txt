#
# Arm SCP/MCP Software
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    ${SCP_MODULE_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_smcf.c"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/smcf_data.c"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/smcf_utils.c"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/mgi.c")

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-timer)
