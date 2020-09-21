#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(CMAKE_ASM_COMPILER
    ${CMAKE_TOOLCHAIN_PREFIX}gcc
    CACHE FILEPATH "Path to the assembler.")
set(CMAKE_C_COMPILER
    ${CMAKE_TOOLCHAIN_PREFIX}gcc
    CACHE FILEPATH "Path to the C compiler.")
set(CMAKE_CXX_COMPILER
    ${CMAKE_TOOLCHAIN_PREFIX}g++
    CACHE FILEPATH "Path to the C++ compiler.")
