#
# Arm SCP/MCP Software
# Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef BS_CPU_MK
BS_CPU_MK := 1

include $(BS_DIR)/defs.mk

# Supported ARMv7-M CPUs
ARMV7M_CPUS := cortex-m3 cortex-m7

ifneq ($(findstring $(BS_FIRMWARE_CPU),$(ARMV7M_CPUS)),)
    BS_ARCH_CPU := $(BS_FIRMWARE_CPU)

    BS_ARCH_ARCH := armv7-m
    BS_ARCH_ISA := thumb

    LDFLAGS_GCC += --specs=nano.specs
    LDFLAGS_ARM += --target=arm-arm-none-eabi
    CFLAGS_CLANG += --target=arm-arm-none-eabi

    CFLAGS += -mfloat-abi=soft # No hardware floating point support
    CFLAGS += -mno-unaligned-access # Disable unaligned access code generation
else ifeq ($(BS_FIRMWARE_CPU),host)
    BS_ARCH_ARCH := host
else ifeq ($(BS_FIRMWARE_CPU),optee)
    BS_ARCH_ARCH := optee

    ifeq ($(BS_ARCH_CPU),cortex-a7)
    CFLAGS += -mfloat-abi=soft # No hardware floating point support
    CFLAGS += -mthumb -mthumb-interwork -fno-short-enums -fno-common -mno-unaligned-access
    endif
else
    $(erro "$(BS_FIRMWARE_CPU) is not a supported CPU. Aborting...")
endif

endif
