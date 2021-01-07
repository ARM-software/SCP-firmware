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
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_f_i2c.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/i2c_api.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/i2c_reg_access.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/i2c_depend.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/i2c_driver.c")

target_link_libraries(
    ${SCP_MODULE_TARGET} PRIVATE module-ppu-v0-synquacer
                                 module-synquacer-system module-power-domain)
