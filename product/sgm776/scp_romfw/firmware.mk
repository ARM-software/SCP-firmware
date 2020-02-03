#
# Arm SCP/MCP Software
# Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m3
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes

BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain \
    timer

BS_FIRMWARE_MODULES := \
    sid \
    system_info \
    pcid \
    ppu_v1 \
    pl011 \
    log \
    gtimer \
    msys_rom \
    sds \
    bootloader \
    system_pll \
    pik_clock \
    css_clock \
    clock

BS_FIRMWARE_SOURCES := \
    sgm776_core.c \
    config_log.c \
    config_timer.c \
    config_msys_rom.c \
    config_sds.c \
    config_bootloader.c \
    config_ppu_v1.c \
    config_system_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_clock.c \
    config_sid.c \
    config_system_info.c

include $(BS_DIR)/firmware.mk
