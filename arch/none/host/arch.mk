#
# Arm SCP/MCP Software
# Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_interrupt.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_main.c

BS_LIB_SOURCES_$(BS_ARCH_ARCH) := $(addprefix $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/src/,$(BS_LIB_SOURCES_$(BS_ARCH_ARCH)))
