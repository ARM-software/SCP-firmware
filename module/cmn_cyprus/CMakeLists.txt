#
# Arm SCP/MCP Software
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/interface/cmn")

target_sources(${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_cmn_cyprus.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_cfg_reg.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_node_info_reg.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_mxp_reg.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_discovery_setup.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_hns_reg.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_common.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_hnsam_setup.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_rnsam_reg.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cmn_cyprus_rnsam_setup.c")

target_link_libraries(${SCP_MODULE_TARGET}
    PRIVATE module-clock module-system-info module-timer)
