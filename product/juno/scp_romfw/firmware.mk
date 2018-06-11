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
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    power_domain \
    timer

BS_FIRMWARE_MODULES := \
    clock \
    pl011 \
    log \
    gtimer

BS_FIRMWARE_SOURCES := \
    juno_id.c \
    config_clock.c \
    config_log.c \
    config_timer.c

include $(BS_DIR)/firmware.mk
