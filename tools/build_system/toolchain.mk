#
# Arm SCP/MCP Software
# Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef BS_TOOLCHAIN_MK
BS_TOOLCHAIN_MK := 1

_compiler_version ?= $(shell $(CC) --version)

ifneq ($(findstring armclang,$(_compiler_version)),)
    BS_COMPILER := CLANG
    BS_LINKER := ARM
else ifneq ($(findstring clang,$(_compiler_version)),)
    BS_COMPILER := CLANG
    BS_TOOLCHAIN := LLVM
    # LLVM GCC-compatible lld
    BS_LINKER := GCC
else
    BS_COMPILER := GCC
    BS_TOOLCHAIN := GNU
    # GNU GCC ld
    BS_LINKER := GCC
endif

endif
