#
# Arm SCP/MCP Software
# Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m3
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_USE_NEWLIB_NANO_SPECS := yes

BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain \
    ppu_v0_synquacer \
    css_clock

INCLUDES += $(OS_DIR)/../Core/Include


BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    synquacer_rom \
    clock \
    synquacer_pik_clock \
    gtimer \
    timer

BS_FIRMWARE_SOURCES := \
    config_armv7m_mpu.c \
    config_clock.c \
    config_synquacer_pik_clock.c \
    config_synquacer_rom.c \
    config_timer.c

include $(BS_DIR)/firmware.mk
