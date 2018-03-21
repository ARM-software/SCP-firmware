#
# Arm SCP/MCP Software
# Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/arch.mk

BS_LIB_INCLUDES_$(BS_ARCH_VENDOR) += $(TOP_DIR)/cmsis/CMSIS/Core/Include

BS_LIB_SOURCES_$(BS_ARCH_VENDOR) += arch_mm.c

BS_LIB_SOURCES_$(BS_ARCH_VENDOR) := $(addprefix $(ARCH_DIR)/$(BS_ARCH_VENDOR)/src/,$(BS_LIB_SOURCES_$(BS_ARCH_VENDOR)))
