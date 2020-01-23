#
# Arm SCP/MCP Software
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_NAME := debugger

BS_LIB_SOURCES += \
	checkpoint.c \
	cli/cli.c \
	cli/cli_commands_core.c \
	cli/cli_commands_checkpoint.c \
	cli/cli_fifo.c \
	cli/cli_platform_time.c

BS_LIB_INCLUDES += $(DBG_DIR)/include
BS_LIB_INCLUDES += $(ARCH_DIR)/include
BS_LIB_INCLUDES += $(FWK_DIR)/include

include $(BS_DIR)/lib.mk
