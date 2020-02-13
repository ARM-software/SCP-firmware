#
# Arm SCP/MCP Software
# Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
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
    ppu_v1 \
    ppu_v0 \
    system_power \
    n1sdp_pll \
    n1sdp_i2c \
    n1sdp_dmc620 \
    n1sdp_ddr_phy \
    mhu \
    smt \
    scmi \
    sds \
    pik_clock \
    css_clock \
    clock \
    gtimer \
    timer \
    n1sdp_scp2pcc \
    n1sdp_sensor \
    sensor \
    scmi_power_domain \
    scmi_sensor \
    scmi_system_power \
    scmi_management \
    scmi_ccix_config \
    n1sdp_flash \
    n1sdp_timer_sync \
    n1sdp_c2c \
    n1sdp_remote_pd \
    n1sdp_pcie \
    ssc \
    system_info \
    n1sdp_system

BS_FIRMWARE_SOURCES := \
    config_system_power.c \
    rtx_config.c \
    n1sdp_core.c \
    config_armv7m_mpu.c \
    config_ssc.c \
    config_system_info.c \
    config_log.c \
    config_power_domain.c \
    config_ppu_v0.c \
    config_ppu_v1.c \
    config_n1sdp_i2c.c \
    config_n1sdp_dmc620.c \
    config_n1sdp_ddr_phy.c \
    config_mhu.c \
    config_smt.c \
    config_scmi.c \
    config_sds.c \
    config_timer.c \
    config_cmn600.c \
    config_scmi_system_power.c \
    config_n1sdp_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_clock.c \
    config_n1sdp_flash.c \
    config_n1sdp_pcie.c \
    config_n1sdp_scp2pcc.c \
    config_sensor.c \
    config_apcontext.c \
    config_n1sdp_c2c_i2c.c \
    config_n1sdp_remote_pd.c \
    config_n1sdp_timer_sync.c \
    config_scmi_power_domain.c

ifeq ($(BUILD_HAS_DEBUGGER),yes)
    BS_FIRMWARE_MODULES += debugger_cli
    BS_FIRMWARE_SOURCES += config_debugger_cli.c
endif

include $(BS_DIR)/firmware.mk
