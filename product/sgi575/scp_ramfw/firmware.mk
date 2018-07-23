#
# Arm SCP/MCP Software
# Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY :=

BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    pl011 \
    log \
    cmn600 \
    apcontext \
    power_domain \
    clock \
    dmc620 \
    ddr_phy500 \
    ppu_v0 \
    ppu_v1 \
    system_power \
    sgi575_system \
    mhu \
    smt \
    scmi \
    sds \
    system_pll \
    pik_clock \
    css_clock \
    gtimer \
    timer \
    sensor \
    reg_sensor \
    scmi_sensor

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    sgi575_core.c \
    config_armv7m_mpu.c \
    config_log.c \
    config_cmn600.c \
    config_power_domain.c \
    config_clock.c \
    config_dmc620.c \
    config_ddr_phy500.c \
    config_ppu_v0.c \
    config_ppu_v1.c \
    config_system_power.c \
    config_mhu.c \
    config_smt.c \
    config_scmi.c \
    config_sds.c \
    config_system_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_timer.c \
    config_sensor.c \
    config_apcontext.c

include $(BS_DIR)/firmware.mk
