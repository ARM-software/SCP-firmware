#
# Arm SCP/MCP Software
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
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
    pl011 \
    ppu_v1 \
    msys_rom \
    sds \
    bootloader \
    system_pll \
    pik_clock \
    css_clock \
    clock \
    gtimer \
    timer \
    cmn_booker

BS_FIRMWARE_SOURCES := \
    config_pl011.c \
    config_ppu_v1.c \
    config_sds.c \
    config_cmn_booker.c \
    config_system_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_clock.c \
    config_gtimer.c \
    config_timer.c \
    config_msys_rom.c \
    config_bootloader.c



include $(BS_DIR)/firmware.mk
