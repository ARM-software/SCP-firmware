#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rdn2-mcp-bl1)

# SCP_PLATFORM_VARIANT value for supported RD-N2 platform variants:
# - '0' for RD-N2 platform
# - '1' for RD-N2-Cfg1 platform
# - '2' for RD-N2-Cfg2 platform
# - '3' for RD-N2-Cfg3 platform

if (SCP_PLATFORM_VARIANT STREQUAL "0")
      target_compile_definitions(rdn2-mcp-bl1
                                 PUBLIC -DPLATFORM_VARIANT=0)
elseif (SCP_PLATFORM_VARIANT STREQUAL "1")
      target_compile_definitions(rdn2-mcp-bl1
                                 PUBLIC -DPLATFORM_VARIANT=1)
elseif (SCP_PLATFORM_VARIANT STREQUAL "2")
      target_compile_definitions(rdn2-mcp-bl1
                                 PUBLIC -DPLATFORM_VARIANT=2)
elseif (SCP_PLATFORM_VARIANT STREQUAL "3")
      target_compile_definitions(rdn2-mcp-bl1
                                 PUBLIC -DPLATFORM_VARIANT=3)
else()
      message(FATAL_ERROR "Please set SCP_PLATFORM_VARIANT to \n"
                                "'0' for RD-N2 \n"
                                "'1' for RD-N2-Cfg1 \n"
                                "'2' for RD-N2-Cfg2 \n"
                                "'3' for RD-N2-Cfg3 \n")
endif()

target_include_directories(
    rdn2-mcp-bl1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    rdn2-mcp-bl1
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_bootloader.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/config_sid.c")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdn2-mcp-bl1 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interfaceinclude
# directories. Each module target adds these include directories totheir own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rdn2-mcp-bl1
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
