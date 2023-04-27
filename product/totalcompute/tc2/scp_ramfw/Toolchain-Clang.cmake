#
# Arm SCP/MCP Software
# Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

set(CMAKE_SYSTEM_PROCESSOR "cortex-m3")

set(CMAKE_ASM_COMPILER_TARGET "arm-arm-none-eabi")
set(CMAKE_C_COMPILER_TARGET "arm-arm-none-eabi")
set(CMAKE_CXX_COMPILER_TARGET "arm-arm-none-eabi")

include(
    "${CMAKE_CURRENT_LIST_DIR}/../../../../cmake/Toolchain/Clang-Baremetal.cmake")
