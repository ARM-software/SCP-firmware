#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

# SCP_PLATFORM_VARIANT value for supported RD-N2 platform variants:
# - '0' for RD-N2 platform

if (SCP_PLATFORM_VARIANT STREQUAL "0")
      target_compile_definitions(${SCP_MODULE_TARGET}
                                 PUBLIC -DPLATFORM_VARIANT=0)
else()
      message(FATAL_ERROR "Please set SCP_PLATFORM_VARIANT to \n"
                                "'0' for RD-N2 \n")
endif()

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_platform_system.c")

target_link_libraries(${SCP_MODULE_TARGET}
    PRIVATE module-sds module-clock module-power-domain module-ppu-v1
            module-scmi module-system-power module-apremap)
