#
# Arm SCP/MCP Software
# Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef BS_CPU_MK
BS_CPU_MK := 1

include $(BS_DIR)/defs.mk

BS_ARCH_CPU := $(BS_FIRMWARE_CPU)

# Supported ARMv7-M CPUs
ARMV7M_CPUS := cortex-m3 cortex-m7

# Supported ARMv8-A CPUs
ARMV8A_CPUS := cortex-a53 cortex-a57 cortex-a57.cortex-a53

ifneq ($(findstring $(BS_FIRMWARE_CPU),$(ARMV7M_CPUS)),)
    BS_ARCH_VENDOR := arm
    BS_ARCH_ARCH := armv7-m
    BS_ARCH_MODE := thumb

    ifeq ($(BS_FIRMWARE_USE_NEWLIB_NANO_SPECS),yes)
        # From GCC ARM Embedded 4.7 version and onwards, GCC toolchain includes
        # newlib-nano library. Selecting nano.specs results in use of
        # newlib-nano as standard C library. This will generate lesser code size
        # but with cuts in some features that were added after C89.
        LDFLAGS_GNU += --specs=nano.specs
    else
        LDFLAGS_GNU += --specs=nosys.specs
    endif

    # Compiler RT builtins architecture
    ifeq ($(BS_FIRMWARE_CPU),cortex-m7)
        CLANG_BUILTINS_ARCH := armv7em
    else
        CLANG_BUILTINS_ARCH := armv7m
    endif

    LDFLAGS_ARM += --target=arm-arm-none-eabi
    CFLAGS_CLANG += --target=arm-arm-none-eabi

    CFLAGS += -mfloat-abi=soft # No hardware floating point support
else ifneq ($(findstring $(BS_FIRMWARE_CPU),$(ARMV8A_CPUS)),)
    BS_ARCH_VENDOR := arm
    BS_ARCH_ARCH := armv8-a

    CFLAGS_CLANG += -Wno-asm-operand-widths --target=aarch64-none-elf
    CFLAGS_GCC += -mstrict-align
    CFLAGS += -fno-builtin

    DEP_CFLAGS_GCC += -DAARCH64
    DEP_ASFLAGS_GCC += -D__ASSEMBLY__
else ifeq ($(BS_FIRMWARE_CPU),host)
    BS_ARCH_VENDOR := none
    BS_ARCH_ARCH := host
else
    $(erro "$(BS_FIRMWARE_CPU) is not a supported CPU. Aborting...")
endif

endif
