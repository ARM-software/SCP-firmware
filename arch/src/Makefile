#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(BS_DIR)/cpu.mk
include $(BS_DIR)/toolchain.mk

BS_LIB_NAME = arch

include $(ARCH_DIR)/$(BS_ARCH_VENDOR)/vendor.mk

#
# Select arch-specific sources and includes
#

BS_LIB_SOURCES += $(BS_LIB_SOURCES_$(BS_ARCH_VENDOR))
BS_LIB_SOURCES += $(BS_LIB_SOURCES_$(BS_ARCH_ARCH))

BS_LIB_INCLUDES += $(BS_LIB_INCLUDES_$(BS_ARCH_VENDOR))
BS_LIB_INCLUDES += $(BS_LIB_INCLUDES_$(BS_ARCH_ARCH))

BS_LIB_INCLUDES += $(ARCH_DIR)/include
BS_LIB_INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/include
BS_LIB_INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/include

include $(BS_DIR)/lib.mk
