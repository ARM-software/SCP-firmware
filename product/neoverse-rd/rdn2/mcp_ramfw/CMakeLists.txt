#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_executable(rdn2-mcp-bl2)

# Add SCP_PLATFORM_VARIANT compiler flag

if(SCP_PLATFORM_VARIANT STREQUAL "0")
    target_compile_definitions(rdn2-mcp-bl2 PUBLIC -DPLATFORM_VARIANT=0)
elseif(SCP_PLATFORM_VARIANT STREQUAL "1")
    target_compile_definitions(rdn2-mcp-bl2 PUBLIC -DPLATFORM_VARIANT=1)
elseif(SCP_PLATFORM_VARIANT STREQUAL "2")
    target_compile_definitions(rdn2-mcp-bl2 PUBLIC -DPLATFORM_VARIANT=2)
elseif(SCP_PLATFORM_VARIANT STREQUAL "3")
    target_compile_definitions(rdn2-mcp-bl2 PUBLIC -DPLATFORM_VARIANT=3)
else()
    message(FATAL_ERROR "Please set SCP_PLATFORM_VARIANT to \n"
                                 "'0' for RD-N2 \n"
                                 "'1' for RD-N2-Cfg1 \n"
                                 "'2' for RD-N2-Cfg2 \n"
                                 "'3' for RD-N2-Cfg3 \n")
endif()

target_include_directories(
    rdn2-mcp-bl2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_SOURCE_DIR}")

# cmake-lint: disable=E1122

target_sources(
    rdn2-mcp-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_gtimer.c")

if(SCP_ENABLE_DEBUGGER)
    target_compile_definitions(rdn2-mcp-bl2 PRIVATE BUILD_HAS_DEBUGGER)
    target_sources(rdn2-mcp-bl2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_debugger_cli.c")
endif()

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdn2-mcp-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(rdn2-mcp-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
