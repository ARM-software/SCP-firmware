#
# Arm SCP/MCP Software
# Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

set(CMAKE_SYSTEM_PROCESSOR "optee")
set(CMAKE_TOOLCHAIN_PREFIX ${CFG_CROSS_COMPILE})

set(CMAKE_ASM_COMPILER_TARGET ${CFG_CROSS_COMPILE})
set(CMAKE_C_COMPILER_TARGET ${CFG_CROSS_COMPILE})
set(CMAKE_CXX_COMPILER_TARGET ${CFG_CROSS_COMPILE})

include(
    "${CMAKE_CURRENT_LIST_DIR}/../../../../cmake/Toolchain/GNU-Baremetal.cmake")

foreach(language IN ITEMS ASM C CXX)
    string(APPEND CMAKE_${language}_FLAGS_INIT ${CFG_CFLAGS_OPTEE})
endforeach()
