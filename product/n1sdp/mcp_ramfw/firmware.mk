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
    power_domain \
    css_clock

BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    pl011 \
    log \
    pik_clock \
    clock

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    config_armv7m_mpu.c \
    config_log.c \
    config_pik_clock.c \
    config_clock.c

include $(BS_DIR)/firmware.mk
