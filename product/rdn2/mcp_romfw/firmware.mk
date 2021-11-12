#
# Arm SCP/MCP Software
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain

BS_FIRMWARE_MODULES := \
    pl011 \
    clock \
    bootloader \
    isys_rom

BS_FIRMWARE_SOURCES := \
    config_clock.c \
    config_bootloader.c \
    config_pl011.c

include $(BS_DIR)/firmware.mk
