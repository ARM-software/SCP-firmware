#
# Arm SCP/MCP Software
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain

BS_FIRMWARE_MODULES := \
    pl011 \
    clock \
    mscp_rom

BS_FIRMWARE_SOURCES := \
    config_pl011.c \
    config_clock.c \
    config_rddanielxlr_rom.c

include $(BS_DIR)/firmware.mk
