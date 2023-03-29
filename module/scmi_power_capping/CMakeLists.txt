#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
         PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_scmi_power_capping.c")

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-power-allocator)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-power-meter)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-power-coordinator)

if(SCP_EXCLUDE_SCMI_POWER_CAPPING_STD_COMMANDS AND
    (NOT SCP_ENABLE_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS))
     message( FATAL_ERROR "Non fast channels commands and fast channels commands can't be excluded simultaneously" )
endif()

if(NOT SCP_EXCLUDE_SCMI_POWER_CAPPING_STD_COMMANDS)
    add_definitions(-BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS)
    target_sources(${SCP_MODULE_TARGET}
             PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/scmi_power_capping_protocol.c")

    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-scmi)
endif()

if("resource-perms" IN_LIST SCP_MODULES)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-resource-perms)
endif()

if(SCP_ENABLE_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS)
    target_sources(
        ${SCP_MODULE_TARGET}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/scmi_power_capping_fast_channels.c")
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-transport)
endif()
