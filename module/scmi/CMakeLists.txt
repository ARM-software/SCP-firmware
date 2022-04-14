#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SCMI_SOURCES)
list(APPEND SCMI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_scmi.c")
if(NOT SCP_TARGET_EXCLUDE_BASE_PROTOCOL)
    add_definitions(-DBUILD_HAS_BASE_PROTOCOL)
    list(APPEND SCMI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_scmi_base.c")
endif()

target_sources(${SCP_MODULE_TARGET} PRIVATE ${SCMI_SOURCES})

if("resource-perms" IN_LIST SCP_MODULES)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-resource-perms)
endif()
