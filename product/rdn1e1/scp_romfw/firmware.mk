#
# Arm SCP/MCP Software
# Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
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
    sid \
    system_info \
    pcid \
    pl011 \
    log \
    rdn1e1_rom \
    gtimer \
    clock

BS_FIRMWARE_SOURCES := \
    config_sid.c \
    config_system_info.c \
    config_log.c \
    config_rdn1e1_rom.c \
    config_gtimer.c \
    config_clock.c

include $(BS_DIR)/firmware.mk
