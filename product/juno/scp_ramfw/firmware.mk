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

BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain

BS_FIRMWARE_MODULES := \
    log \
    pl011 \
    clock \
    sds

BS_FIRMWARE_SOURCES := \
    rtx_config.c \
    juno_utils.c \
    config_sds.c \
    config_log.c \
    config_clock.c

include $(BS_DIR)/firmware.mk
