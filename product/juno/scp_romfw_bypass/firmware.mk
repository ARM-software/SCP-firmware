#
# Arm SCP/MCP Software
# Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
#

DEFINES += SCP_ROM_BYPASS=1

BS_FIRMWARE_CPU := cortex-m3
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes

BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain \
    system_power \
    psu \
    timer

BS_FIRMWARE_MODULES := \
    juno_ppu \
    juno_rom \
    juno_soc_clock \
    clock \
    gtimer \
    sds \
    bootloader

BS_FIRMWARE_SOURCES := \
    juno_utils.c \
    juno_id.c \
    config_juno_ppu.c \
    config_juno_rom.c \
    config_juno_soc_clock.c \
    config_clock.c \
    config_timer.c \
    config_sds.c \
    config_bootloader.c \
    juno_pll_workaround.c

LDFLAGS_GCC += -Wl,--wrap=arch_exception_reset

include $(BS_DIR)/firmware.mk
