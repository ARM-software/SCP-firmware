#
# Arm SCP/MCP Software
# Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_interrupt.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_main.c

ifeq ($(BUILD_HAS_MULTITHREADING),yes)
    BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_thread.c
endif

BS_LIB_SOURCES_$(BS_ARCH_ARCH) := $(addprefix $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/src/,$(BS_LIB_SOURCES_$(BS_ARCH_ARCH)))
