#
# Arm SCP/MCP Software
# Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
#

BS_FIRMWARE_CPU := cortex-m3
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes

BS_FIRMWARE_MODULE_HEADERS_ONLY :=

BS_FIRMWARE_MODULES := \
    log \
    pl011 \
    juno_soc_clock_ram \
    clock \
    juno_cdcel937 \
    juno_hdlcd \
    dvfs \
    gtimer \
    timer \
    juno_ddr_phy400 \
    juno_dmc400 \
    juno_ram \
    juno_ppu \
    juno_system \
    system_power \
    juno_debug \
    debug \
    power_domain \
    mhu \
    smt \
    scmi \
    scmi_clock \
    scmi_perf \
    scmi_power_domain \
    sensor \
    scmi_sensor \
    scmi_system_power \
    sds \
    i2c \
    dw_apb_i2c \
    juno_adc \
    juno_xrp7724 \
    reg_sensor \
    psu \
    mock_psu \
    juno_pvt \
    juno_thermal

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    juno_utils.c \
    config_sds.c \
    config_log.c \
    config_juno_soc_clock_ram.c \
    config_clock.c \
    config_dvfs.c \
    config_juno_cdcel937.c \
    config_juno_hdlcd.c \
    juno_id.c \
    config_timer.c \
    config_juno_ddr_phy400.c \
    config_juno_dmc400.c \
    config_juno_ram.c \
    config_juno_ppu.c \
    config_system_power.c \
    config_juno_debug.c \
    config_debug.c \
    config_power_domain.c \
    config_mhu.c \
    config_smt.c \
    config_scmi.c \
    config_scmi_clock.c \
    config_scmi_perf.c \
    config_scmi_system_power.c \
    config_i2c.c \
    config_juno_adc.c \
    config_juno_xrp7724.c \
    config_sensor.c \
    config_reg_sensor.c \
    config_psu.c \
    config_mock_psu.c \
    config_juno_pvt.c \
    config_juno_thermal.c \
    config_scmi_power_domain.c

include $(BS_DIR)/firmware.mk
