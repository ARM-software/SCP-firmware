#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
   PUBLIC "include"
   PRIVATE "src")

target_sources(${SCP_MODULE_TARGET}
    PRIVATE "src/mod_xr77128.c")

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-psu
                                                   module-clock
                                                   module-timer
                                                   module-cdns-i2c)
