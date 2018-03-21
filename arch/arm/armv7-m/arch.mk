#
# Arm SCP/MCP Software
# Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_exceptions.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_handlers.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_main.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_nvic.c

BS_LIB_SOURCES_$(BS_ARCH_ARCH) := $(addprefix $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/src/,$(BS_LIB_SOURCES_$(BS_ARCH_ARCH)))

ifneq ($(filter $(BS_FIRMWARE_CPU),cortex-m3 cortex-m7),)
    ifeq ($(BS_COMPILER),ARM)
        BS_LIB_DEPS += $(OS_DIR)/RTX/Library/ARM/RTX_CM3.lib
    else
        BS_LIB_DEPS += $(OS_DIR)/RTX/Library/GCC/libRTX_CM3.a
    endif
endif
