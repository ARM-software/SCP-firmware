#
# Arm SCP/MCP Software
# Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY :=

BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    sid \
    system_info \
    pcid \
    pl011 \
    log \
    cmn600 \
    apcontext \
    power_domain \
    ppu_v0 \
    ppu_v1 \
    system_power \
    dmc620 \
    ddr_phy500 \
    mhu2 \
    smt \
    scmi \
    sds \
    system_pll \
    pik_clock \
    css_clock \
    clock \
    gtimer \
    timer \
    sensor \
    reg_sensor \
    scmi_sensor \
    scmi_power_domain \
    scmi_system_power \
    scmi_apcore \
    mock_psu \
    psu \
    dvfs \
    scmi_perf \
    rdn1e1_system

BS_FIRMWARE_SOURCES := \
    config_system_power.c \
    config_sid.c \
    config_system_info.c \
    rtx_config.c \
    config_armv7m_mpu.c \
    config_log.c \
    config_power_domain.c \
    config_ppu_v0.c \
    config_ppu_v1.c \
    config_dmc620.c \
    config_ddr_phy500.c \
    config_mhu2.c \
    config_smt.c \
    config_scmi.c \
    config_sds.c \
    config_timer.c \
    config_sensor.c \
    config_cmn600.c \
    config_scmi_system_power.c \
    config_system_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_clock.c \
    config_mock_psu.c \
    config_psu.c \
    config_dvfs.c \
    config_scmi_perf.c \
    config_scmi_apcore.c \
    config_apcontext.c \
    config_scmi_power_domain.c

include $(BS_DIR)/firmware.mk
