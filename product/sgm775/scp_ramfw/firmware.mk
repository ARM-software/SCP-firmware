#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
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
    sgm775_ddr_phy500 \
    sgm775_dmc500 \
    pik_clock \
    clock \
    system_pll \
    css_clock \
    ppu_v0 \
    ppu_v1 \
    system_power \
    sgm775_system \
    power_domain \
    reg_sensor \
    sensor \
    dvfs \
    psu \
    mock_psu \
    mhu \
    smt \
    scmi \
    scmi_power_domain \
    scmi_clock \
    scmi_perf \
    scmi_sensor \
    scmi_system_power \
    scmi_apcore \
    sds

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    sgm775_core.c \
    config_log.c \
    config_timer.c \
    config_sgm775_ddr_phy500.c \
    config_sgm775_dmc500.c \
    config_sds.c \
    config_pik_clock.c \
    config_clock.c \
    config_system_pll.c \
    config_css_clock.c \
    config_ppu_v0.c \
    config_ppu_v1.c \
    config_power_domain.c \
    config_sensor.c \
    config_dvfs.c \
    config_psu.c \
    config_mock_psu.c \
    config_mhu.c \
    config_smt.c \
    config_scmi.c \
    config_scmi_clock.c \
    config_scmi_perf.c \
    config_scmi_system_power.c \
    config_scmi_apcore.c \
    config_system_power.c \
    config_sid.c \
    config_system_info.c \
    config_scmi_power_domain.c

ifeq ($(BUILD_HAS_DEBUGGER),yes)
    BS_FIRMWARE_MODULES += debugger_cli
    BS_FIRMWARE_SOURCES += config_debugger_cli.c
endif

include $(BS_DIR)/firmware.mk
