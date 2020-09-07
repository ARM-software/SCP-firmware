#
# Arm SCP/MCP Software
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m3
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_HAS_RESOURCE_PERMISSIONS := yes

BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    pl011 \
    ppu_v1 \
    system_power \
    mhu2 \
    smt \
    scmi \
    sds \
    system_pll \
    pik_clock \
    css_clock \
    clock \
    power_domain \
    scmi_power_domain \
    scmi_system_power \
    dvfs \
    scmi_clock \
    scmi_perf \
    gtimer \
    timer \
    mock_psu \
    psu \
    tc0_system

ifeq ($(BS_FIRMWARE_HAS_RESOURCE_PERMISSIONS),yes)
    BS_FIRMWARE_MODULES += resource_perms
endif

BS_FIRMWARE_SOURCES := \
    config_system_power.c \
    rtx_config.c \
    config_armv7m_mpu.c \
    config_pl011.c \
    config_power_domain.c \
    config_ppu_v1.c \
    config_mhu2.c \
    config_smt.c \
    config_scmi.c \
    config_sds.c \
    config_scmi_system_power.c \
    config_scmi_clock.c \
    config_scmi_perf.c \
    config_gtimer.c \
    config_timer.c \
    config_dvfs.c \
    config_psu.c \
    config_mock_psu.c \
    config_system_pll.c \
    config_pik_clock.c \
    config_css_clock.c \
    config_clock.c \
    config_scmi_power_domain.c

ifeq ($(BS_FIRMWARE_HAS_RESOURCE_PERMISSIONS),yes)
    BS_FIRMWARE_SOURCES += config_resource_perms.c
endif

include $(BS_DIR)/firmware.mk
