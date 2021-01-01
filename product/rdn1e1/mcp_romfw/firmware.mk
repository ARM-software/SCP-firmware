#
# Arm SCP/MCP Software
# Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_USE_NEWLIB_NANO_SPECS := no

BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain

BS_FIRMWARE_MODULES := \
    bootloader \
    isys_rom \
    pl011 \
    clock

BS_FIRMWARE_SOURCES := \
    config_bootloader.c \
    config_clock.c \
    config_pl011.c

include $(BS_DIR)/firmware.mk
