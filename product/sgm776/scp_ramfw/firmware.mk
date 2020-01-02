#
# Arm SCP/MCP Software
# Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m3
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes

BS_FIRMWARE_MODULES := \
    sid \
    system_info \
    pcid \
    pl011 \
    log \
    gtimer \
    timer \
    ddr_phy500 \
    dmc500 \
    reg_sensor \
    sensor \
    ppu_v1 \
    system_power \
    sgm776_system \
    power_domain \
    psu \
    mock_psu \
    mhu2 \
    smt \
    system_pll \
    pik_clock \
    css_clock \
    clock \
    dvfs \
    scmi \
    scmi_power_domain \
    scmi_system_power \
    scmi_clock \
    scmi_perf \
    scmi_sensor \
    scmi_apcore \
    sds

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    config_log.c \
    config_timer.c \
    config_ddr_phy500.c \
    config_dmc500.c \
    config_sensor.c \
    config_ppu_v1.c \
    config_system_power.c \
    config_power_domain.c \
    sgm776_core.c \
    config_sds.c \
    config_mhu.c \
    config_smt.c \
    config_scmi.c \
    config_scmi_system_power.c \
    config_scmi_clock.c \
    config_scmi_perf.c \
    config_scmi_apcore.c \
    config_system_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_clock.c \
    config_psu.c \
    config_mock_psu.c \
    config_dvfs.c \
    config_sid.c \
    config_system_info.c \
    config_scmi_power_domain.c

include $(BS_DIR)/firmware.mk
