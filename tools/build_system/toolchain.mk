#
# Arm SCP/MCP Software
# Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

_compiler_version ?= $(shell $(CC) --version)

ifneq ($(findstring armclang,$(_compiler_version)),)
    BS_COMPILER := CLANG
    BS_LINKER := ARM
else
    BS_COMPILER := GCC
    BS_LINKER := GCC
endif
