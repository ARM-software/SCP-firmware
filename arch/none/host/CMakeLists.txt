#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-none)

target_include_directories(arch-none
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    arch-none PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_interrupt.c"
                      "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_main.c")
