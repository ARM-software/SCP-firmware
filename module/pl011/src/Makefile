#
# Arm SCP/MCP Software
# Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_NAME := "PL011"
BS_LIB_SOURCES = mod_pl011.c

ifeq ($(BUILD_HAS_DEBUGGER),yes)
    BS_LIB_SOURCES += debug_uart.c
endif

include $(BS_DIR)/lib.mk
