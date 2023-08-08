#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

set(CMAKE_SYSTEM_PROCESSOR "cortex-m55+nodsp")
set(CMAKE_TOOLCHAIN_PREFIX "arm-none-eabi-")

set(CMAKE_ASM_COMPILER_TARGET "arm-none-eabi")
set(CMAKE_C_COMPILER_TARGET "arm-none-eabi")
set(CMAKE_CXX_COMPILER_TARGET "arm-none-eabi")

include(
    "${CMAKE_CURRENT_LIST_DIR}/../../../cmake/Toolchain/GNU-Baremetal.cmake")
