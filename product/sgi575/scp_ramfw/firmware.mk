#
# Arm SCP/MCP Software
# Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain

BS_FIRMWARE_MODULES := \
    pl011 \
    log \
    clock \
    system_pll \
    pik_clock \
    css_clock \
    gtimer \
    timer

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    config_log.c \
    config_clock.c \
    config_system_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_timer.c

include $(BS_DIR)/firmware.mk
