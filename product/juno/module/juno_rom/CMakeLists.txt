#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/juno_wdog_rom.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/juno_debug_rom.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_juno_rom.c")

if(SCP_MODULE IN_LIST SCP_DT_CONFIG_MODULES_ALL)
    target_sources(${SCP_MODULE_TARGET}
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/config_dt_juno_rom.c")

    # no need to add header files to DT defines for this module
    
endif()
  
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-juno-ppu)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-power-domain)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-bootloader)
