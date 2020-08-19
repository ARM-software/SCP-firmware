#
# Arm SCP/MCP Software
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
#

BS_FIRMWARE_CPU := cortex-m7
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \

BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    pl011 \
    cmn_skeena \
    apcontext \
    power_domain \
    ppu_v1 \
    ppu_v0 \
    system_power \
    morello_pll \
    dmc_bing \
    mhu \
    smt \
    scmi \
    sds \
    pik_clock \
    css_clock \
    clock \
    gtimer \
    timer \
    scmi_power_domain \
    scmi_system_power \
    scmi_management \
    fip \
    ssc \
    system_info \
    psu \
    mock_psu \
    dvfs \
    scmi_perf \
    morello_system \
    resource_perms

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    morello_core.c \
    config_armv7m_mpu.c \
    config_ssc.c \
    config_system_info.c \
    config_power_domain.c \
    config_ppu_v0.c \
    config_ppu_v1.c \
    config_dmc_bing.c \
    config_system_power.c \
    config_mhu.c \
    config_smt.c \
    config_scmi.c \
    config_sds.c \
    config_timer.c \
    config_cmn_skeena.c \
    config_scmi_system_power.c \
    config_scmi_power_domain.c \
    config_pl011.c \
    config_scmi_management.c \
    config_morello_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_clock.c \
    config_fip.c \
    config_psu.c \
    config_mock_psu.c \
    config_dvfs.c \
    config_scmi_perf.c \
    config_apcontext.c \
    config_resource_perms.c
include $(BS_DIR)/firmware.mk
