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
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain \
    css_clock

BS_FIRMWARE_MODULES := \
    armv7m_mpu \
    pl011 \
    log \
    pik_clock \
    clock \
    n1sdp_mhu \
    n1sdp_smt \
    scmi_agent \
    n1sdp_mcp_system

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    config_armv7m_mpu.c \
    config_log.c \
    config_pik_clock.c \
    config_clock.c \
    config_mhu.c \
    config_smt.c \
    config_scmi_agent.c

include $(BS_DIR)/firmware.mk
