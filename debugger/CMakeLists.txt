#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Build the debugger framework integration library.
#

add_library(debugger)

target_compile_definitions(debugger PUBLIC "BUILD_HAS_DEBUGGER")

target_include_directories(debugger
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    debugger
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/checkpoint.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/cli.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/cli_commands_checkpoint.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/cli_commands_core.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/cli_fifo.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/cli_platform_time.c")

target_link_libraries(debugger PUBLIC framework)
