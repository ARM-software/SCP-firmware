#
# Arm SCP/MCP Software
# Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain \
    timer

BS_FIRMWARE_MODULES := \
    pl011 \
    clock \
    log \
    sgi575_rom \
    gtimer

BS_FIRMWARE_SOURCES := \
    config_log.c \
    config_clock.c \
    config_sgi575_rom.c \
    config_gtimer.c

include $(BS_DIR)/firmware.mk
