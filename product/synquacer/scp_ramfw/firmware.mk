#
# Arm SCP/MCP Software
# Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-m3
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes

DEFINES += HAS_RTOS
DEFINES += PCIE_FILTER_BUS0_TYPE0_CONFIG
DEFINES += ENABLE_OPTEE
DEFINES += SET_PCIE_NON_SECURE
#DEFINES += CA53_USE_F_UART

INCLUDES += $(OS_DIR)/RTX/Include1


BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    f_uart3 \
    log \
    gtimer \
    timer \
    ppu_v0_synquacer \
    system_power \
    power_domain \
    clock \
    synquacer_system \
    pik_clock \
    css_clock \
    ccn512 \
    f_i2c \
    hsspi \
    synquacer_memc \
    mhu \
    smt \
    scmi \
    scmi_power_domain \
    scmi_system_power \
    scmi_apcore \
    scmi_vendor_ext


BS_FIRMWARE_SOURCES := \
    config_armv7m_mpu.c \
    config_ccn512.c \
    config_clock.c \
    config_css_clock.c \
    config_f_i2c.c \
    config_hsspi.c \
    config_log_f_uart3.c \
    config_mhu.c \
    config_pik_clock.c \
    config_power_domain.c \
    config_ppu_v0_synquacer.c \
    config_scmi.c \
    config_scmi_apcore.c \
    config_scmi_system_power.c \
    config_smt.c \
    config_synquacer_memc.c \
    config_system_power.c \
    config_timer.c \
    rtx_config.c \
    config_scmi_power_domain.c

include $(BS_DIR)/firmware.mk
