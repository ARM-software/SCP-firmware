#
# Arm SCP/MCP Software
# Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
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
    clock \
    gtimer \
    timer \
    juno_ddr_phy400 \
    juno_dmc400 \
    juno_ram \
    juno_ppu \
    juno_system \
    system_power \
    power_domain \
    sds

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    juno_utils.c \
    config_sds.c \
    config_log.c \
    config_clock.c \
    juno_id.c \
    config_timer.c \
    config_juno_ddr_phy400.c \
    config_juno_dmc400.c \
    config_juno_ram.c \
    config_juno_ppu.c \
    config_system_power.c \
    config_power_domain.c

include $(BS_DIR)/firmware.mk
