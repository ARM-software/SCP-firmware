#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_executable(host)

target_include_directories(host PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(host PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_stdio.c"
                            "${CMAKE_CURRENT_SOURCE_DIR}/config_host.c")
