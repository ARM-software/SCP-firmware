#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef BS_CPU_MK
BS_CPU_MK := 1

include $(BS_DIR)/defs.mk

BS_ARCH_CPU := $(BS_FIRMWARE_CPU)

# Supported ARMv7-M CPUs
ARMV7M_CPUS := cortex-m3 cortex-m7

ifneq ($(findstring $(BS_FIRMWARE_CPU),$(ARMV7M_CPUS)),)
    BS_ARCH_ARCH := armv7-m
    BS_ARCH_ISA := thumb

    LDFLAGS_GCC += --specs=nano.specs
    LDFLAGS_ARM += --target=arm-arm-none-eabi
    CFLAGS_CLANG += --target=arm-arm-none-eabi

    CFLAGS += -mfloat-abi=soft # No hardware floating point support
else ifeq ($(BS_FIRMWARE_CPU),host)
    BS_ARCH_ARCH := host

else
    $(erro "$(BS_FIRMWARE_CPU) is not a supported CPU. Aborting...")
endif

endif
